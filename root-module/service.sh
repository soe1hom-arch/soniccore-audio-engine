#!/system/bin/sh

LOG_DIR=/data/adb/soniccore/logs
mkdir -p "$LOG_DIR"
log_file="$LOG_DIR/service.log"

log_line() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') $*" >> "$log_file"
}

until [ "$(getprop sys.boot_completed)" = "1" ]; do
    sleep 2
done

enabled=$(getprop persist.soniccore.enabled)
adapter=$(getprop persist.soniccore.adapter)
if [ "$enabled" != "1" ] || [ -z "$adapter" ] || [ "$adapter" = "none" ]; then
    log_line "SonicCore disabled; stock audio path unchanged"
    exit 0
fi

log_line "SonicCore requested adapter=$adapter"
log_line "No generic adapter installed; refusing unsafe audio changes"
exit 0

