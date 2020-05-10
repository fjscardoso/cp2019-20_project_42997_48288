#! /bin/bash
ITEREATIONS=(10000 100000 1000000 10000000 100000000)
#THREADS=(1 2 4 8 16 32 64)
THREADS=(4)
NRUNS=30


for TH in "${THREADS[@]}"; do
    for IT in "${ITEREATIONS[@]}"; do
        for R in `seq 1 $NRUNS`; do
            ./main $TH $IT
        done
    done
done | tee output.csv


# echo "starting script2"  
#     for TH in "${THREADS[@]}"; do
#             for R in `seq 1 $NRUNS`; do
#                 ./main "$TH" "$IT"
#             done
#     done
# done | tee output.csv 
