benchmarks=(bwaves  GemsFDTD  lbm  leslie3d  libquantum  mcf  milc  omnetpp  soplex)

numbers=(1 2 3 4 5 6 7 8 9)
k=0
for i in "${benchmarks[@]}"
do
    rm ./results/results/$i/${i}_L3WriteAtOffset.csv
    for k in "${numbers[@]}"
    do
        echo $k
        echo $i
        #./tools/dumpstats.py -d ./results/${benchmarks[$i]}/${numbers[$k]} | grep L3.numberOfL3WriteAtOffset> ./results/${benchmarks[$i]}/${numbers[$k]}/accessLoads${numbers[$k]}.csv
        #k=$((k+1)) #double parenthese are required
        echo "./tools/dumpstats.py -d ./results/results/$i/$k"
        ./tools/dumpstats.py -d ./results/results/$i/$k | grep L3.numberOfL3WriteAtOffset >> ./results/results/$i/${i}_L3WriteAtOffset.csv
        cp ./results/results/$i/${i}_L3WriteAtOffset.csv ./results/results/${i}_L3WriteAtOffset.csv
        k=$((k+1)) #double parenthese are required
        echo "-----------------Pinball:${numbers[$k]} Done!!!----------------------"
    done
done
