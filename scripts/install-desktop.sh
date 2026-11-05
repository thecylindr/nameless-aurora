#!/usr/bin/env bash
set -euo pipefail
AURORA_SOURCE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
AURORA_APP_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
AURORA_ICON_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/icons/hicolor/scalable/apps"
AURORA_BIN_DIR="$HOME/.local/bin"
mkdir -p "$AURORA_APP_DIR" "$AURORA_ICON_DIR" "$AURORA_BIN_DIR"
cp -- "$AURORA_SOURCE_DIR/packaging/aurora.svg" "$AURORA_ICON_DIR/org.nameless.Aurora.svg"
python3 - "$AURORA_SOURCE_DIR" "$AURORA_APP_DIR" "$AURORA_BIN_DIR" <<'PY'
from pathlib import Path
import shlex, sys
source, appdir, bindir = map(Path, sys.argv[1:])
launcher = bindir / 'aurora'
if launcher.exists() and 'nameless-aurora' not in launcher.read_text(errors='replace'):
    raise SystemExit(f'{launcher} уже существует и относится к другому приложению. Выберите другой путь.')
launcher.write_text('#!/usr/bin/env bash\nexec ' + shlex.quote(str(source / 'scripts/run.sh')) + ' "$@"\n')
launcher.chmod(0o755)
text = (source / 'packaging/org.nameless.Aurora.desktop').read_text()
# Desktop Entry quoting is separate from shell quoting.
path = str(launcher).replace('\\', '\\\\').replace('"', '\\"').replace('`', '\\`').replace('$', '\\$').replace('%', '%%')
text = text.replace('Exec=AuroraPlayer', 'Exec="' + path + '"')
(appdir / 'org.nameless.Aurora.desktop').write_text(text)
print('Установлен ярлык Aurora: ' + str(appdir / 'org.nameless.Aurora.desktop'))
PY
if command -v update-desktop-database >/dev/null; then update-desktop-database "$AURORA_APP_DIR"; fi
