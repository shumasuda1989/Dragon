#!/bin/bash

counter=0

while [ $counter -lt $1 ]
do 
    ./Eth 192.168.1.89 24 16 100 1024 0 /dev/null >/dev/null

    sleep 1
    counter=$((counter + 1))
    echo $counter
done