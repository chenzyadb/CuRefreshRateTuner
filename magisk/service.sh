#!/system/bin/sh
BASE_PATH=$(dirname $0)
while [ ! -e /sdcard/Android/CuRefreshRateTuner/config.json ] ; do
	sleep 1
done
${BASE_PATH}/CuRefreshRateTuner "/sdcard/Android/CuRefreshRateTuner/config.json" "/sdcard/Android/CuRefreshRateTuner/log.txt"
