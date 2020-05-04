#! /bin/bash
ITEREATIONS=(10000 100000 1000000 10000000 100000000 1000000000)
#ITEREATIONS=(10000)
NRUNS=30

#echo "Array_Size,Pattern_Tested,Time"

for IT in "${ITEREATIONS[@]}"; do
        for R in `seq 1 $NRUNS`; do
            ./main $IT
        done
done | tee output.csv 