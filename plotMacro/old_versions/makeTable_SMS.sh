#!/bin/bash

jet=$1

dir=../output/multiChannel

outfile=SMSScan_${jet}.table

[ -e $outfile ] && rm $outfile
touch $outfile

prefix=SMSScan_mS0_mG
#SMSScan_mS0_mG1000_mN1000_1jet.dat.result.txt
fileName=$dir/
if [ $jet == 'jet' ]; then
    fileName=$dir/${prefix}*_1jet.dat.result.txt
fi
if [ $jet == 'nojet' ]; then
    fileName=$dir/${prefix}*_nojet.dat.result.txt
fi

for file in `dir -d $fileName`
do
    mS=`grep "squark = " $file | cut -d = -f 2`

    check=x$mS
    [ "$check" == "x" ] && continue

    mG=`grep "gluino = " $file | cut -d = -f 2`
    mN=`grep "chi1 = " $file | cut -d = -f 2`
    acc=`grep "signal.acceptance = " $file | cut -d = -f 2`
    xsec=`grep "Xsection.NLO = " $file | cut -d = -f 2`
    xsecPDFError=`grep "signal.scale.uncertainty = " $file | cut -d = -f 2`
    xsecRSErrorNeg=`grep "signal.scale.uncertainty.UP = " $file | cut -d = -f 2`
    xsecRSErrorPos=`grep "signal.scale.uncertainty.DN = " $file | cut -d = -f 2`

    obsLimit=`grep "CLs observed =" $file | cut -d = -f 2`
    expLimit=`grep "CLs expected =" $file | cut -d = -f 2`
    exp_m1s=`grep "CLs expected m1sigma =" $file | cut -d = -f 2`
    exp_m2s=`grep "CLs expected m2sigma =" $file | cut -d = -f 2`
    exp_p1s=`grep "CLs expected p1sigma =" $file | cut -d = -f 2`
    exp_p2s=`grep "CLs expected p2sigma =" $file | cut -d = -f 2`

    check=x$acc
    [ "$check" == "x 0" ] && continue

    check=x$exp_p2s
    #[ "$check" == "x " ] && continue

#if CLs failed for whatever reason, just grab the less accurate versions?
check=x$obsLimit
[ "$check" == "x " ] && obsLimit=`grep "CLs observed asymptotic =" $file | cut -d = -f 2`
check=x$expLimit
[ "$check" == "x " ] && expLimit=`grep "CLs expected profile likelihood =" $file | cut -d = -f 2`
check=x$exp_m1s
[ "$check" == "x " ] && exp_m1s=`grep "CLs expected m1sigma profile likelihood =" $file | cut -d = -f 2`
check=x$exp_m2s
[ "$check" == "x " ] && exp_m2s=`grep "CLs expected m2sigma profile likelihood =" $file | cut -d = -f 2`
check=x$exp_p1s
[ "$check" == "x " ] && exp_p1s=`grep "CLs expected p1sigma profile likelihood =" $file | cut -d = -f 2`
check=x$exp_p2s
[ "$check" == "x " ] && exp_p2s=`grep "CLs expected p2sigma profile likelihood =" $file | cut -d = -f 2`

    #echo "mS mG mN acc xsec xsecPDFError xsecRSErrorNeg xsecRSErrorPos obsLimit expLimit exp_m1s exp_m2s exp_p1s exp_p2s"

    echo -e "$mS\t$mG\t$mN\t$acc\t$xsec\t$xsecPDFError\t$xsecRSErrorNeg\t$xsecRSErrorPos\t$obsLimit\t$expLimit\t$exp_m1s\t$exp_m2s\t$exp_p1s\t$exp_p2s" >> $outfile

done

sed -i 's/nan/10000/' $outfile
