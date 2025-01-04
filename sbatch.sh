#!/bin/sh
#SBATCH -c 64
#SBATCH -n 1
##SBATCH -N 1
#SBATCH --mem-per-cpu=2G
#SBATCH -t 10:00:00

echo "N,threads,seq_t,par_t" > $2

for size in 1000 2000 4000 6000 8000 10000; do
    for t in 2 4 8 16 32 48 64; do
        export OMP_NUM_THREADS=$t
        for _ in {1..5}; do
            srun $1 $size $2
        done
    done
done
