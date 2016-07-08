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
table/Dragon_rbcp.sh -wrb x109c 4 $DragonIP

sleep 3

#freqlist=\
#"1319998
# 1320130
# 1320262
# 1320394"
freqlist=\
"1333206"


for freq in $freqlist
do

for offset in 10 20
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

./Mkoffset ped.dat offset.dat $readdepth
./Rootify ped.dat
./Pedestal ped.root
