echo "Installing NCN26010 10-Base-T1S Ethernet Driver"
echo "Installing dependencies . . ."
sudo apt install libreadline6 libreadline6-dev
sudo apt install pigpio
echo "Compiling Driver . . ."
make
./netconf.sh
echo "Installation Complete"
