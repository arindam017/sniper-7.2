benchmarks=(bwaves)

numbers=(1 2 3 4 5 6 7 8 9)
k=0
for i in "${benchmarks[@]}"
do
    rm ./$i/${i}_L3WriteAtOffset.csv
    rm ./$i/${i}_L3WriteLengths.csv
    for k in "${numbers[@]}"
    do
        echo $k
        echo $i
        #./tools/dumpstats.py -d ./results/${benchmarks[$i]}/${numbers[$k]} | grep L3.numberOfL3WriteAtOffset> ./results/${benchmarks[$i]}/${numbers[$k]}/accessLoads${numbers[$k]}.csv
        #k=$((k+1)) #double parenthese are required
        echo "../../tools/dumpstats.py -d ./$i/$k"
        # >> indicates that you want to append
        ../../tools/dumpstats.py -d ./$i/$k | grep L3.numberOfL3WriteAtOffset >> ./$i/${i}_L3WriteAtOffset.csv
        ../../tools/dumpstats.py -d ./$i/$k | grep L3.lengthOfL3Writes >> ./$i/${i}_L3WriteLengths.csv
        cp ./$i/${i}_L3WriteAtOffset.csv ./${i}_L3WriteAtOffset.csv
        cp ./$i/${i}_L3WriteLengths.csv ./${i}_L3WriteLengths.csv
        k=$((k+1)) #double parenthese are required
        echo "-----------------Pinball:${numbers[$k]} Done!!!----------------------"
    done
done
