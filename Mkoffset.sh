#!/bin/bash
DragonIPdef=146
if [ "x$1" != "x" ]; then
    DragonIP=$1
else
    DragonIP=$DragonIPdef
fi

table/Dragon_rbcp.sh -i $DragonIP
table/Dragon_rbcp.sh -f 65 $DragonIP
sleep 3

freqlist=\
"1319998
1320130
1320262
1320394" # for ped_freq=65
# freqlist=\
# "1333206
# 1333342
# 1333478
# 1333614" # for ped_freq=67


for freq in $freqlist
do

for offset in 10 77 
do
    table/Dragon_rbcp.sh -pf $freq $DragonIP
    sleep 1
    table/Dragon_rbcp.sh -pfo $offset $DragonIP
    sleep 1
    ./EthDispCas 192.168.1.$DragonIP 2500 ped${freq}_${offset}.dat
#    ./EthDispCas 192.168.1.$DragonIP 10000 /dev/null

done
done

cat ped*.dat >ped.dat

./Mkoffset ped.dat offset.dat 1024
./Rootify ped.dat
./Pedestal ped.root
