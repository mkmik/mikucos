echo executing system startup script

cd /System/Startup

echo system startup completed

echo
cat /System/Startup/greeting

shell user.com

shell
