#!/system/bin/sh

LOG_DIR=/data/adb/soniccore/logs
mkdir -p "$LOG_DIR"
echo "$(date '+%Y-%m-%d %H:%M:%S') SonicCore module removed; stock files were not overwritten" >> "$LOG_DIR/uninstall.log"

