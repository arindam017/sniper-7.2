#!/usr/bin/python

import re
import sys
import os
import time
import datetime

#This parses param_new.out files from given directory.
#And makes a .csv file containing BENCHMARKS, IPC, L3_MPKI for that policy.

      
def main():
  print '------------------------------------------------------------------------'
  print 'USAGE: Make SURE you are in the directory where the script is'
  print 'usage for 1 benchmark: ./csv_generator.py ./path/where/*.out/files/are '
  print '------------------------------------------------------------------------'
  args = sys.argv[1:]
  print args
  accum=[]
  if not args:
    print 'usage: [ input dir ] '
    sys.exit(1)
  policy = args[0].split("/")[-1]
  if policy == "":
    policy = args[0].split("/")[-2]	
  policy = policy + ".csv"
  #print policy
  op_file = os.path.join(args[0],policy)
  fop=open(op_file,'w')
  fop.write("Benchmark, CPI, IPC, L3_MPKI\n")
  #fop.write("Benchmark, IPC, L3_Miss, WRTI, WRTE\n")
  ResultsNotGenerated = []
  for dirpath, dirnames, filenames in os.walk(args[0]):
    for filename in [f for f in filenames if f=="params_new.out"]:
	file_path = os.path.join(dirpath, filename)
    	
	if (file_path.split("/")[-3] != ""):
		print file_path
        	out_file = open(file_path)
		lines = out_file.readlines()
        	try:
			#print lines[8]
			if (lines[8] == "\n" or lines[8] == ""):
				print "One or more traces not generated"
				ResultsNotGenerated.append(file_path.split("/")[-3])
			fop.write(lines[8])
		except IndexError:
			#print "Index Error occured"
			#print "results not generated for: "+ file_path.split("/")[-3]+"\n"
			ResultsNotGenerated.append(file_path.split("/")[-3])
		out_file.close()
  fop.close()
	
  print "Results Not Generated for: "+ str(ResultsNotGenerated)	  	
           
if __name__ == "__main__":
  main()
