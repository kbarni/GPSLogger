#!/bin/bash

if test "$#" -ne 2; then
    echo "Useage: ./convert.sh file.ubx base.21n"
    exit
fi

ubxfile="$1"
obsfile="${ubxfile%.ubx}.obs"
ppkfile="${ubxfile%.ubx}.pos"
baseobs="$2"
basenav="${baseobs%.21o}.21n"

# Decode UBX file to OBS
echo -e "\033[1m\nPPK conversion utility for UBX files\n(C)2021 Barna Keresztes / IMS Bordeaux\n\n--> Decoding UBX file $ubxfile to $obsfile...\n\033[0m"
./convbin -r ubx -o $obsfile $ubxfile

# Run PPK correction
echo -e "\033[1m\n--> Running PPK correction on $obsfile using $baseobs and $basenav...\n\033[0m"
./rnx2rtkp -k rtkpost.conf -o $ppkfile $obsfile $baseobs $basenav

echo "Ready."
