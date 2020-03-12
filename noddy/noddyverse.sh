#!/bin/bash
# Basic while loop
counter=1
while [ $counter -le 9999 ]
do
	NOW=$(date +"%y-%m-%d-%H-%M-%S-%N")
	echo $NOW
    ./noddy3.exe dikefault.his $NOW.his RANDOM 
    ((counter++))
done
