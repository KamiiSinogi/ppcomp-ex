#!/bin/bash
#YBATCH -r epyc-7502_8
#SBATCH -N 1
#SBATCH --cpus-per-task=8
#SBATCH --time=00:5:00

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PROC_BIND=spread
export OMP_PLACES=cores
./bsort 10000039