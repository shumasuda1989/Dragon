#!/bin/bash

DEBUG_FLAG=0

DragonIPdef=0
DragonIP=$DragonIPdef

usage(){
    echo "Usage: $0 [OPTIONS] IPaddr(default:$DragonIPdef)"
    echo "Options:"
    echo -e "-i      \t: initialize by default value"
    echo -e "-it     \t: initialize L0/L1 trigger"
    echo -e "-t val  \t: trigger select(0-8 default:2)"
    echo -e "-d val  \t: read depth(default: 1024)"
    echo -e "-s val  \t: stop timing from trigger"
    echo -e "-f val  \t: sampling frequency(default: 65~1GHz)"
    echo -e "-pf val  \t: pedestal frequency(default: 444444=300Hz)"
    echo -e "-pfo val  \t: pedestal frequency offset"
    echo -e "-a[a,0-6] val\t: attenuator gain(0.6-1.35 (++0.05))"
    echo -e "-c[a,0-6] val\t: clipping level(68-900 [mV])"
    echo -e "-d[a,0-6] val\t: delay(0-5.75ns (++0.25))"
    echo -e "-dc val  \t: DC offset (-0.05 - 0.95 [V] default: 0 V)"
    echo -e "-l0[a,0-6] val\t: L0 IPR threshold(1-1023 512~0V)"
    echo -e "-t[hl][012] val\t: L1 sum threshold(1-1023 1023~1.2V)"
    echo -e "-tpe 6543210P \t: SCB test pulse enable(base 2)"
    echo -e "-tpg[a,0-6] val\t: SCB test pulse gain(0-83dB)"
    echo -e "-p   6543210P \t: PMT power enable(base 2)"
    echo -e "-h[a,0-6] val\t: PMT HV(0-1500V)"
    echo -e "-wr[bsw] add val: write Dragon register"
    echo -e "-rd add len \t: read Dragon register"
    echo -e "-rdo add len .txt: read Dragon register and output the value"
    echo -e "-m      \t: monitor HV, curent, etc.."
    exit 1
}

RBCP(){
    echo "$1
          quit" | \
    $HOME/Dragon/rbcp 192.168.1.$DragonIP 4660
}

command_rbcp(){
    if [ "$1" = -nq ]; then QMODE=0; shift; else QMODE=1; fi

    if [ $DEBUG_FLAG -ne 0 ]; then
	echo ""
	echo "########################################################"
	echo "#####  " $1
	echo "########################################################"
	echo ""
	if [ "x$(echo $1| grep load )" != "x" ];then
	    cat $tmpfile ; echo ""
	fi
    else

	if [ $QMODE -eq 1 ]; then
	    RBCP "$1" >/dev/null 
	else
	    RBCP "$1" 
	fi
	if [ $? -ne 0 ]; then
	    echo ""; echo "RBCP ERROR"
	    usage
	fi

    fi
}

rbcpread(){
    # if [ $DEBUG_FLAG -eq 0 ] || [ $DEBUG_FLAG -eq 2 ] ; then
    # 	read $1 <rcvdBuf.txt 
    # fi
    # if [ $DEBUG_FLAG -eq 0 ] && [ -f rcvdBuf.txt ]; then rm rcvdBuf.txt; fi

    RBCP "rd $1 $2" | grep "0x" | sed -e 's/[0x........]//g' -e 's/- //g' | sed 's/ //g'
}

rbcpload(){
    if [ -s $1 ]; then 
	command_rbcp "load $tmpfile"
	if [ $? -eq 0 ]; then
	    : >$1
	fi
    else
	echo $1 is 0 byte or does not exist
    fi
}

ch_check(){
    if [ "$ch" = a ] || ( expr "$ch" + 1 >/dev/null 2>&1 \
	&& [ "$ch" -ge 0 ] && [ "$ch" -lt 7 ] ); then :
    else
	echo $ch is invalid value 
	usage
    fi
}

int_check(){
    if expr "$1" + 1 >/dev/null 2>&1 \
	&& [ "$1" -ge $2 ] && [ "$1" -le $3 ]; then :
    else 
	echo $1 is invalid value 
	usage
    fi
}

num_check(){
    if [ $(echo "$1 < $2" | bc) -eq 1 ] \
	|| [ $(echo "$1 > $3" | bc) -eq 1 ]
	then 
	echo $1 is invalid value 
	usage
    fi
}

flag_check(){
    while [ $(rbcpread $1 1) -ne 0 ]; do
	echo "writing command not done!"
	usleep 100000
    done
}

if [ $# -eq 0 ]; then usage; fi
#if [ $# -eq 0 ]; then flag_check; exit; fi

DupFlag=0
DEFAULTFlag=0
TRIGGERINITFlag=0
HVFlag=0
HVnAFlag=0
MonitorFlag=0
FirstDoneFlag=0
RBCPWrFlag=0
RBCPRdFlag=0

while [ "$#" -gt 0 ]
do

  case "$1" in
      -i)
	  if [ $FirstDoneFlag -eq 1 ]; then
	      echo ERROR: -i option MUST be written firstly
	      exit 1
	  fi

	  TRIGGER_SELECT=2
	  READDEPTH=1024
	  STOP_FROM_TRIG=530  #79 #for AT    125   530 #for TP   
	  SAMP_FREQ=65
	  PEDE_FREQ=444444
	  PEDE_FREQ_OFFSET=50

	  DEFAULTFlag=1
	  shift
	  ;;
      -it)
	  for i in `seq 0 6`; do
	      THLEL0[$i]=0
	      Att_G_b[$i]=7
	      Att_G_b_Gain[$i]=$(echo "scale=2; (27 - ${Att_G_b[$i]}) * 0.05" | bc)
	      Clip_Sel_b[$i]=1
	      Ib_ClipVb[$i]=63
	  done
	  DELAY=(21 21 2a 2a 2a 2a 2a)
	  DAC_TH_H=(1023 1023 1023)
	  DAC_TH_L=(1023 1023 1023)

	  TRIGGERINITFlag=1
	  shift
	  ;;
      -t)
	  int_check "$2" 0 4

	  TRIGGER_SELECT=$2
	  shift 2
	  ;;
      -d)
	  int_check "$2" 0 4095

	  READDEPTH=$2
	  shift 2
	  ;;
      -s)
	  int_check "$2" 0 2047

	  STOP_FROM_TRIG=$2
	  shift 2
	  ;;
      -f)
	  int_check "$2" 0 255

	  SAMP_FREQ=$2
	  shift 2
	  ;;
      -pf)
	  int_check "$2" 0 4294967295

	  PEDE_FREQ=$2
	  shift 2
	  ;;

      -pfo)
	  int_check "$2" 0 132

	  PEDE_FREQ_OFFSET=$2
	  shift 2
	  ;;

      -a?)
	  ch=$(echo "$1" | sed -e 's/-a//')
	  ch_check

	  num_check "$2" 0.6 1.35

	  if [ "$ch" = a ] ; then
	      for i in `seq 0 6`; do
		  Att_G_b[$i]=$(echo "( 1.35 - $2 )/ 0.05" | bc)
		  Att_G_b_Gain[$i]=$(echo "scale=2; (27 - ${Att_G_b[$i]}) * 0.05" | bc)
	      done

	  else
	      Att_G_b[$ch]=$(echo "( 1.35 - $2 )/ 0.05" | bc)
	      Att_G_b_Gain[$ch]=$(echo "scale=2; (27 - ${Att_G_b[$ch]}) * 0.05" | bc)

	  fi
	  shift 2
	  ;;
      -c?)
	  ch=$(echo "$1" | sed -e 's/-c//')
	  ch_check

	  int_check "$2" 68 900

	  if [ $(echo "$2 <= 160" | bc) -eq 1 ]; then
	      Clip_Sel_b_tmp=3
	      Ib_ClipVb_tmp=$(($2 * 57 / 92 - 831 / 23))
	  elif [ $(echo "$2 <= 270" | bc) -eq 1 ]; then
	      Clip_Sel_b_tmp=2
	      Ib_ClipVb_tmp=$(($2 * 29 / 84 - 423 / 14))
	  else
	      Clip_Sel_b_tmp=1
	      Ib_ClipVb_tmp=$(($2 * 29 / 350 - 81 / 7))
	  fi

	  if [ "$ch" = a ] ; then
	      for i in `seq 0 6`; do
		  Clip_Sel_b[$i]=$Clip_Sel_b_tmp
		  Ib_ClipVb[$i]=$Ib_ClipVb_tmp
	      done

	  else
	      Clip_Sel_b[$ch]=$Clip_Sel_b_tmp
	      Ib_ClipVb[$ch]=$Ib_ClipVb_tmp

	  fi
	  shift 2
	  ;;
      -dc)
	  #num_check "$2" -0.05 0.95
	  num_check "$2" -0.5 1.2

	  COMMONdef=0.8 #[V]
	  if [ "x$DCCOMMONext" != "x" ]; then
	      COMMON=$DCCOMMONext
	  else
	      COMMON=$COMMONdef
	  fi
	  DAC_CALP=$(echo "scale=10;($COMMON+($2)/2.)*2^16/2.5+0.5" | bc)
	  DAC_CALN=$(echo "scale=10;($COMMON-($2)/2.)*2^16/2.5+0.5" | bc)
	  DAC_CALP=$(echo "$DAC_CALP/1" | bc)
	  DAC_CALN=$(echo "$DAC_CALN/1" | bc)
	  unset COMMON

	  shift 2
	  ;;
      -d?)
	  ch=$(echo "$1" | sed -e 's/-d//')
	  ch_check

	  num_check "$2" 0 5.75

	  A0=$(echo "$2%1" | bc)
	  A0=$(echo "${A0}*4/1" | bc)
	  A1=$(echo "$2/1" | bc)
	  B1=(2 D 9 6 8 7)
	  if [ $A1 -lt 4 ]; then
	      B0=(1 A 5 E)
	  else
	      B0=(3 8 7 C)
	  fi

	  if [ "$ch" = a ] ; then
	      for i in `seq 0 6`; do
		  DELAY[$i]="${B1[${A1}]}${B0[${A0}]}"
	      done
	  else
	      DELAY[$ch]="${B1[${A1}]}${B0[${A0}]}"
	  fi
	  shift 2
	  ;;
      -l0?)
	  ch=$(echo "$1" | sed -e 's/-l0//')
	  ch_check

	  int_check "$2" 1 1023

	  if [ $ch = a ]; then
	      for i in {0..6}; do
		  THLEL0[$i]=$2
	      done
	  else
	      THLEL0[$ch]=$2
	  fi

	  shift 2
	  ;;
      -th?|-tl?)
	  DACch=$(echo "$1" | sed -e 's/-t.//')
	  int_check $DACch 0 2
	  int_check "$2" 1 1023

	  if [ $(echo "$1" | cut -c 3) = h ]; then  
		  DAC_TH_H[$DACch]=$2 ; else 
		  DAC_TH_L[$DACch]=$2 ; fi

	  shift 2
	  ;;
      -tpg?)
	  ch=$(echo "$1" | sed -e 's/-tpg//')
	  ch_check

	  int_check "$2" 0 83

	  if [ "$2" -gt 63 ]; then
	      GAIN_TRUE=$(($2 + 128+63-83))
	  else
	      GAIN_TRUE=$2
	  fi

	  if [ "$ch" = a ]; then
#	      ENABLED_Ch=$((0xff))
	      for i in `seq 0 6`; do
		  TP_GAIN[$i]=$GAIN_TRUE
	      done
	  else
#	      if [ -z "$ENABLED_Ch" ]; then ENABLED_Ch=1; fi
#	      if [ "$((ENABLED_Ch & ( 1<<(ch+1) ) ))" -eq 0 ]; then
#		  ENABLED_Ch="$((ENABLED_Ch +( 1<<(ch+1) ) ))"
#	      fi
	      TP_GAIN[$ch]=$GAIN_TRUE
	  fi

	  shift 2
	  ;;
      -tpe)
	  if [ ${#2} -gt 8 ] || [ "$(echo $2 | sed -e 's/[01]//g')x" != x ]
	  then 
	      echo $2 is invalid value 
	      usage
	  fi

	  ENABLED_Ch=$(echo "obase=16;$((2#$2))"|bc)

	  shift 2
	  ;;
      -p)
	  if [ ${#2} -gt 8 ] || [ "$(echo $2 | sed -e 's/[01]//g')x" != x ]
	  then 
	      echo $2 is invalid value 
	      usage
	  fi

	  PMTen_Ch=$(echo "obase=16;$((2#$2))"|bc)

	  shift 2
	  ;;
      -h?)
	  HVFlag=1
	  ch=$(echo "$1" | sed -e 's/-h//')
	  ch_check

	  num_check "$2" 0 1500

	  if [ $ch = a ]; then
	      for i in {0..6}; do
		  HV[$i]=$(echo "scale=10;($2+(1500/4095/2))*4095/1500"| bc)
		  HV[$i]=$(echo "${HV[$i]}/1" | bc)
		  HVcont[$i]=$(echo "scale=2;${HV[$i]}*1512/4095" | bc)
	      done
	  else
	      HVnAFlag=1
	      HV[$ch]=$(echo "scale=10;($2+(1500/4095/2))*4095/1500"| bc)
	      HV[$ch]=$(echo "${HV[$ch]}/1" | bc)
	      HVcont[$ch]=$(echo "scale=2;${HV[$ch]}*1512/4095" | bc)
	  fi
	  shift 2
	  ;;

      -m)
	  MonitorFlag=1
	  shift
	  ;;

      -wr?)
	  int_check $((0$2)) $((0x1000)) $((0x10cf))
	  RBCPWrFlag=1
	  com=$(echo "$1" | sed -e 's/^-//')
	  RBCPCOM="$com $2 $3" 
	  shift 3
	  ;;
      -rd)
	  int_check $((0$2)) $((0x0)) $((0x10cf))
	  RBCPRdFlag=1
	  RBCPADDR=$2
	  RBCPLENG=$3
	  TMPBUFFILENAME=""
	  shift 3
	  ;;
      -rdo)
	  int_check $((0$2)) $((0x0)) $((0x10cf))
	  RBCPRdFlag=1
	  RBCPADDR=$2
	  RBCPLENG=$3
	  RBCPCOM="rd $2 $3"
	  TMPBUFFILENAME=$4
	  shift 4
	  ;;
      -*)
	  usage
	  ;;
      *)
	  if [ $DupFlag -eq 1 ]; then 
	      usage
	  fi
	  int_check "$1" 0 255
	  DragonIP=$1
	  DupFlag=1
	  shift
	  ;;
  esac

  FirstDoneFlag=1

done

echo DragonIP=192.168.1.$DragonIP

if [ "$RBCPWrFlag" -eq 1 ]; then 
    command_rbcp -nq "$RBCPCOM"
    exit
fi

if [ "$RBCPRdFlag" -eq 1 ]; then
    command_rbcp -nq "rd $RBCPADDR $RBCPLENG"
    if [ "x$(echo "$TMPBUFFILENAME" | grep ".txt$")" != "x" ]; then
	rbcpread $RBCPADDR $RBCPLENG >$TMPBUFFILENAME
    fi
#	  rbcpread tmptmptmp
#	  echo $2 $tmptmptmp
    exit
fi
 
#tmpfile=$(date +"tmp%m%d%H%M%S%N")
tmpfile=$(date +"tmp%m%d%H%M%S")
if [ -f $tmpfile ]; then rm $tmpfile; fi
touch $tmpfile

if [ "$DEFAULTFlag" -eq 1 ]; then

echo \
"#trigger enable
wrb x101e 1

#ADC set
wrs x1012 x0014
wrb x1011 x00
wrb x1010 xff
#DRS CLKOUT ENABLE
wrb x1099 1

#cascade num
wrb x109c x14
#wrb x109c 4

#DRS4 PLL check config
wrb x109a xff
#wrb x109a xfe
#wrb x109a x00

#DRS Reference Clock Select
#local clock
wrb x109E x00
#external clock
#wrb x109E x01

#DRS Timing Calibration enable
#wrb x1099 x03
#wrb x109B xff
#disable
wrb x1099 x00
wrb x109B x00

" >>$tmpfile

fi


if [ -n "$TRIGGER_SELECT" ]; then
    echo "#trigger select" >>$tmpfile
    echo "wrb x100b $TRIGGER_SELECT" >>$tmpfile
    echo "" >>$tmpfile
    echo TRIGGER_SELECT=$TRIGGER_SELECT
fi

if [ -n "$PEDE_FREQ" ]; then
    echo \
"#pedrun freq
wrw x100c $PEDE_FREQ
#wrw x100c 1319998  ##fixed
#wrw x100c 1320064  ##drift
#wrw x100c 1320130
#wrw x100c 1320196
#wrw x100c 1320262
#wrw x100c 1320328
#wrw x100c 1320394
#wrw x100c 1320460
" >>$tmpfile
    echo PEDE_FREQ=$PEDE_FREQ
fi

if [ -n "$PEDE_FREQ_OFFSET" ]; then
    echo \
"#pedrun freq offset
wrs x10cd $PEDE_FREQ_OFFSET
#wrs x10cd 1
#wrs x10cd 67
" >>$tmpfile
    echo PEDE_FREQ_OFFSET=$PEDE_FREQ_OFFSET
fi

if [ -n "$READDEPTH" ]; then
    echo "#read depth (should be <1023)" >>$tmpfile
    echo "wrs x1090 $READDEPTH" >>$tmpfile
    echo READDEPTH=$READDEPTH
fi

if [ -n "$STOP_FROM_TRIG" ]; then
    echo "#stop from trig" >>$tmpfile
    echo "wrs x1092 $STOP_FROM_TRIG" >>$tmpfile
    echo STOP_FROM_TRIG=$STOP_FROM_TRIG
fi

if [ -n "$SAMP_FREQ" ];then
    echo "#Sampling Freq" >>$tmpfile
    echo "wrb x1094 $SAMP_FREQ" >>$tmpfile
    echo SAMP_FREQ=$SAMP_FREQ
fi

if [ -n "$DAC_CALP" ];then
    echo "" >>$tmpfile
    echo "wrs x1087 $DAC_CALP" >>$tmpfile
    echo "wrs x1089 $DAC_CALN" >>$tmpfile
    echo "wrb x1080 xff" >>$tmpfile
    echo DAC_CALP=$DAC_CALP=$(echo "scale=4;$DAC_CALP*2.5/2^16" | bc)
    echo DAC_CALN=$DAC_CALN=$(echo "scale=4;$DAC_CALN*2.5/2^16" | bc)
    echo V_CAL   =$(echo "scale=4;($DAC_CALP-$DAC_CALN)*2.5/2^16" | bc)
fi

rbcpload $tmpfile

########################
### Analog Trigger
########################

if [ "$TRIGGERINITFlag" -eq 1 ]; then
echo \
"
#Analog Trigger Iinitialize Chain
#L0###########
#global config (sum)
wrb x1050 4
wrs x1051 x0436
wrb x1040 xff
#rd x1040 1
#wrb x1041 xff
#rd x1053 3
" >>$tmpfile

rbcpload $tmpfile
flag_check x1040

fi

for ch in `seq 0 6`; do
if [ -n "${THLEL0[$ch]}"  ]; then
echo \
"#ch$ch thresh
wrb x1050 $((24 + ch))
wrs x1051 ${THLEL0[$ch]}
wrb x1040 xff
rd x1040 1" >>$tmpfile
L0THREFlag="Done"
rbcpload $tmpfile
flag_check x1040
fi
done
echo "" >>$tmpfile

if [ "x$L0THREFlag" != "x" ]; then
    echo L0_THRE=${THLEL0[@]}
fi


for ch in `seq 0 6`; do
if   [ -n "${Att_G_b[$ch]}" ] && [ -n "${Ib_ClipVb[$ch]}" ]; then
    ConfRes[$ch]="$((${Att_G_b[$ch]} << 9 ))"
    ConfRes[$ch]="$((${ConfRes[$ch]} + ( ${Clip_Sel_b[$ch]} << 13 ) ))"
    ConfRes[$ch]=$((${ConfRes[$ch]} + ${Ib_ClipVb[$ch]}))
    ConfRes[$ch]=$(echo "obase=16;${ConfRes[$ch]}" | bc)
    AttFlag="Done"
    ClipFlag="Done"

elif [ -n "${Att_G_b[$ch]}" ] || [ -n "${Ib_ClipVb[$ch]}" ]; then
    #ConfRes[$ch]=$((0x2E3F))
    command_rbcp "wrb x1050 1$ch"
    command_rbcp "wrb x1041 xff"
    command_rbcp "rd x1054 2"
    # rbcpread "ConfRes[$ch]"
    ConfRes[$ch]=$(rbcpread x1054 2)
    ConfRes[$ch]=$((0x${ConfRes[$ch]}))

    if [ -n "${Att_G_b[$ch]}" ]; then
	ConfRes[$ch]=$((${ConfRes[$ch]} &  0xE1FF))
	ConfRes[$ch]="$((${ConfRes[$ch]} + ( ${Att_G_b[$ch]} << 9 ) ))"
	AttFlag="Done"
    fi
    if [ -n "${Ib_ClipVb[$ch]}" ]; then
	ConfRes[$ch]=$((${ConfRes[$ch]} &  0x9FC0))
	ConfRes[$ch]="$((${ConfRes[$ch]} + ( ${Clip_Sel_b[$ch]} << 13 )))"
	ConfRes[$ch]=$((${ConfRes[$ch]} + ${Ib_ClipVb[$ch]}))
	ClipFlag="Done"
    fi
    ConfRes[$ch]=$(echo "obase=16;${ConfRes[$ch]}" | bc)
fi

if [ -n "${ConfRes[$ch]}" ]; then
echo \
"#ch$ch config
wrb x1050 1$ch
wrs x1051 x${ConfRes[$ch]}
wrb x1040 xff" >>$tmpfile
rbcpload $tmpfile
flag_check x1040
fi

done

if [ "x$AttFlag" != "x" ]; then
    echo ATTE_GAIN=${Att_G_b_Gain[@]}
fi
if [ "x$ClipFlag" != "x" ]; then
    echo Clip_Sel_b=${Clip_Sel_b[@]}
    echo Ib_ClipVb=${Ib_ClipVb[@]}
fi

if [ "$TRIGGERINITFlag" -eq 1 ]; then
echo \
"
#L1###########
#default settings for register 102 (not necessary to modyfy)
#wrb x1056 102
#wrs x1057 x3400
#wrb x1045 xff
#adder switch1
wrb x1056 101
wrs x1057 x031B
wrb x1045 xff
" >>$tmpfile
rbcpload $tmpfile
flag_check x1045

echo \
"
#adder switch0
wrb x1056 100
wrs x1057 x3B76
wrb x1045 xff
" >>$tmpfile
rbcpload $tmpfile
flag_check x1045

fi

ch2regconv=(98 99 97)
ch2chrconv=(A B C)
for DACch in 0 1 2; do
if [ -n "${DAC_TH_H[$DACch]}" ] && [ -n "${DAC_TH_L[$DACch]}" ]; then
echo \
"#L1 thresh adder ${ch2chrconv[$DACch]}
wrb x1056 ${ch2regconv[$DACch]}
wrb x1057 ${DAC_TH_H[$DACch]}
wrb x1058 ${DAC_TH_L[$DACch]}
wrb x1045 xff" >>$tmpfile
rbcpload $tmpfile
flag_check x1045

echo DAC_TH${DACch}_H=${DAC_TH_H[$DACch]}, DAC_TH${DACch}_L=${DAC_TH_L[$DACch]}
elif [ -n "${DAC_TH_H[$DACch]}" ] || [ -n "${DAC_TH_L[$DACch]}" ]; then
    command_rbcp "wrb x1056 ${ch2regconv[$DACch]}"
    command_rbcp "wrb x1046 xff"
    command_rbcp "rd x1059 2" 
    # rbcpread tmpthre
    tmpthre=$(rbcpread x1059 2)

    if [ "x${DAC_TH_H[$DACch]}" = "x" ]; then  
	DAC_TH_H[$DACch]=$((0x$(echo $tmpthre | cut -c 1-2)))
	echo DAC_TH${DACch}_L=${DAC_TH_L[$DACch]}; fi
    if [ "x${DAC_TH_L[$DACch]}" = "x" ]; then  
	DAC_TH_L[$DACch]=$((0x$(echo $tmpthre | cut -c 3-4)))
	echo DAC_TH${DACch}_H=${DAC_TH_H[$DACch]}; fi

echo \
"#dac thresh adder ${ch2chrconv[$DACch]}
wrb x1056 ${ch2regconv[$DACch]}
wrb x1057 ${DAC_TH_H[$DACch]}
wrb x1058 ${DAC_TH_L[$DACch]}
wrb x1045 xff" >>$tmpfile
rbcpload $tmpfile
flag_check x10

fi
done


if [ "$TRIGGERINITFlag" -eq 1 ]; then
echo \
"
#L0 delay#####
#init delay
wrw x105c x40000000
wrb x1043 xff" >> $tmpfile
rbcpload $tmpfile
flag_check x1043

echo \
"
wrw x105c x40010000
wrb x1043 xff" >> $tmpfile
rbcpload $tmpfile
flag_check x1043

echo \
"
wrw x105c x400a0800
wrb x1043 xff" >> $tmpfile
rbcpload $tmpfile
flag_check x1043

echo \
"
#channel enable
wrw x105c x46127f00
wrb x1043 xff" >> $tmpfile
rbcpload $tmpfile
flag_check x1043

fi


for ch in `seq 0 6`; do
if [ -n "${DELAY[$ch]}"  ]; then
    comword=$((ch / 2))
    comword="$((comword << 9 ))"
    comword=$((comword + (0x13 - (ch % 2)) ))
    comword="$((comword << 8 ))"
    comword=$((comword + 0x${DELAY[$ch]}))
    comword="$((comword << 8 ))"
    comword=$((comword + 0x40000000 ))
    comword=$(echo "obase=16;$comword" | bc)

echo \
"#ch$ch delay
wrw x105c x$comword
wrb x1043 xff" >>$tmpfile
rbcpload $tmpfile
flag_check x1043

fi
done





########################
### Test Pulse 
########################


if [ -n "$ENABLED_Ch" ]; then
#echo "obase=16;$ENABLED_Ch" | bc
#ENABLED_tmp=0
#ENABLED_Ch=$(( (ENABLED_tmp & ~ENABLED_Ch) + ENABLED_Ch))

echo \
"#tp trig enable
wrb x10a1 xff
#tp freq
wrs x10a3 3333
#tp mode
#wrs x10a5 x2880
wrs x10a5 x2800
#wrs x10a5 x2804
#wrs x10a5 x2808
#wrs x10a5 x280c
#wrs x10a5 x2801
#wrs x10a5 x2802
wrb x10b6 1
wrb x10a0 xff" >> $tmpfile
rbcpload $tmpfile
flag_check x10a0

echo \
"#tp pow enable channel
wrb x10a5 x29
wrb x10a6 x$ENABLED_Ch
wrb x10a0 xff" >>$tmpfile
rbcpload $tmpfile
flag_check x10a0

fi


for ch in {0..6}; do
if [ -n "${TP_GAIN[$ch]}" ]; then
echo \
"#gain ch$ch
wrb x10a5 x2$ch
wrb x10a6 ${TP_GAIN[$ch]}
wrb x10b6 1
wrb x10a0 xff" >>$tmpfile
rbcpload $tmpfile
flag_check x10a0

fi
done


if [ -n "$DISABLED_Ch" ]; then
#DISABLED_tmp=1
#DISABLED_Ch=$(( DISABLED_tmp & ~DISABLED_Ch ))

echo \
"#tp pow disable channel
wrb x10a5 x29
wrb x10a6 $DISABLED_Ch
wrb x10a0 xff" >> $tmpfile

if [ "$DISABLED_Ch" -eq 0 ]; then
echo \
"#tp trig disable
wrb x10a1 xff" >>$tmpfile
fi

fi





########################
### PMT HV setting
########################

if [ -n "$PMTen_Ch" ]; then

echo \
"wrb x10b6 1
wrb x10a5 x10
wrb x10a6 x$PMTen_Ch
wrb x10a0 xff" >$tmpfile
rbcpload $tmpfile
flag_check x10a0

fi

if [ "$HVnAFlag" -eq 1 ]; then 
    command_rbcp "wrb x10a5 xc4" # RAM_0x010 read
    command_rbcp "wrb x10b6 16"
    command_rbcp "wrb x10a0 xff"
    flag_check x10a0
    # command_rbcp "rd x10b9 14" 
    # rbcpread RAM_0x010
    RAM_0x010=$(rbcpread x10b9 14)
fi

if [ "$HVFlag" -eq 1 ]; then
echo HV=${HVcont[@]} [V]

echo \
"#hv value
wrb x10a5 x44
wrs x10a6 0" >>$tmpfile

for ch in {0..6}; do
ch_base16=$(echo "obase=16;$((0x10a8+ch*2))" | bc)

if [ -z "${HV[$ch]}" ];then
    HV[$ch]=$(echo $RAM_0x010 | cut -c $((1+4*ch))-$((4+4*ch)) )
    HV[$ch]=$((0x${HV[$ch]}))
fi

echo \
"wrs x${ch_base16} ${HV[$ch]}" >>$tmpfile

done

echo \
"wrb x10b6 16
wrb x10a0 xff" >>$tmpfile
rbcpload $tmpfile
flag_check x10a0

echo \
"#hv set
wrs x10a5 x3011
wrb x10b6 1
wrb x10a0 xff" >>$tmpfile
rbcpload $tmpfile
flag_check x10a0

fi


#command_rbcp "load $tmpfile"




#########################
### HV & current monitor
#########################


conversion(){
    val10=$((0x$1))
    echo "scale=2; ${val10} * 2.048/32767 * $2" | bc
}

rdscbram(){
echo \
"wrb x10b6 16
wrb x10a5 $(( (2#11<<6) + (($i+2)<<2) ))
wrb x10a0 xff" >$tmpfile
rbcpload $tmpfile
flag_check x10a0
#command_rbcp "load $tmpfile"

echo \
"rd x10b7 16" >$tmpfile
rbcpload $tmpfile

#rbcpread "RAM_0x020[$i]"
RAM_0x020[$i]=$(rbcpread x10b7 16)
#echo RAM[$i]=${RAM_0x020[$i]}

}

if [ "$MonitorFlag" -eq 1 ];then
echo \
"wrb x10b6 1
wrs x10a5 x3020
wrb x10a0 xff" >$tmpfile
echo "converting ADC values..."
rbcpload $tmpfile
flag_check x10a0



sleep 2

echo ""
echo "reading SCB RAM..."
for i in {0..2}
do
  rdscbram
done


echo \
"wrb x10b6 1
wrs x10a5 x3053
wrb x10a0 xff" >$tmpfile
echo ""
echo "converting Amp Temp...."
rbcpload $tmpfile
flag_check x10a0


sleep 2

echo ""
echo "reading SCB RAM..."
i=3 rdscbram

echo \
"wrb x10b6 1
wrs x10a5 x306f
wrb x10a0 xff" >>$tmpfile
echo ""
echo "converting SCB Temp...."
rbcpload $tmpfile
flag_check x10a0


sleep 2

echo ""
echo "reading SCB RAM..."
i=4 rdscbram

usleep 700000

for i in {5..9}
do
  rdscbram
done

if [ $DEBUG_FLAG -ne 0 ]; then
    exit
fi

TPpV=$(echo ${RAM_0x020[0]} | cut -c 1-4 )
TPpV=$(conversion $TPpV 3 )
TPnV=$(echo ${RAM_0x020[1]} | cut -c 1-4 )
TPnV=$(conversion $TPnV 3 )

for ch in {0..6}
do
  HVmoni[$ch]=$(echo ${RAM_0x020[0]} | cut -c $((5+4*ch))-$((8+4*ch)) )
  HVmoni[$ch]=$(conversion ${HVmoni[$ch]} 1001 )

  ANcur[$ch]=$(echo ${RAM_0x020[1]} | cut -c $((5+4*ch))-$((8+4*ch)) )
  ANcur[$ch]=$(conversion ${ANcur[$ch]} "0.1" )

  AmpTemp[$ch]=$(echo ${RAM_0x020[3]} | cut -c $((5+4*ch))-$((8+4*ch)) )
  AmpTemp[$ch]=$(echo "scale=2; $((0x${AmpTemp[$ch]}))/20" | bc)
done

P1V2=$(echo ${RAM_0x020[2]} | cut -c 1-4 )
P1V2=$(conversion $P1V2 1 )

PMTcur=$(echo ${RAM_0x020[2]} | cut -c 5-8 )
PMTcur=$(conversion $PMTcur "0.5" )

P6PMT=$(echo ${RAM_0x020[2]} | cut -c 9-12 )
P6PMT=$(conversion $P6PMT 4 )

P6V=$(echo ${RAM_0x020[2]} | cut -c 13-16 )
P6V=$(conversion $P6V 4 )

P3V3=$(echo ${RAM_0x020[2]} | cut -c 21-24 )
P3V3=$(conversion $P3V3 3 )
N3V3=$(echo ${RAM_0x020[2]} | cut -c 25-28 )
N3V3=$(conversion $N3V3 3 )

Ref1V5=$(echo ${RAM_0x020[2]} | cut -c 29-32 )
Ref1V5=$(conversion $Ref1V5 1 )

SCB_Temp=$(echo ${RAM_0x020[4]} | cut -c 5-8 )
SCB_Temp=$(echo "scale=2; $((0x$SCB_Temp)) * 175.72/65535 -46.85 " | bc)

SCB_Humm=$(echo ${RAM_0x020[4]} | cut -c 5-8 )
SCB_Humm=$(echo "scale=2; $((0x$SCB_Humm)) * 125/65535 -6 " | bc)

for ch in {0..6}
do
  FactCode[$ch]=$(echo ${RAM_0x020[5]} | cut -c $((3+2*ch))-$((4+2*ch)) )

  case ${FactCode[$ch]} in
      00) FactCode[$ch]="nonePMT" ;;
      30) FactCode[$ch]="undefined" ;;
      31) FactCode[$ch]="Factor1" ;;
      32) FactCode[$ch]="Factor2" ;;
      33) FactCode[$ch]="Factor3.5" ;;
  esac
done

echo ""
echo HVmoni=${HVmoni[@]} [V]
echo ANcur=${ANcur[@]} [mA]
echo "+6V=$P6V [V], +3.3V=$P3V3 [V], Ref1.5V=$Ref1V5 [V]"
echo Amp_Temp=${AmpTemp[@]} [*C]
echo SCB_TEMP=$SCB_Temp [*C], HUMI=$SCB_Humm [%]
echo FactCode=${FactCode[@]}

fi


echo ""
if [ $DEBUG_FLAG -eq 0 ] && [ -f rcvdBuf.txt ]; then rm rcvdBuf.txt; fi

rm $tmpfile
