#!/usr/bin/python

import re
import sys
import os
import time
import datetime

#This parses parameters from .out files from given directory.
#First argument is the directory from where to take .out files

#Extract the weight assigned to a trace from its respective weight file
def xtract_weight(tracenum,weightdir,application):
  f=open(os.path.join(weightdir,application+'_t.weights'),'rU')
  str=f.read()
  str=str.split()
  ind=str.index(tracenum)
  weight=str[ind-1]
  #print application, tracenum, weight
  return float(weight)


#Function to extract parameters from the sim.out file  
def xtract_para(ipdir,filename, num_apps):
  paras=[]
  paras.append(filename.split('.')[0]) #0
  print filename
  f=open(os.path.join(ipdir,filename),'rU')
  

  str=f.read()
  if (num_apps == '2'):
    tmp=re.findall('Cache L3[\s]+',str) # extract parameters of the L3
    tmp=tmp[0]
    access=re.findall('num cache accesses[\s]+[|]+[\s\d]+[|]+[\s\d]+',tmp) #extract num accesses
    miss=re.findall('num cache misses[\s]+[|]+[\s\d]+[|]+[\s\d]+',tmp)	#extract num miss
    miss_r=re.findall('miss rate[\s]+[|]+[\s\d".%"]+[|]+[\s\d".%"]+',tmp)	#extract num miss
    mpki=re.findall('mpki[\s]+[|]+[\s\d"."]+[|]+[\s\d"."]+',tmp) #extract mpki
  else:
    tmp_ins=re.findall('Instructions[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    ins=re.findall('[\d]+',tmp_ins[0])
    paras.append(float(ins[0])) #1  #Extracted Number of Instructions
   
    tmp_cycl=re.findall('Cycles[\s]+[|]+[\s]+[\d]+[\s|\d]+',str) 
    cycl=re.findall('[\d]+',tmp_cycl[0])
    paras.append(float(cycl[0]))  #2 #Extracted Number of cycles 

    tmp_cpi = float(cycl[0])/float(ins[0]) #3 Extracted cpi
    tmp_cpi = format(tmp_cpi, '.2f')
    paras.append(float(tmp_cpi))
  
    tmp_ipc = float(ins[0])/float(cycl[0]) #4 Extracted IPC
    tmp_ipc = format(tmp_ipc, '.2f')
    paras.append(float(tmp_ipc))

    #arindam
    #tmp_wrti=re.findall('Write to Read Transition Count at an Interval[\s]+[|]+[\s]+[\d]+[\s|\d]+',str) 
    #wrti=re.findall('[\d]+',tmp_wrti[0])
    #paras.append(float(wrti[0]))  #5 #Extracted Write to Read Transition Count at an Interval

    #tmp_wrte=re.findall('Write to Read Transition Count at an Eviction[\s]+[|]+[\s]+[\d]+[\s|\d]+',str) 
    #wrte=re.findall('[\d]+',tmp_wrte[0])
    #paras.append(float(wrte[0]))  #6 #Extracted Write to Read Transition Count at an Eviction
    ########



    tmp=re.findall('Cache L3[\s]+[|]+[\s]+[\n]+[\s]+[\w\s\d|\.\%]+',str) 
    tmp=tmp[0].split('\n')
    tmp = tmp[1:-3]
    L3Param = []
    L3ParamValues = [] 
    for i in range(len(tmp)):
	tmp[i] = tmp[i].strip()
	L3Param.append(tmp[i].split('|')[0].strip())	   
        L3ParamValues.append(tmp[i].split('|')[1].strip())
    for i in range(len(L3ParamValues)):
	if '%' in L3ParamValues[i]:
		L3ParamValues[i] = L3ParamValues[i][:-1]
	L3ParamValues[i] = float(L3ParamValues[i])
	paras.append(L3ParamValues[i])	
    print paras
  #paras contains [benchmark_trace, num_instrutions, num_cycles, cpi, ipc, l3_num_cache_accesses, l3_num_cache_misses, l3_miss_rate, l3_mpki]: Abhijit's version
  #                 0               1                 2           3   4     5                     6                     7             8           
  #paras contains [benchmark_trace, num_instrutions, num_cycles, cpi, ipc, wrti, wrte, l3_num_cache_accesses, l3_num_cache_misses, l3_miss_rate, l3_mpki]: Arindam's version
  #                 0               1                 2           3   4     5     6     7                     8                     9             10
  return paras                       
      
def main():
  print '------------------------------------------------------------------------'
  print 'USAGE: Make SURE you are in the directory where the script is'
  print 'usage for 1 benchmark: ./parser_all.py ./path/where/*.out/files/are 1'
  print 'usage for 2 benchmark: ./parser_all.py ./path/where/*.out/files/are 2'
  print '------------------------------------------------------------------------'
  args = sys.argv[1:]
  print args
  accum=[]
  if not args:
    print 'usage: [ input dir ] '
    sys.exit(1)

  ip_dir=args[0];
  num_apps=args[1];

  #weight_dir='/home/newton/research/tools/sniper/sniper-6.0/traces/250M'
  weight_dir='/home/arindam/Desktop/Sniper/sniper-7.2/benchmark_weights'
  op_dir=os.path.join(ip_dir,'para_extracted')
  cmd="mkdir "+op_dir
  os.system(cmd)
  op_file = os.path.join(op_dir,'params_new.out')
  fop=open(op_file,'w')
  
  cumm_wgt = 0
  wgt_cpi = 0
  cumm_wgt_cpi = 0
  wgt_mpki = 0
  cumm_wgt_mpki = 0
  cumm_wgt_miss = 0
  cumm_wgt_wrti = 0
  cumm_wgt_wrte = 0

  print "results directory: "+ip_dir
  
  if num_apps == '2':
    fop.write("app1\tmpki\tmisr\tcpi\t\tdeadB\t\tinsB\t\tapp2\tmpki\tmisr\tcpi\t\tdeadB\t\tinsB\tinvB\n")
  else:  
    fop.write("app1, CPI, IPC, l3_access, l3_miss, l3_miss_r, l3_mpki, wgt\n")
    #fop.write("app1, CPI, IPC, l3_access, l3_miss, l3_miss_r, l3_mpki, wgt, wrti, wrte\n")
  
  for files in os.listdir(ip_dir):    # iterates over all files
    if os.path.isfile(os.path.join(ip_dir, files)) and files.endswith(".out"):  
      print files
      trace_num = files.split('.')[0].split('_')[1]
      application = files.split('.')[0].split('_')[0]
      print "Weight for "+files 
      wgt = xtract_weight(trace_num, weight_dir, application)
      print wgt 
      paras = xtract_para(ip_dir, files, num_apps)# [benchmark_trace,num_instrutions,num_cycles,cpi,ipc,l3_accesses,l3_misses,l3_miss_r,l3_mpki]
      paras.append(wgt) #paras[9] = weight, paras[11] = weight according to arindam
      print paras
      benchmark_name = paras[0]
      print benchmark_name

      num_instr = paras[1]
      print num_instr
      num_cycl = paras[2]
      print num_cycl

      cpi = paras[3]
      print cpi
      ipc = paras[4]
      print ipc

      access = paras[5]
      print access
      miss = paras[6]
      print miss
      miss_r = paras[7]
      print miss_r
      mpki = paras[8]
      print mpki

      #arindam##
      #access = paras[7]
      #print access
      #miss = paras[8]
      #print miss
      #miss_r = paras[9]
      #print miss_r
      #mpki = paras[10]
      #print mpki
      
      #wrti = paras[5]
      #print wrti
      #wrte = paras[6]
      #print wrte

      
      print "================================================"
      cumm_wgt += wgt
      print "Cummulative Weights = "+str(cumm_wgt)

      wgt_cpi = cpi*wgt
      print "Weighted CPI = "+str(wgt_cpi)
      cumm_wgt_cpi += wgt_cpi
      print "Cummulative Weighted CPI = "+str(cumm_wgt_cpi)

      wgt_mpki = mpki*wgt
      print "Weighted MPKI = "+str(wgt_mpki)
      cumm_wgt_mpki += wgt_mpki
      print "Cummulative Weighted MPKI = "+str(cumm_wgt_mpki)

      #Arindam
      #wgt_miss = miss*wgt
      #print "Weighted l3 miss = "+str(wgt_miss)
      #cumm_wgt_miss += wgt_miss
      #print "Cummulative Weighted l3 miss = "+str(cumm_wgt_miss)

      #wgt_wrti = wrti*wgt
      #print "Weighted wrti = "+str(wgt_wrti)
      #cumm_wgt_wrti += wgt_wrti
      #print "Cummulative Weighted wrti = "+str(cumm_wgt_wrti)

      #wgt_wrte = wrte*wgt
      #print "Weighted wrte = "+str(wgt_wrte)
      #cumm_wgt_wrte += wgt_wrte
      #print "Cummulative Weighted wrte = "+str(cumm_wgt_wrte)


      print "================================================"      


      print "CPI: "+format((float(paras[2])/float(paras[1])),'.2f')
      print "IPC: "+format((float(paras[1])/float(paras[2])),'.2f')
      fop.write(files+","+str(cpi)+","+str(ipc)+","+str(access)+","+str(miss)+","+str(miss_r)+","+str(mpki)+","+str(wgt)+"\n")
      #fop.write(files+","+str(cpi)+","+str(ipc)+","+str(access)+","+str(miss)+","+str(miss_r)+","+str(mpki)+","+str(wgt)+","+str(wrti)+","+str(wrte)+"\n")
     
      
      print "================================"
  Weighted_cpi = cumm_wgt_cpi/cumm_wgt
  Weighted_cpi = format(Weighted_cpi, '.2f')
  Weighted_ipc = cumm_wgt/cumm_wgt_cpi
  Weighted_ipc = format(Weighted_ipc, '.2f')
  Weighted_mpki = cumm_wgt_mpki/cumm_wgt
  Weighted_mpki = format(Weighted_mpki, '.2f')

  #Arindam
  #Weighted_l3miss = cumm_wgt_miss/cumm_wgt
  #Weighted_l3miss = format(Weighted_l3miss, '.0f')
  #Weighted_wrti = cumm_wgt_wrti/cumm_wgt
  #Weighted_wrti = format(Weighted_wrti, '.0f')
  #Weighted_wrte = cumm_wgt_wrte/cumm_wgt
  #Weighted_wrte = format(Weighted_wrte, '.0f')



  fop.write("\n\n")
  #normalizing mpki by dividing it with weight
  fop.write("Benchmark, Weighted CPI, Weighted IPC, Weighted MPKI\n")
  fop.write(application+","+str(Weighted_cpi)+","+str(Weighted_ipc)+","+Weighted_mpki+"\n")
  #fop.write(application+","+str(Weighted_ipc)+","+str(Weighted_l3miss)+","+str(Weighted_wrti)+","+str(Weighted_wrte)+"\n")

  ts = time.time()
  st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
  fop.write("\n\n\n\n"+st)
           
if __name__ == "__main__":
  main()
