all: msr dpm

msr: msr.c r0e.h
	gcc msr.c -o msr -g
dpm: dpm.c r0e.h
	gcc dpm.c -o dpm -g

clean:
	rm -f dpm msr
	
