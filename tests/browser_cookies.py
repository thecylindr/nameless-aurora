import importlib.util
import json
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
import time
import types
import unittest
from unittest.mock import patch

sys.dont_write_bytecode = True

SCRIPT = Path(__file__).resolve().parents[1] / "src/core/web/browser_cookies.py"
SPEC = importlib.util.spec_from_file_location("aurora_browser_cookies", SCRIPT)
reader = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(reader)


class BrowserCookies(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def firefox(self):
        db = sqlite3.connect(self.root / "cookies.sqlite")
        db.execute("PRAGMA journal_mode=WAL")
        db.execute("PRAGMA wal_autocheckpoint=0")
        db.execute("CREATE TABLE moz_cookies(host TEXT,name TEXT,value TEXT,path TEXT,expiry INTEGER,isSecure INTEGER,isHttpOnly INTEGER)")
        return db

    def chromium(self):
        db = sqlite3.connect(self.root / "Cookies")
        db.execute("CREATE TABLE meta(key TEXT,value TEXT)")
        db.execute("INSERT INTO meta VALUES('version','24')")
        db.execute("CREATE TABLE cookies(host_key TEXT,name TEXT,value TEXT,encrypted_value BLOB,path TEXT,expires_utc INTEGER,is_secure INTEGER,is_httponly INTEGER,top_frame_site_key TEXT)")
        return db

    def test_firefox_reads_live_wal_and_only_the_selected_service(self):
        with self.firefox() as db:
            db.executemany("INSERT INTO moz_cookies VALUES(?,?,?,?,?,?,?)", [
                (".vk.ru", "remixsid6", "fixture-session", "/", int(time.time()) + 3600, 1, 1),
                (".yandex.ru", "Session_id", "other-service", "/", 0, 1, 1),
                (".example.com", "password", "unrelated", "/", 0, 1, 1),
                (".vk.ru.attacker.example", "remixsid6", "spoof", "/", 0, 1, 1),
                (".vk.ru", "expired", "old", "/", 10, 1, 1),
            ])
            db.commit()
            self.assertTrue((self.root / "cookies.sqlite-wal").is_file())
            result = reader.read_session("firefox", str(self.root), "vk")
            self.assertEqual([c["name"] for c in result["cookies"]], ["remixsid6"])
            self.assertTrue(result["cookies"][0]["httpOnly"])
            self.assertTrue(result["cookies"][0]["secure"])

    def test_domain_filter_runs_before_chromium_decryption(self):
        with self.chromium() as db:
            db.executemany("INSERT INTO cookies VALUES(?,?,?,?,?,?,?,?,?)", [
                (".vk.ru", "remixsid6", "", b"v10fixture", "/", 0, 1, 1, ""),
                (".yandex.ru", "Session_id", "", b"must-not-decrypt", "/", 0, 1, 1, ""),
                (".vk.ru", "partitioned", "", b"must-not-decrypt", "/", 0, 1, 1, "https://example.com"),
            ])
        seen = []

        class Decryptor:
            def decrypt(self, value):
                seen.append(value)
                return "fixture-session"

        module = types.ModuleType("yt_dlp.cookies")
        module.get_cookie_decryptor = lambda *args, **kwargs: Decryptor()
        with patch.dict(sys.modules, {"yt_dlp": types.ModuleType("yt_dlp"), "yt_dlp.cookies": module}):
            result = reader.read_session("brave", str(self.root), "vk")
        self.assertEqual(seen, [b"v10fixture"])
        self.assertEqual(len(result["cookies"]), 1)
        self.assertEqual(result["cookies"][0]["value"], "fixture-session")

    def test_keyring_failure_is_distinct_from_missing_login(self):
        with self.chromium() as db:
            db.execute("INSERT INTO cookies VALUES(?,?,?,?,?,?,?,?,?)",
                       (".vk.ru", "remixsid6", "", b"v11fixture", "/", 0, 1, 1, ""))
        module = types.ModuleType("yt_dlp.cookies")
        module.get_cookie_decryptor = lambda *args, **kwargs: types.SimpleNamespace(decrypt=lambda value: None)
        with patch.dict(sys.modules, {"yt_dlp": types.ModuleType("yt_dlp"), "yt_dlp.cookies": module}):
            self.assertEqual(reader.read_session("brave", str(self.root), "vk")["error"], "keyring")

    def test_expired_session_and_wrong_service_are_not_accepted(self):
        with self.firefox() as db:
            db.executemany("INSERT INTO moz_cookies VALUES(?,?,?,?,?,?,?)", [
                (".vk.ru", "remixsid6", "old", "/", 10, 1, 1),
                (".yandex.ru", "Session_id", "fixture", "/", 0, 1, 1),
            ])
        self.assertEqual(reader.read_session("firefox", str(self.root), "vk")["error"], "no_session")
        self.assertEqual(reader.read_session("firefox", str(self.root), "yandex")["cookies"][0]["name"], "Session_id")

    def test_source_database_is_unchanged(self):
        with self.chromium() as db:
            db.execute("INSERT INTO cookies VALUES(?,?,?,?,?,?,?,?,?)",
                       (".vk.ru", "remixsid6", "fixture-session", b"", "/", 0, 1, 1, ""))
        before = (self.root / "Cookies").read_bytes()
        self.assertEqual(len(reader.read_session("brave", str(self.root), "vk")["cookies"]), 1)
        self.assertEqual(before, (self.root / "Cookies").read_bytes())

    def test_cli_errors_are_json_without_tracebacks_or_secrets(self):
        result = subprocess.run([sys.executable, str(SCRIPT), "brave", str(self.root), "vk"],
                                capture_output=True, text=True, check=True)
        self.assertEqual(json.loads(result.stdout), {"error": "missing"})
        self.assertEqual(result.stderr, "")
        self.assertEqual(reader.read_session("unknown", str(self.root), "vk")["error"], "unsupported")


if __name__ == "__main__":
    unittest.main()
