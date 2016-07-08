#!/bin/bash
DragonIPdef=126
if [ "x$1" != "x" ]; then
    DragonIP=$1
else
    DragonIP=$DragonIPdef
fi

readdepth=40

table/Dragon_rbcp.sh -i $DragonIP
table/Dragon_rbcp.sh -f 65 $DragonIP
table/Dragon_rbcp.sh -d ${readdepth} $DragonIP
table/Dragon_rbcp.sh -s 0 $DragonIP
table/Dragon_rbcp.sh -wrb x109c x14 $DragonIP

sleep 3

# freqlist=\
# "1319998
# 1320130
# 1320262
# 1320394"
freqlist=\
"1333330
1333462
1333594
1333726"


for freq in $freqlist
do

for offset in `seq 1 2 131`
do
    table/Dragon_rbcp.sh -pf $freq $DragonIP
    sleep 1
    table/Dragon_rbcp.sh -pfo $offset $DragonIP
    sleep 1
    ./EthDispCas 192.168.1.$DragonIP 110 pede${freq}_${offset}.dat
#    ./EthDispCas 192.168.1.$DragonIP 10000 /dev/null

done
done

cat pede*.dat >ped4.dat

./Mkoffset ped4.dat offset4.dat $readdepth
./Rootify ped4.dat
./Pedestal ped4.root
