echo "Configuring TAP Interface"
read -p "Enter IP Address [Default 192.1.1.1]:" ipAddr
ipAddr=${ipAddr:-192.1.1.1}
sudo ip tuntap add tap0 mode tap user $UID
sudo ifconfig tap0 $ipAddr netmask 255.255.255.0
ifconfig
