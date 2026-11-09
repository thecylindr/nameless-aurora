#!/usr/bin/env bash
# Bundle a cross-built llvm-mingw Windows binary on Linux.
set -euo pipefail
AURORA_SOURCE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
AURORA_EXE="$(realpath "${1:-$AURORA_SOURCE_DIR/build/windows/AuroraPlayer.exe}")"
AURORA_OUT="$(realpath -m "${2:-$AURORA_SOURCE_DIR/dist/windows}")"
AURORA_QT="${QT_WIN_DIR:-$HOME/Qt/6.11.1/llvm-mingw_64}"
AURORA_LLVM="${LLVM_MINGW_DIR:-$HOME/Qt/llvm-mingw}"
AURORA_OBJDUMP="$AURORA_LLVM/bin/llvm-objdump"
[[ -f "$AURORA_EXE" && -x "$AURORA_OBJDUMP" ]] || { echo 'Missing executable or llvm-objdump' >&2; exit 1; }
mkdir -p "$AURORA_OUT"
cp "$AURORA_EXE" "$AURORA_OUT/AuroraPlayer.exe"
declare -A aurora_seen
resolve_dlls() {
    local input="$1" dll candidate folder
    while IFS= read -r dll; do
        [[ -n "$dll" && -z "${aurora_seen[${dll,,}]:-}" ]] || continue
        aurora_seen[${dll,,}]=1
        candidate=''
        for folder in "$AURORA_QT/bin" "$AURORA_LLVM/x86_64-w64-mingw32/bin" "$AURORA_LLVM/x86_64-w64-mingw32/lib"; do
            if [[ -f "$folder/$dll" ]]; then candidate="$folder/$dll"; break; fi
        done
        if [[ -n "$candidate" ]]; then
            cp "$candidate" "$AURORA_OUT/"
            resolve_dlls "$candidate"
        fi
    done < <("$AURORA_OBJDUMP" -p "$input" | awk '/DLL Name:/{print $3}')
}
resolve_dlls "$AURORA_EXE"
for plugin in platforms/qwindows.dll platforms/qoffscreen.dll imageformats/qjpeg.dll imageformats/qsvg.dll imageformats/qwebp.dll imageformats/qgif.dll iconengines/qsvgicon.dll tls/qschannelbackend.dll sqldrivers/qsqlite.dll multimedia/ffmpegmediaplugin.dll multimedia/windowsmediaplugin.dll styles/qmodernwindowsstyle.dll; do
    if [[ -f "$AURORA_QT/plugins/$plugin" ]]; then
        mkdir -p "$AURORA_OUT/plugins/$(dirname "$plugin")"
        cp "$AURORA_QT/plugins/$plugin" "$AURORA_OUT/plugins/$plugin"
        resolve_dlls "$AURORA_QT/plugins/$plugin"
    fi
done
# Runtime QML imports used by Aurora and their standard Qt dependencies.
mkdir -p "$AURORA_OUT/qml"
for module in QtQuick QtQml QtCore Qt; do
    if [[ -d "$AURORA_QT/qml/$module" ]]; then
        cp -r "$AURORA_QT/qml/$module" "$AURORA_OUT/qml/"
    fi
done
while IFS= read -r -d '' plugin; do resolve_dlls "$plugin"; done < <(find "$AURORA_OUT/qml" -name '*.dll' -print0)
for dll in opengl32sw.dll d3dcompiler_47.dll; do
    if [[ -f "$AURORA_QT/bin/$dll" ]]; then cp "$AURORA_QT/bin/$dll" "$AURORA_OUT/"; fi
done
cat > "$AURORA_OUT/qt.conf" <<'CONF'
[Paths]
Plugins=plugins
QmlImports=qml
CONF
printf '@echo off\r\ncd /d "%%~dp0"\r\nset QT_WIN_DEBUG_CONSOLE=attach\r\nset QT_DEBUG_PLUGINS=1\r\nAuroraPlayer.exe > debug_log.txt 2>&1\r\npause\r\n' > "$AURORA_OUT/run-debug.bat"
echo "Ready: $AURORA_OUT/AuroraPlayer.exe (copy the entire directory)"
