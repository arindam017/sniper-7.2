export SNIPER_ROOT=/home/abhijit/new_sniper_61_damlp/sniper_61_damlp

path1="/home/abhijit/sniper_original/sniper-6.1/results/DAAIP/"
path2="/home/abhijit/sniper_original/sniper-6.1/results/LRU/"
path3="/home/abhijit/sniper_original/sniper-6.1/results/SRRIP/"
path4="/home/abhijit/sniper_original/sniper-6.1/results/REUSE_DEC_AFTER_8/"
path5="/home/abhijit/sniper_original/sniper-6.1/results/REUSE_DEC_ON_EVICT/"

parser="./parser_all_qsize.py"
#All BENCHMARKS = 24
#benchmarks=(astar libq lbm bwaves bzip cactusADM gcc gobmk GemsFDTD gromacs h264ref hmmer leslie milc namd omnetpp sjeng soplex sphinx tonto wrf zeusmp mcf perlbench)

#REUSE SELECTED BENCHMARKS = 15
benchmarks=(astar bwaves bzip cactusADM gcc GemsFDTD gobmk gromacs h264ref hmmer lbm leslie libq mcf milc namd omnetpp perlbench sjeng soplex sphinx tonto trace wrf zeusmp)
#benchmarks=(cactusADM gcc GemsFDTD lbm leslie libq mcf milc omnetpp soplex zeusmp)  #Done in MTP2


#Thrashing benchmarks = 8 milc to be added.
#benchmarks=(cactusADM lbm leslie libq mcf sphinx milc bwaves)

#REMAINING REUSE BENCHMARKS = 8
#benchmarks=(gobmk gromacs h264ref namd perlbench sjeng tonto wrf zeusmp)


#FOR RUNNING SINGLE BENCHMARK 
#benchmarks=(bzip)

#suffix=(1 10 20 100)
#benchmarks=(cactusADM mcf sphinx milc astar leslie lbm bwaves libq)
#benchmarks=(cactusADM sphinx)
for i in {0..24}
    do
	echo "===================================================================================================================================="
	echo ""
	echo ""
	echo ""
	echo ""
	/home/arindam/Desktop/Sniper/sniper-7.2_para23_6_t192/parser_files/phc/ultimate_parser_single_core.py /home/arindam/Desktop/Sniper/sniper-7.2_para23_6_t192/expt_results/phc_para23_6_t192/${benchmarks[i]} 1
	echo ""
	echo ""
	echo ""
	echo ""
	echo "===================================================================================================================================="
       
    done
