sudo cat keyboard > /etc/default/keyboard
sudo cat config.txt > /boot/config.txt
sudo cat modules > /etc/modules
sudo -u pi git clone https://github.com/kmucloudusb/cloudusb /home/pi/cloudusb
sudo wget https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source -O /usr/bin/rpi-source && sudo chmod +x /usr/bin/rpi-source && /usr/bin/rpi-source -q --tag-update
sudo apt-get install bc -y
sudo apt-get install libncurses5-dev -y
sudo touch /piusb.bin
sudo apt-get install python-pip -y
sudo pip install --upgrade google-api-python-client
sudo -u pi rpi-source
