"""Read only one music service's cookies from an existing browser profile.

The caller receives JSON over a private process pipe. Never log cookie values,
decrypt unrelated domains, or modify the source database. Chromium's supported
OS key stores and AES formats are handled by the installed yt-dlp library.
"""
import contextlib
import json
import os
from pathlib import Path
import sqlite3
import shutil
import sys
import time
from urllib.parse import quote

DOMAINS = {
    "vk": ("vk.com", "vk.ru", "vk.id", "vk.me"),
    "yandex": ("yandex.ru", "yandex.com", "yandex.by", "yandex.kz", "yandex.uz"),
}
MARKERS = {"vk": {"remixsid", "remixsid6"}, "yandex": {"Session_id"}}
KEYRINGS = {
    "brave": ("Brave",),
    "chrome": ("Chrome",),
    "chromium": ("Chromium",),
    "edge": ("Chromium",),
    "opera": ("Chromium",),
    "vivaldi": ("Chrome",),
    "yandex": ("Yandex", "Yandex Browser", "Chromium"),
}


class QuietLogger:
    def debug(self, *args, **kwargs):
        pass

    info = warning = error = debug

    def progress_bar(self):
        return None


def scoped(domain, service):
    host = domain.lstrip(".").lower()
    return any(host == base or host.endswith("." + base) for base in DOMAINS[service])


def read_session(browser, profile, service):
    if service not in DOMAINS or browser not in {*KEYRINGS, "firefox"}:
        return {"error": "unsupported"}
    root = Path(profile).expanduser()
    choices = [root / "cookies.sqlite"] if browser == "firefox" else [
        root / "Network/Cookies", root / "Cookies"]
    database = next((path for path in choices if path.is_file()), None)
    if database is None:
        return {"error": "missing"}
    deadline = time.monotonic() + 12

    def progress(*args):
        if time.monotonic() > deadline:
            raise TimeoutError()

    # SQLite backup includes WAL transactions from a running browser. Copying
    # only the database file can silently return an old or empty login.
    source = sqlite3.connect(f"file:{quote(str(database), safe='/')}?mode=ro",
                             uri=True, timeout=2)
    snapshot = sqlite3.connect(":memory:")
    try:
        source.backup(snapshot, pages=128, progress=progress, sleep=0.02)
    finally:
        source.close()
    try:
        table = "moz_cookies" if browser == "firefox" else "cookies"
        columns = {row[1] for row in snapshot.execute(f"PRAGMA table_info({table})")}
        host_column = "host" if browser == "firefox" else "host_key"
        terms, parameters = [], []
        for domain in DOMAINS[service]:
            terms.extend([f"{host_column} = ?", f"{host_column} = ?",
                          f"{host_column} LIKE ?"])
            parameters.extend([domain, "." + domain, "%." + domain])
        snapshot.row_factory = sqlite3.Row
        rows = snapshot.execute(
            f"SELECT * FROM {table} WHERE " + " OR ".join(terms),
            parameters).fetchall()
        if len(rows) > 5000:
            return {"error": "oversized"}
        decryptors = None
        now = time.time()
        cookies, encrypted_session, decryption_failed = [], False, False
        for row in rows:
            domain, name = row[host_column], row["name"]
            if not scoped(domain, service):
                continue
            if browser == "firefox" and "originAttributes" in columns and row["originAttributes"]:
                continue
            # A partitioned third-party cookie must not become a first-party one.
            if "top_frame_site_key" in columns and row["top_frame_site_key"]:
                continue
            if browser == "firefox":
                expiry, value = row["expiry"], row["value"]
                secure, http_only = bool(row["isSecure"]), bool(row["isHttpOnly"])
            else:
                expiry = row["expires_utc"] / 1_000_000 - 11644473600 if row["expires_utc"] else 0
                value = row["value"]
                secure = bool(row["is_secure"] if "is_secure" in columns else row["secure"])
                http_only = bool(row["is_httponly"]) if "is_httponly" in columns else False
                if not value and row["encrypted_value"]:
                    encrypted_session |= name in MARKERS[service]
                    if expiry > 0 and expiry <= now:
                        continue
                    if decryptors is None:
                        try:
                            from yt_dlp.cookies import get_cookie_decryptor
                        except ImportError:
                            return {"error": "dependency"}
                        version_row = snapshot.execute(
                            "SELECT value FROM meta WHERE key = 'version'").fetchone()
                        version = int(version_row[0]) if version_row else 0
                        # Hyprland does not identify as GNOME/KDE, even when the
                        # browser used libsecret. Do not mistake that for logout.
                        backends = [None]
                        if sys.platform.startswith("linux"):
                            backends.append("GNOMEKEYRING")
                            if shutil.which("kwallet-query"):
                                backends.extend(["KWALLET6", "KWALLET5"])
                        decryptors = [get_cookie_decryptor(
                            str(root.parent), product, QuietLogger(), keyring=backend, meta_version=version)
                            for product in KEYRINGS[browser] for backend in backends]
                    value = None
                    for decryptor in decryptors:
                        try:
                            value = decryptor.decrypt(bytes(row["encrypted_value"]))
                        except Exception:
                            value = None
                        if value:
                            break
                    if not value:
                        decryption_failed = True
                        continue
            if not value or (expiry > 0 and expiry <= now):
                continue
            cookies.append({
                "name": name, "value": value, "domain": domain,
                "path": row["path"] or "/", "expires": max(0, int(expiry)),
                "secure": secure, "httpOnly": http_only,
            })
        if not any(cookie["name"] in MARKERS[service] for cookie in cookies):
            return {"error": "keyring" if encrypted_session and decryption_failed else "no_session"}
        return {"cookies": cookies}
    finally:
        snapshot.close()


def main():
    try:
        if len(sys.argv) != 4:
            result = {"error": "arguments"}
        else:
            # Libraries must never leak logging into the JSON protocol.
            with open(os.devnull, "w") as quiet, contextlib.redirect_stdout(quiet), contextlib.redirect_stderr(quiet):
                result = read_session(*sys.argv[1:])
    except (PermissionError, sqlite3.OperationalError):
        result = {"error": "locked"}
    except TimeoutError:
        result = {"error": "timeout"}
    except Exception:
        result = {"error": "read"}
    print(json.dumps(result, ensure_ascii=False))


if __name__ == "__main__":
    main()
