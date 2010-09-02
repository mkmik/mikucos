echo mounting 0:0

mount -t fat /Devices/Harddisk/IDE/0/part1 /System/mnt

echo done
