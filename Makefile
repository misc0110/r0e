all: msr dpm

msr: msr.c
	gcc msr.c -o msr -g
dpm: dpm.c
	gcc dpm.c -o dpm -g
