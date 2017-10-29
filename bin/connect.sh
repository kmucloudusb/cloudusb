#!/bin/bash

sh ./../src/kernelModule/connect_module/makenode.sh
sudo insmod ./../src/kernelModule/connect_module/connModule.ko
./main.out &
