#!/bin/bash
DragonIPdef=126
if [ "x$1" != "x" ]; then
    DragonIP=$1
else
    DragonIP=$DragonIPdef
fi

readdepth=1024

table/Dragon_rbcp.sh -i $DragonIP
table/Dragon_rbcp.sh -d ${readdepth} $DragonIP
table/Dragon_rbcp.sh -s 0 $DragonIP
table/Dragon_rbcp.sh -wrb x109c x14 $DragonIP
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

logfile=mkoffset.log
index=0

echo "" >$logfile

while [ $(date "+%m%d%H%M%S") -lt 0722130000 ]
do

index=$((index+1))
index_form=$(printf "%05d" $index)

echo start $index at $(date "+%m%d%H%M%S") >>$logfile

for freq in $freqlist
do

for offset in 10 77 
do
    table/Dragon_rbcp.sh -pf $freq $DragonIP
    sleep 1
    table/Dragon_rbcp.sh -pfo $offset $DragonIP
    sleep 1
    ./EthDispCas 192.168.1.$DragonIP 150 pede${freq}_${offset}_${index_form}.dat
#    ./EthDispCas 192.168.1.$DragonIP 10000 /dev/null

done
done

echo finish $index at $(date "+%m%d%H%M%S") >>$logfile

#./Mkoffset ped_${index_form}.dat offset_${index_form}.dat 1024
(cat pede*_${index_form}.dat >ped_${index_form}.dat && \
    ./Rootify ped_${index_form}.dat && \
    ./Pedestal ped_${index_form}.root offset_${index_form}.root && \
    rm pede*_${index_form}.dat ) &

done