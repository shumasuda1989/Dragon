#!/bin/bash
#
# @(#) Base script for making fundamental pedestal table.
# Parameters to be set: readdepth chreset N_event samplingfreq 
#                       freqlist offsetlist logfile elapselimit
# Run this script as below:
#   $ readdepth=40 N_event=1000 ./Mkoffset.sh [IP]
# or, 
#   $ export readdepth=40 N_event=1000
#   $ ./Mkoffset.sh [IP]
#
# Parameters not specified in an execution are set to default 
# values descrived below.
#

DragonIPdef=126
if [ "x$1" != "x" ]; then
    DragonIP=$1
else
    DragonIP=$DragonIPdef
fi

tmpfolder=$(mktemp -d)

[ -z $readdepth ] && readdepth=1024
[ -z $chreset ] && chreset=x14
[ -z $N_event ] && N_event=250
[ -z $samplingfreq ] && samplingfreq=65

table/Dragon_rbcp.sh -i $DragonIP
table/Dragon_rbcp.sh -d ${readdepth} $DragonIP
table/Dragon_rbcp.sh -s 0 $DragonIP
table/Dragon_rbcp.sh -wrb x109c $chreset $DragonIP
table/Dragon_rbcp.sh -f $samplingfreq $DragonIP
sleep 1

if [ -z $freqlist ]; then 
    if   [ $samplingfreq -eq 65 ]; then
freqlist=\
"1319998
1320130
1320262
1320394" # for ped_freq=65
    elif [ $samplingfreq -eq 67 ]; then
freqlist=\
"1333206
1333342
1333478
1333614" # for ped_freq=67
    else
	exit 1
    fi
fi

if [ -z $offsetlist ]; then
offsetlist="10 77"
fi

[ -z $logfile ] && logfile=/dev/null
exectime=$(date "+%-y%m%d%H%M%S")
[ -z $elapselimit ] && elapselimit=$((exectime+5))
index=0
Nrun=0
echo -n >$logfile

while [ $(date "+%-y%m%d%H%M%S") -lt $elapselimit ]
do

index=$((index+1))
if [ "$elapselimit" = "$((exectime+5))" ]; then
    index_form=""
else
    index_form=$(printf "_%05d" $index)
fi

echo start $index at $(date "+%m%d%H%M%S") >>$logfile

for freq in $freqlist
do

for offset in $offsetlist 
do
    if [ $Nrun -gt 0 ]; then batchflag="-b"; fi
    Nrun=$((Nrun+1))
    table/Dragon_rbcp.sh -pf $freq $DragonIP
    sleep 1
    table/Dragon_rbcp.sh -pfo $offset $DragonIP
    sleep 1
    ./EthDispCas 192.168.1.$DragonIP $N_event $tmpfolder/pede${freq}_${offset}${index_form}.dat $batchflag
    echo ""

done
done

echo finish $index at $(date "+%m%d%H%M%S") >>$logfile

if [ -z $index_form ]; then
    cat $tmpfolder/pede*.dat >ped${exectime}.dat
    ./Mkoffset ped${exectime}.dat offset.dat $readdepth
    ./Rootify ped${exectime}.dat
    ./Pedestal ped${exectime}.root offset${exectime}.root

else
    (cat $tmpfolder/pede*${index_form}.dat >ped${index_form}.dat && \
	./Rootify ped${index_form}.dat && \
	./Pedestal ped${index_form}.root offset${index_form}.root ) &
    pidarray[$!]=$!
fi

done

wait ${pidarray[@]}
rm -rf $tmpfolder

echo ""
echo "Finish!"