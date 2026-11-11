#!/bin/sh
BASE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd) || exit 1
ROOT="$BASE/ShovelKnight_RGDSPlus";RT="$ROOT/runtime";LOGS="$ROOT/logs"
mkdir -p "$LOGS" "$ROOT/savedata/home" "$ROOT/savedata/config" "$ROOT/savedata/data" "$ROOT/savedata/game" || exit 1
LOCK="$ROOT/savedata/.running"
if ! mkdir "$LOCK" 2>/dev/null; then
 old=$(cat "$LOCK/pid" 2>/dev/null);case "$old" in ''|*[!0-9]*) old='';; esac
 [ -z "$old" ] || ! kill -0 "$old" 2>/dev/null || exit 1
 rm -f "$LOCK/pid";rmdir "$LOCK" 2>/dev/null && mkdir "$LOCK" || exit 1
fi
echo "$$" > "$LOCK/pid"
[ ! -f "$LOGS/Launcher.txt" ] || cp "$LOGS/Launcher.txt" "$LOGS/Previous_Launcher.txt"
exec > "$LOGS/Launcher.txt" 2>&1
LOWER=/sys/class/backlight/backlight/brightness;POWER=/sys/class/backlight/backlight/bl_power;VOLUME=/sys/class/anbernic_misc/openbor_volume
lower_saved='' power_saved='' volume_saved='' game_pid='' input_pid='' temp=''
cleanup(){
 trap - EXIT INT TERM HUP
 if [ -n "$game_pid" ]; then
  kill -TERM "$game_pid" 2>/dev/null || :;n=0
  while kill -0 "$game_pid" 2>/dev/null && [ "$n" -lt 3 ]; do sleep 1;n=$((n+1));done
  kill -KILL "$game_pid" 2>/dev/null || :;wait "$game_pid" 2>/dev/null || :
 fi
 [ -z "$input_pid" ] || { kill "$input_pid" 2>/dev/null || :;wait "$input_pid" 2>/dev/null || :; }
 [ -z "$temp" ] || python3 "$RT/performance-mode.py" restore "$temp/performance.json" >> "$LOGS/Performance.txt" 2>&1 || :
 [ -z "$power_saved" ] || printf '%s\n' "$power_saved" > "$POWER" 2>/dev/null || :
 [ -z "$lower_saved" ] || printf '%s\n' "$lower_saved" > "$LOWER" 2>/dev/null || :
 [ -z "$volume_saved" ] || printf '%s\n' "$volume_saved" > "$VOLUME" 2>/dev/null || :
 cat /proc/meminfo > "$LOGS/Memory_End.txt" 2>/dev/null || :
 echo "Restored state: volume=$(cat "$VOLUME" 2>/dev/null), lower=$(cat "$LOWER" 2>/dev/null), power=$(cat "$POWER" 2>/dev/null)"
 case "$temp" in /tmp/shovelknight-rgdsplus.*) rm -rf "$temp";; esac
 rm -f "$LOCK/pid";rmdir "$LOCK" 2>/dev/null || :
 python3 "$RT/collect-logs.py" "$ROOT" || :
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM HUP
printf 'Shovel Knight RGDS+ Beta 1\n';date;uname -srvm
case "$(uname -m)" in aarch64|arm64) :;; *) echo 'ARM64 Linux required';exit 2;; esac
command -v python3 >/dev/null 2>&1 || { echo 'Firmware Python 3 unavailable';exit 3; }
python3 -B "$RT/import-game.py" "$ROOT" || exit 3
python3 "$RT/verify-install.py" "$ROOT" || exit 3
for d in /var/run /run "/run/user/$(id -u)"; do
 for s in "$d"/wayland-*; do
  [ -S "$s" ] || continue;export XDG_RUNTIME_DIR="$d" WAYLAND_DISPLAY="${s##*/}";break 2
 done
done
[ -n "${WAYLAND_DISPLAY:-}" ] || { echo 'No firmware Wayland display found';exit 4; }
unset LD_PRELOAD SDL_DYNAMIC_API BOX64_EMULATED_LIBS BOX64_PREFER_EMULATED
export LANG=C LC_ALL=C
export LD_LIBRARY_PATH="$RT/gl4es:/lib:/usr/lib:/usr/lib64"
export BOX64_DYNAREC_STRONGMEM=1 BOX64_DYNAREC_ALIGNED_ATOMICS=1
unset BOX64_BOX32
export BOX64_DYNAREC=1 BOX64_NOPERSONA32BITS=1 BOX64_DYNACACHE=0 BOX64_LOG=1 BOX64_SHOWSEGV=1
export BOX64_LD_LIBRARY_PATH="$RT/libs.x64:$ROOT/game/64/lib"
unset BOX64_EMULATED_LIBS
export BOX64_LD_PRELOAD="$RT/libs.x64/libshovel_rgds.so"
export BOX64_LIBGL="$RT/gl4es/libGL.so.1"
export SHOVEL_NATIVE_GLES=1
export SDL_VIDEODRIVER=wayland SDL_AUDIODRIVER=alsa SDL_OPENGL_ES_DRIVER=1 SDL_VIDEO_GL_DRIVER=/lib/libGLESv2.so.2
export LIBGL_NOARBPROGRAM=0
export LIBGL_ES=2 LIBGL_GL=21 LIBGL_FB=0 LIBGL_EGL=/lib/libEGL.so.1 LIBGL_GLES=/lib/libGLESv2.so.2 BRAID_RGDS_GLES=1
# Use firmware native SDL2 and the guest compatibility preload.
unset SDL_DYNAMIC_API
export SDL_GAMECONTROLLERCONFIG=$(cat "$RT/controller.txt")
export SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS=1
unset DISPLAY
export XDG_CONFIG_HOME="$ROOT/savedata/config" XDG_DATA_HOME="$ROOT/savedata/data"
# Keep SDL preferences private to this port.
export SHOVEL_SAVED_HOME="$ROOT/savedata/home"
export SHOVEL_VOLUME_NODE="$VOLUME"
export SHOVEL_PREF_PATH="$ROOT/savedata/game"
temp=$(mktemp -d /tmp/shovelknight-rgdsplus.XXXXXX) || exit 5
export SHOVEL_INPUT_STATE="$temp/input.bin" SHOVEL_EXIT_REQUEST="$temp/exit-request"
for spec in "$VOLUME" "$LOWER" "$POWER"; do
 [ -r "$spec" ] && [ -w "$spec" ] || { echo "Hardware setting unavailable: $spec";continue; }
 value=$(cat "$spec");case "$value" in ''|*[!0-9]*) continue;; esac
 case "$spec" in "$VOLUME") [ "$value" -gt 10 ] || volume_saved="$value";; "$LOWER") lower_saved="$value";printf '0\n' > "$LOWER";; "$POWER") power_saved="$value";printf '4\n' > "$POWER";; esac
done
python3 -u "$RT/device.py" "$SHOVEL_INPUT_STATE" "$SHOVEL_EXIT_REQUEST" > "$LOGS/Controls.txt" 2>&1 & input_pid=$!
n=0;while [ ! -s "$SHOVEL_INPUT_STATE" ] && [ "$n" -lt 30 ]; do sleep .1;n=$((n+1));done
[ -s "$SHOVEL_INPUT_STATE" ] || { echo 'Input mailbox initialization failed';exit 6; }
python3 "$RT/performance-mode.py" apply "$temp/performance.json" > "$LOGS/Performance.txt" 2>&1 || :
cat /proc/meminfo > "$LOGS/Memory_Start.txt"
chmod +x "$RT/box64-box32" "$ROOT/game/64/ShovelKnight" 2>/dev/null || :
ulimit -c 0
cd "$ROOT/game" || exit 7
LD_PRELOAD="$RT/librgds_video.so" "$RT/box64-box32" ./64/ShovelKnight > "$LOGS/Game.txt" 2>&1 & game_pid=$!
# SDL_QUIT is injected by the bridge; retain a timeout for hung exits.
while kill -0 "$game_pid" 2>/dev/null; do
 if [ -f "$SHOVEL_EXIT_REQUEST" ]; then
  n=0;while kill -0 "$game_pid" 2>/dev/null && [ "$n" -lt 5 ]; do sleep 1;n=$((n+1));done
  kill -TERM "$game_pid" 2>/dev/null || :
  n=0;while kill -0 "$game_pid" 2>/dev/null && [ "$n" -lt 3 ]; do sleep 1;n=$((n+1));done
  kill -KILL "$game_pid" 2>/dev/null || :
 fi
 sleep 1
done
wait "$game_pid";result=$?;game_pid='';echo "Game exit code: $result";exit "$result"
