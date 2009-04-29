rem     start this batch
rem     before you try to compile the m4 machine
rem     it converts the .wav to .h files (convert + raw2h)
rem     and generates waves.cpp and wavename.inc

convert *.wav -raw
raw2h
dir2c
del *.raw