CC ?= gcc
all: msr dpm

msr: msr.c r0e.h
	$(CC) msr.c -o msr -g
dpm: dpm.c r0e.h
	$(CC) dpm.c -o dpm -g

clean:
	rm -f dpm msr
	
