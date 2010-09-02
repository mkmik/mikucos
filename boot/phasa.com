ip -a 192.168.0.15 -d /Devices/Pcnet32/0 -p 1
echo phasa testing
net_test -w
