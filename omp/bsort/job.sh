#!/bin/sh
#$ -cwd
#$ -l cpu_4=1
#$ -l h_rt=00:10:00

export OMP_PROC_BIND=close
export OMP_PLACES=cores
./bsort 10000039
