#!/bin/bash

# $1 $2 $3 $4 $5 $6 $7 $8 (-s s -E e -b b -t file)
./csim   $1 $2 $3 $4 $5 $6 $7 $8 > output1
./csim-ref   $1 $2 $3 $4 $5 $6 $7 $8 > output2

md5sum output1
md5sum output2

rm -f output1 output2
