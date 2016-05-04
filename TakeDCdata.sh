#!/bin/bash
DragonIPdef=22
if [ "x$1" != "x" ]; then
    DragonIP=$1
else
    DragonIP=$DragonIPdef
fi

DIR=data/dccal/adc_4ch

table/Dragon_rbcp.sh -i $DragonIP
sleep 3

freqlist=\
"1319998"
# 1320064
# 1320130
# 1320196
# 1320262
# 1320328
# 1320394
# 1320460"

for DCCOMMONext in 0.8 #0.6 1.0
do

varDC=-0.2

for N_DC in `seq 27`
do
    varDC=$(echo "scale=2; $varDC + 0.05" | bc)
    DCCOMMONext=$DCCOMMONext table/Dragon_rbcp.sh -dc $varDC $DragonIP

    filepre=com$(printf "%02d" $(echo "10 * $DCCOMMONext /1" | bc) )_dc$(printf "%03d" $(echo "100 * $varDC /1" | bc) )_
    echo $filepre
    echo ""
for freq in $freqlist
do

    table/Dragon_rbcp.sh -pf $freq $DragonIP
    echo ""
    sleep 1
    ./EthDispCas 192.168.10.$DragonIP 1000 $DIR/$filepre$freq.dat
    echo ""

done

cat $DIR/$filepre*.dat >$DIR/$filepre.dat

./Rootify $DIR/$filepre.dat
done
done
