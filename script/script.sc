sudo cat keyboard > /etc/default/keyboard
sudo cat network >> /etc/wpa_supplicant/wpa_supplicant.conf
sudo cat dtoverlay >> /boot/config.txt
sudo cat modules >> /etc/modules
sudo apt-get update
sudo apt-get install git -y
sudo wget https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source -O /usr/bin/rpi-source && sudo chmod +x /usr/bin/rpi-source && /usr/bin/rpi-source -q --tag-update
sudo apt-get install bc -y
sudo apt-get install libncurses5-dev -y
rpi-source
