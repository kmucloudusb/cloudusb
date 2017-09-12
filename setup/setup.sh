sudo cat keyboard > /etc/default/keyboard
sudo cat config.txt > /boot/config.txt
sudo cat modules > /etc/modules
sudo apt-get install vim -y
sudo cat vimrc > ~/.vimrc
sudo -u pi git clone https://github.com/lch01387/for_pi /home/pi/for_pi
sudo wget https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source -O /usr/bin/rpi-source && sudo chmod +x /usr/bin/rpi-source && /usr/bin/rpi-source -q --tag-update
sudo apt-get install bc -y
sudo apt-get install libncurses5-dev -y
sudo dd if=/dev/zero of=/piusb.bin bs=1M count=1K
sudo mkdosfs -F 32 -s 8 -S 512 /piusb.bin
sudo apt-get install python-pip -y
sudo pip install --upgrade google-api-python-client
sudo -u pi rpi-source
sudo -u pi make -C ~/MyCloudUSB/MYFAT/MYFAT/MYFAT
