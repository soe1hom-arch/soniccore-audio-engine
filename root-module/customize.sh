#!/system/bin/sh

ui_print "*******************************"
ui_print " SonicCore Root Audio Foundation"
ui_print "*******************************"
ui_print "Disabled by default: no audio config will be replaced."

mkdir -p /data/adb/soniccore/backup
mkdir -p /data/adb/soniccore/logs

for config in /system/etc/audio_effects.xml /vendor/etc/audio_effects.xml /odm/etc/audio_effects.xml /system/etc/audio_effects.conf /vendor/etc/audio_effects.conf; do
    if [ -f "$config" ]; then
        safe_name=$(echo "$config" | sed 's#/#_#g')
        if [ ! -f "/data/adb/soniccore/backup/$safe_name" ]; then
            cp -p "$config" "/data/adb/soniccore/backup/$safe_name"
        fi
    fi
done

set_perm "$MODPATH/service.sh" 0 0 0755
set_perm "$MODPATH/uninstall.sh" 0 0 0755

