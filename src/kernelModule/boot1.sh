#!/bin/bash

sh ./connect_module/makenode.sh
sudo insmod ./connect_module/connModule.ko
./../../bin/main.out &
