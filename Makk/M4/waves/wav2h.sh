#!/bin/sh
# start this batch
# before you try to compile the m4 machine
# it converts the .wav to .h files (convert + raw2h)
# and generates waves.cpp and wavename.inc

wine Convert.exe "*.wav" -raw
wine Raw2h.exe
wine Dir2c.exe
rm *.RAW
sed -i -e "s/static char \*/static const char \*/" wavename.inc

