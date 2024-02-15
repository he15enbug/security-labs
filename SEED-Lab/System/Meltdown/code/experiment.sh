#!/bin/bash

gcc -march=native -o MeltdownExperiment MeltdownExperiment.c
for i in {1..1000}
do
	echo $i
	./MeltdownExperiment
done