ui_print "- CuRefreshRateTuner Magisk Module"
ui_print "- Installing..."

ANDROID_SDK=$(getprop ro.build.version.sdk)
DEVICE_ABI=$(getprop ro.product.cpu.abi)
if [ ${ANDROID_SDK} -lt 29 || ${DEVICE_ABI} != "arm64-v8a" ] ; then
	abort "- Your device does not meet the requirement, Abort."
fi

unzip -o "${ZIPFILE}" -x 'META-INF/*' -d ${MODPATH} >&2
chmod -R 7777 ${MODPATH}

if [ ! -d /sdcard/Android/CuRefreshRateTuner ] ; then
	mkdir -p /sdcard/Android/CuRefreshRateTuner
	cp -f ${MODPATH}/config.json /sdcard/Android/CuRefreshRateTuner/config.json
	ui_print "- Config file released at \"/sdcard/Android/CuRefreshRateTuner/config.json\""
fi
rm -f ${MODPATH}/config.json

ui_print "- Install finished."
