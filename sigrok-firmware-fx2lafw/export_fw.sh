#!/bin/bash

outfile=../host/vcapt_hex.c

echo "#include <stdio.h> /* For NULL... */" > $outfile
echo >> $outfile
echo "const char *vcapt_firmware[]={" >> $outfile
bash -c 'while read line ; do echo -e "\t\"$line\"," >>'$outfile'; done < hw/cypress-fx2/fx2lafw-cypress-fx2.ihx'
echo -e "\tNULL" >> $outfile
echo "};" >> $outfile
echo "Created "$outfile
