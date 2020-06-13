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
    miss=re.findall('num cache misses[\s]+[|]+[\s\d]+[|]+[\s\d]+',tmp)  #extract num miss
    miss_r=re.findall('miss rate[\s]+[|]+[\s\d".%"]+[|]+[\s\d".%"]+',tmp)   #extract num miss
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
    
    tmp_ribc=re.findall('Read Intense Block Counter[\s]+[|]+[\s]+[\d]+[\s|\d]+',str) 
    ribc=re.findall('[\d]+',tmp_ribc[0])
    paras.append(float(ribc[0]))  #5 Extracted Read Intense Block Counter

    tmp_wibc=re.findall('Write Intense Block Counter[\s]+[|]+[\s]+[\d]+[\s|\d]+',str) 
    wibc=re.findall('[\d]+',tmp_wibc[0])
    paras.append(float(wibc[0]))  #6 Extracted Write Intense Block Counter

    tmp_dbc=re.findall('Deadblock Counter[\s]+[|]+[\s]+[\d]+[\s|\d]+',str) 
    dbc=re.findall('[\d]+',tmp_dbc[0])
    paras.append(float(dbc[0]))  #7 Extracted Deadblock Counter

    tmp_wrte=re.findall('Write to Read Transition Count at an Eviction[\s]+[|]+[\s]+[\d]+[\s|\d]+',str) 
    wrte=re.findall('[\d]+',tmp_wrte[0])
    paras.append(float(wrte[0]))  #8 #Extracted Write to Read Transition Count at an Eviction

    tmp_rwte=re.findall('Read to Write Transition Count at an Eviction[\s]+[|]+[\s]+[\d]+[\s|\d]+',str) 
    rwte=re.findall('[\d]+',tmp_rwte[0])
    paras.append(float(rwte[0]))  #9 #Extracted Read to Write Transition Count at an Eviction



    ###########################################################################
    ###########################################################################

    tmp_Read_Intense_Blocks_M=re.findall('Read Intense Blocks M[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)

    Read_Intense_Blocks_M=re.findall('[\d]+',tmp_Read_Intense_Blocks_M[0])

    paras.append(float(Read_Intense_Blocks_M[0]))  #10



    tmp_Write_Intense_Blocks_M=re.findall('Write Intense Blocks M[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                   
    
    Write_Intense_Blocks_M=re.findall('[\d]+',tmp_Write_Intense_Blocks_M[0])                   
    
    paras.append(float(Write_Intense_Blocks_M[0]))  #11
    
    
    
    tmp_Read_Intense_Blocks_MDuringMigrate  =re.findall('Read Intense Blocks MDuringMigrate[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)        
    
    Read_Intense_Blocks_MDuringMigrate  =re.findall('[\d]+',tmp_Read_Intense_Blocks_MDuringMigrate[0])        
    
    paras.append(float(Read_Intense_Blocks_MDuringMigrate[0]))  #12
    
    
    
    tmp_Write_Intense_Blocks_MDuringMigrate =re.findall('Write Intense Blocks MDuringMigrate[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)       
    
    Write_Intense_Blocks_MDuringMigrate =re.findall('[\d]+',tmp_Write_Intense_Blocks_MDuringMigrate[0])       
    
    paras.append(float(Write_Intense_Blocks_MDuringMigrate[0])) #13
    
    

    
    tmp_M_Count                              =re.findall('M Count[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                                
    M_Count                              =re.findall('[\d]+',tmp_M_Count[0])                                
    paras.append(float(M_Count[0]))#14
    
    tmp_Migrate_During_Write_Count            =re.findall('Migrate During Write Count[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)              
    Migrate_During_Write_Count            =re.findall('[\d]+',tmp_Migrate_During_Write_Count[0])              
    paras.append(float(Migrate_During_Write_Count[0]))  #15

    tmp_Valid_Block_Evicted            =re.findall('Valid Blocks Evicted[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)              
    Valid_Block_Evicted            =re.findall('[\d]+',tmp_Valid_Block_Evicted[0])              
    paras.append(float(Valid_Block_Evicted[0]))  #16



    tmp_SRAM_Read_Before_Migration    =re.findall('SRAM Read Before Migration    [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    SRAM_Read_Before_Migration    =re.findall('[\d]+',tmp_SRAM_Read_Before_Migration[0])    
    paras.append(float(SRAM_Read_Before_Migration[0]))      #17
    
    tmp_SRAM_Read_After_Migration     =re.findall('SRAM Read After Migration     [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    SRAM_Read_After_Migration     =re.findall('[\d]+',tmp_SRAM_Read_After_Migration[0])     
    paras.append(float(SRAM_Read_After_Migration[0]))       #18
    
    tmp_SRAM_Write_Before_Migration   =re.findall('SRAM Write Before Migration   [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    SRAM_Write_Before_Migration   =re.findall('[\d]+',tmp_SRAM_Write_Before_Migration[0])   
    paras.append(float(SRAM_Write_Before_Migration[0]))     #19
    
    tmp_SRAM_Write_After_Migration    =re.findall('SRAM Write After Migration    [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    SRAM_Write_After_Migration    =re.findall('[\d]+',tmp_SRAM_Write_After_Migration[0])    
    paras.append(float(SRAM_Write_After_Migration[0]))      #20
    
    tmp_STTRAM_Read_Before_Migration  =re.findall('STTRAM Read Before Migration  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    STTRAM_Read_Before_Migration  =re.findall('[\d]+',tmp_STTRAM_Read_Before_Migration[0])  
    paras.append(float(STTRAM_Read_Before_Migration[0]))    #21
    
    tmp_STTRAM_Read_After_Migration   =re.findall('STTRAM Read After Migration   [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    STTRAM_Read_After_Migration   =re.findall('[\d]+',tmp_STTRAM_Read_After_Migration[0])   
    paras.append(float(STTRAM_Read_After_Migration[0]))     #22
    
    tmp_STTRAM_Write_Before_Migration =re.findall('STTRAM Write Before Migration [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    STTRAM_Write_Before_Migration =re.findall('[\d]+',tmp_STTRAM_Write_Before_Migration[0]) 
    paras.append(float(STTRAM_Write_Before_Migration[0]))   #23
    
    tmp_STTRAM_Write_After_Migration  =re.findall('STTRAM Write After Migration  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    STTRAM_Write_After_Migration  =re.findall('[\d]+',tmp_STTRAM_Write_After_Migration[0])  
    paras.append(float(STTRAM_Write_After_Migration[0]))    #24



    tmp_living_SRAM_Blocks_Evicted  =re.findall('living SRAM Blocks Evicted  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    living_SRAM_Blocks_Evicted  =re.findall('[\d]+',tmp_living_SRAM_Blocks_Evicted[0])  
    paras.append(float(living_SRAM_Blocks_Evicted[0]))    #25

    tmp_dead_SRAM_Blocks_Evicted  =re.findall('dead SRAM Blocks Evicted  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    dead_SRAM_Blocks_Evicted  =re.findall('[\d]+',tmp_dead_SRAM_Blocks_Evicted[0])  
    paras.append(float(dead_SRAM_Blocks_Evicted[0]))    #26



    tmp_single_Migration_Count  =re.findall('single Migration Count  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    single_Migration_Count  =re.findall('[\d]+',tmp_single_Migration_Count[0])  
    paras.append(float(single_Migration_Count[0]))    #27

    tmp_double_Migration_Count  =re.findall('double Migration Count  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    double_Migration_Count  =re.findall('[\d]+',tmp_double_Migration_Count[0])  
    paras.append(float(double_Migration_Count[0]))    #28

    tmp_no_Migration_Count  =re.findall('no Migration Count  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    no_Migration_Count  =re.findall('[\d]+',tmp_no_Migration_Count[0])  
    paras.append(float(no_Migration_Count[0]))    #29



    tmp_g_STTr_Write  =re.findall('g STTr Write  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    g_STTr_Write  =re.findall('[\d]+',tmp_g_STTr_Write[0])      
    paras.append(float(g_STTr_Write[0]))    #30
    
    tmp_g_STTr_Reads  =re.findall('g STTr Reads  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    g_STTr_Reads  =re.findall('[\d]+',tmp_g_STTr_Reads[0])      
    paras.append(float(g_STTr_Reads[0]))    #31
    
    tmp_g_Sram_Write  =re.findall('g Sram Write  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    g_Sram_Write  =re.findall('[\d]+',tmp_g_Sram_Write[0])      
    paras.append(float(g_Sram_Write[0]))    #32
    
    tmp_g_Sram_Reads  =re.findall('g Sram Reads  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    g_Sram_Reads  =re.findall('[\d]+',tmp_g_Sram_Reads[0])      
    paras.append(float(g_Sram_Reads[0]))    #33







    ###########################################################################
    ############################################################################

    
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

  #paras contains [benchmark_trace, num_instrutions, num_cycles, cpi, ipc]: 
  #                 0               1                 2           3   4             
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
  cumm_wgt_rwte = 0
  cumm_wgt_ribc = 0
  cumm_wgt_wibc = 0
  cumm_wgt_dbc = 0
  cumm_wgt_Valid_Block_Evicted = 0

  cumm_wgt_Read_Intense_Blocks_M                  = 0
  cumm_wgt_Write_Intense_Blocks_M                 = 0
  cumm_wgt_Read_Intense_Blocks_MDuringMigrate      = 0
  cumm_wgt_Write_Intense_Blocks_MDuringMigrate     = 0
  
  cumm_wgt_M_Count                                = 0
  cumm_wgt_Migrate_During_Write_Count              = 0

  cumm_wgt_SRAM_Read_Before_Migration          = 0
  cumm_wgt_SRAM_Read_After_Migration           = 0
  cumm_wgt_SRAM_Write_Before_Migration         = 0
  cumm_wgt_SRAM_Write_After_Migration          = 0
  cumm_wgt_STTRAM_Read_Before_Migration        = 0
  cumm_wgt_STTRAM_Read_After_Migration         = 0
  cumm_wgt_STTRAM_Write_Before_Migration       = 0
  cumm_wgt_STTRAM_Write_After_Migration        = 0

  cumm_wgt_living_SRAM_Blocks_Evicted        = 0
  cumm_wgt_dead_SRAM_Blocks_Evicted        = 0

  cumm_wgt_single_Migration_Count           = 0
  cumm_wgt_double_Migration_Count           = 0
  cumm_wgt_no_Migration_Count               = 0

  cumm_wgt_g_STTr_Write      = 0
  cumm_wgt_g_STTr_Reads      = 0
  cumm_wgt_g_Sram_Write      = 0
  cumm_wgt_g_Sram_Reads      = 0
  

  print "results directory: "+ip_dir
  
  if num_apps == '2':
    fop.write("app1\tmpki\tmisr\tcpi\t\tdeadB\t\tinsB\t\tapp2\tmpki\tmisr\tcpi\t\tdeadB\t\tinsB\tinvB\n")
  else:  
    fop.write("app1, CPI, IPC, l3_access, l3_miss, l3_miss_r, l3_mpki, wgt, ribc, wibc, dbc, wrte, rwte, Read_Intense_Blocks_M, Write_Intense_Blocks_M, Read_Intense_Blocks_MDuringMigrate, Write_Intense_Blocks_MDuringMigrate, M_Count, Migrate_During_Write_Count, Valid_Block_Evicted, SRAM_Read_Before_Migration, SRAM_Read_After_Migration, SRAM_Write_Before_Migration, SRAM_Write_After_Migration, STTRAM_Read_Before_Migration, STTRAM_Read_After_Migration, STTRAM_Write_Before_Migration, STTRAM_Write_After_Migration, living_SRAM_Blocks_Evicted, dead_SRAM_Blocks_Evicted, single_Migration_Count, double_Migration_Count, no_Migration_Count, g_STTr_Write, g_STTr_Reads, g_Sram_Write, g_Sram_Reads\n")
  

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

      #arindam##

      ribc = paras[5]
      print ribc
      wibc = paras[6]
      print wibc
      dbc = paras[7]
      print dbc

            

      wrte = paras[8]
      print wrte
      rwte = paras[9]
      print rwte

      ###################################
      ###################################
      Read_Intense_Blocks_M= paras[10]
      print Read_Intense_Blocks_M

      Write_Intense_Blocks_M= paras[11]
      print Write_Intense_Blocks_M



      Read_Intense_Blocks_MDuringMigrate        = paras[12]

      print   Read_Intense_Blocks_MDuringMigrate



      Write_Intense_Blocks_MDuringMigrate       = paras[13]

      print   Write_Intense_Blocks_MDuringMigrate




      M_Count = paras[14]
      print M_Count



      Migrate_During_Write_Count = paras[15]
      print Migrate_During_Write_Count


      Valid_Block_Evicted  = paras[16]
      print Valid_Block_Evicted




      SRAM_Read_Before_Migration      = paras[17]   
      print  SRAM_Read_Before_Migration    
      
      SRAM_Read_After_Migration       = paras[18] 
      print  SRAM_Read_After_Migration     
      
      SRAM_Write_Before_Migration     = paras[19] 
      print  SRAM_Write_Before_Migration   
      
      SRAM_Write_After_Migration      = paras[20] 
      print  SRAM_Write_After_Migration    
      
      STTRAM_Read_Before_Migration    = paras[21] 
      print  STTRAM_Read_Before_Migration  
      
      STTRAM_Read_After_Migration     = paras[22] 
      print  STTRAM_Read_After_Migration   
      
      STTRAM_Write_Before_Migration   = paras[23] 
      print  STTRAM_Write_Before_Migration 
      
      STTRAM_Write_After_Migration    = paras[24] 
      print  STTRAM_Write_After_Migration  



      living_SRAM_Blocks_Evicted    = paras[25] 
      print  living_SRAM_Blocks_Evicted  

      dead_SRAM_Blocks_Evicted    = paras[26] 
      print  dead_SRAM_Blocks_Evicted



      single_Migration_Count    = paras[27] 
      print  single_Migration_Count

      double_Migration_Count    = paras[28] 
      print  double_Migration_Count

      no_Migration_Count    = paras[29] 
      print  no_Migration_Count  



      g_STTr_Write  =   paras[30]           
      print  g_STTr_Write
      
      g_STTr_Reads    =   paras[31]           
      print  g_STTr_Reads
      
      g_Sram_Write    =   paras[32]           
      print  g_Sram_Write
      
      g_Sram_Reads    =   paras[33]           
      print  g_Sram_Reads





      ##################################
      ##################################



      access = paras[34]
      print access

      miss = paras[35]
      print miss

      miss_r = paras[36]
      print miss_r

      mpki = paras[37]
      print mpki


      ##################################

      

      
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
      wgt_miss = miss*wgt
      print "Weighted l3 miss = "+str(wgt_miss)
      cumm_wgt_miss += wgt_miss
      print "Cummulative Weighted l3 miss = "+str(cumm_wgt_miss)


      wgt_wrte = wrte*wgt
      print "Weighted wrte = "+str(wgt_wrte)
      cumm_wgt_wrte += wgt_wrte
      print "Cummulative Weighted wrte = "+str(cumm_wgt_wrte)

      wgt_rwte = rwte*wgt
      print "Weighted rwte = "+str(wgt_rwte)
      cumm_wgt_rwte += wgt_rwte
      print "Cummulative Weighted rwte = "+str(cumm_wgt_rwte)

      wgt_ribc = ribc*wgt
      print "Weighted ribc = "+str(wgt_ribc)
      cumm_wgt_ribc += wgt_ribc
      print "Cummulative Weighted ribc = "+str(cumm_wgt_ribc)

      wgt_wibc = wibc*wgt
      print "Weighted wibc = "+str(wgt_wibc)
      cumm_wgt_wibc += wgt_wibc
      print "Cummulative Weighted wibc = "+str(cumm_wgt_wibc)




      #######################################
      #######################################

      wgt_Read_Intense_Blocks_M  = wgt*Read_Intense_Blocks_M                    

      print   "Weighted Read Intense Blocks M = "+str(wgt_Read_Intense_Blocks_M)

      cumm_wgt_Read_Intense_Blocks_M += wgt_Read_Intense_Blocks_M                                          

      print "Cummulative Weighted Read Intense Blocks M = "+str(cumm_wgt_Read_Intense_Blocks_M)                

      

      wgt_Write_Intense_Blocks_M = wgt*Write_Intense_Blocks_M                              

      print   "Weighted Write Intense Blocks M  = "+str(wgt_Write_Intense_Blocks_M)                              

      cumm_wgt_Write_Intense_Blocks_M += wgt_Write_Intense_Blocks_M                                        

      print "Cummulative Weighted Write Intense Blocks M = "+str(cumm_wgt_Write_Intense_Blocks_M)                

      

      wgt_Read_Intense_Blocks_MDuringMigrate  = wgt*Read_Intense_Blocks_MDuringMigrate       

      print   "Weighted Read Intense Blocks MDuringMigrate = "+str(wgt_Read_Intense_Blocks_MDuringMigrate)         

      cumm_wgt_Read_Intense_Blocks_MDuringMigrate += wgt_Read_Intense_Blocks_MDuringMigrate                  

      print "Cummulative Weighted Read Intense Blocks MDuringMigrate = "+str(cumm_wgt_Read_Intense_Blocks_MDuringMigrate)                

      

      wgt_Write_Intense_Blocks_MDuringMigrate = wgt*Write_Intense_Blocks_MDuringMigrate      

      print   "Weighted Write Intense Blocks MDuringMigrate  = "+str(wgt_Write_Intense_Blocks_MDuringMigrate)      

      cumm_wgt_Write_Intense_Blocks_MDuringMigrate += wgt_Write_Intense_Blocks_MDuringMigrate                

      print "Cummulative Weighted Write Intense Blocks MDuringMigrate = "+str(cumm_wgt_Write_Intense_Blocks_MDuringMigrate)                

                  

      

      wgt_M_Count  = wgt*M_Count                                                           

      print   "Weighted M Count = "+str(wgt_M_Count)                                                             

      cumm_wgt_M_Count += wgt_M_Count                                                                      

      print "Cummulative Weighted M Count = "+str(cumm_wgt_M_Count)

      

      wgt_Migrate_During_Write_Count  = wgt*Migrate_During_Write_Count                  

      print   "Weighted Migrate During Write Count = "+str(wgt_Migrate_During_Write_Count)                         

      cumm_wgt_Migrate_During_Write_Count += wgt_Migrate_During_Write_Count                                  

      print "Cummulative Weighted Migrate During Write Count = "+str(cumm_wgt_Migrate_During_Write_Count)



      wgt_Valid_Block_Evicted  = wgt*Valid_Block_Evicted                  
      print   "Weighted Valid Blocks Evicted = "+str(wgt_Valid_Block_Evicted)                         
      cumm_wgt_Valid_Block_Evicted += wgt_Valid_Block_Evicted                                  
      print "Cummulative Weighted Valid Blocks Evicted = "+str(cumm_wgt_Valid_Block_Evicted)  




      wgt_SRAM_Read_Before_Migration      = wgt*SRAM_Read_Before_Migration        
      print "Weighted SRAM Read Before Migration     = "+str(wgt_SRAM_Read_Before_Migration)      
      cumm_wgt_SRAM_Read_Before_Migration     += wgt_SRAM_Read_Before_Migration         
      print "Cummulative Weighted SRAM Read Before Migration     = "+str(cumm_wgt_SRAM_Read_Before_Migration)  
      
      wgt_SRAM_Read_After_Migration       = wgt*SRAM_Read_After_Migration         
      print "Weighted SRAM Read After Migration      = "+str(wgt_SRAM_Read_After_Migration)       
      cumm_wgt_SRAM_Read_After_Migration      += wgt_SRAM_Read_After_Migration          
      print "Cummulative Weighted SRAM Read After Migration      = "+str(cumm_wgt_SRAM_Read_After_Migration)  
      
      wgt_SRAM_Write_Before_Migration     = wgt*SRAM_Write_Before_Migration       
      print "Weighted SRAM Write Before Migration    = "+str(wgt_SRAM_Write_Before_Migration)     
      cumm_wgt_SRAM_Write_Before_Migration    += wgt_SRAM_Write_Before_Migration        
      print "Cummulative Weighted SRAM Write Before Migration    = "+str(cumm_wgt_SRAM_Write_Before_Migration)  
      
      wgt_SRAM_Write_After_Migration      = wgt*SRAM_Write_After_Migration        
      print "Weighted SRAM Write After Migration     = "+str(wgt_SRAM_Write_After_Migration)      
      cumm_wgt_SRAM_Write_After_Migration     += wgt_SRAM_Write_After_Migration         
      print "Cummulative Weighted SRAM Write After Migration     = "+str(cumm_wgt_SRAM_Write_After_Migration)  
      
      wgt_STTRAM_Read_Before_Migration    = wgt*STTRAM_Read_Before_Migration      
      print "Weighted STTRAM Read Before Migration   = "+str(wgt_STTRAM_Read_Before_Migration)    
      cumm_wgt_STTRAM_Read_Before_Migration   += wgt_STTRAM_Read_Before_Migration       
      print "Cummulative Weighted STTRAM Read Before Migration   = "+str(cumm_wgt_STTRAM_Read_Before_Migration)  
      
      wgt_STTRAM_Read_After_Migration     = wgt*STTRAM_Read_After_Migration       
      print "Weighted STTRAM Read After Migration    = "+str(wgt_STTRAM_Read_After_Migration)     
      cumm_wgt_STTRAM_Read_After_Migration    += wgt_STTRAM_Read_After_Migration        
      print "Cummulative Weighted STTRAM Read After Migration    = "+str(cumm_wgt_STTRAM_Read_After_Migration)  
      
      wgt_STTRAM_Write_Before_Migration   = wgt*STTRAM_Write_Before_Migration     
      print "Weighted STTRAM Write Before Migration  = "+str(wgt_STTRAM_Write_Before_Migration)   
      cumm_wgt_STTRAM_Write_Before_Migration  += wgt_STTRAM_Write_Before_Migration     
      print "Cummulative Weighted STTRAM Write Before Migration  = "+str(cumm_wgt_STTRAM_Write_Before_Migration)  
      
      wgt_STTRAM_Write_After_Migration    = wgt*STTRAM_Write_After_Migration      
      print "Weighted STTRAM Write After Migration   = "+str(wgt_STTRAM_Write_After_Migration)    
      cumm_wgt_STTRAM_Write_After_Migration   += wgt_STTRAM_Write_After_Migration       
      print "Cummulative Weighted STTRAM Write After Migration   = "+str(cumm_wgt_STTRAM_Write_After_Migration) 



      wgt_living_SRAM_Blocks_Evicted    = wgt*living_SRAM_Blocks_Evicted      
      print "Weighted living SRAM Blocks Evicted   = "+str(wgt_living_SRAM_Blocks_Evicted)    
      cumm_wgt_living_SRAM_Blocks_Evicted   += wgt_living_SRAM_Blocks_Evicted      
      print "Cummulative Weighted living SRAM Blocks Evicted   = "+str(cumm_wgt_living_SRAM_Blocks_Evicted) 

      wgt_dead_SRAM_Blocks_Evicted    = wgt*dead_SRAM_Blocks_Evicted      
      print "Weighted dead SRAM Blocks Evicted   = "+str(wgt_dead_SRAM_Blocks_Evicted)    
      cumm_wgt_dead_SRAM_Blocks_Evicted  += wgt_dead_SRAM_Blocks_Evicted       
      print "Cummulative Weighted dead SRAM Blocks Evicted   = "+str(cumm_wgt_dead_SRAM_Blocks_Evicted)



      wgt_single_Migration_Count    = wgt*single_Migration_Count      
      print "Weighted single Migration Count   = "+str(wgt_single_Migration_Count)    
      cumm_wgt_single_Migration_Count  += wgt_single_Migration_Count       
      print "Cummulative Weighted single Migration Count   = "+str(cumm_wgt_single_Migration_Count)

      wgt_double_Migration_Count    = wgt*double_Migration_Count      
      print "Weighted double Migration Count   = "+str(wgt_double_Migration_Count)    
      cumm_wgt_double_Migration_Count  += wgt_double_Migration_Count       
      print "Cummulative Weighted double Migration Count   = "+str(cumm_wgt_double_Migration_Count)

      wgt_no_Migration_Count    = wgt*no_Migration_Count      
      print "Weighted no Migration Count   = "+str(wgt_no_Migration_Count)    
      cumm_wgt_no_Migration_Count  += wgt_no_Migration_Count       
      print "Cummulative Weighted no_Migration_Count   = "+str(cumm_wgt_no_Migration_Count)




      wgt_g_STTr_Write = wgt*g_STTr_Write      
      print "Weighted g STTr Write   = "+str(wgt_g_STTr_Write)    
      cumm_wgt_g_STTr_Write  += wgt_g_STTr_Write       
      print "Cummulative Weighted g STTr Write   = "+str(cumm_wgt_g_STTr_Write)
      
      wgt_g_STTr_Reads = wgt*g_STTr_Reads      
      print "Weighted g STTr Reads   = "+str(wgt_g_STTr_Reads)    
      cumm_wgt_g_STTr_Reads  += wgt_g_STTr_Reads       
      print "Cummulative Weighted g STTr Reads   = "+str(cumm_wgt_g_STTr_Reads)
      
      wgt_g_Sram_Write = wgt*g_Sram_Write      
      print "Weighted g Sram Write   = "+str(wgt_g_Sram_Write)    
      cumm_wgt_g_Sram_Write  += wgt_g_Sram_Write       
      print "Cummulative Weighted g Sram Write   = "+str(cumm_wgt_g_Sram_Write)
      
      wgt_g_Sram_Reads = wgt*g_Sram_Reads      
      print "Weighted g Sram Reads   = "+str(wgt_g_Sram_Reads)    
      cumm_wgt_g_Sram_Reads  += wgt_g_Sram_Reads       
      print "Cummulative Weighted g Sram Reads   = "+str(cumm_wgt_g_Sram_Reads)       


              




      #######################################
      #######################################







      wgt_dbc = dbc*wgt
      print "Weighted dbc = "+str(wgt_dbc)
      cumm_wgt_dbc += wgt_dbc
      print "Cummulative Weighted dbc = "+str(cumm_wgt_dbc)


      print "================================================"      


      print "CPI: "+format((float(paras[2])/float(paras[1])),'.2f')
      print "IPC: "+format((float(paras[1])/float(paras[2])),'.2f')
      
      fop.write(files+","+str(cpi)+","+str(ipc)+","+str(access)+","+str(miss)+","+str(miss_r)+","+str(mpki)+","+str(wgt)+","+str(ribc)+","+str(wibc)+","+str(dbc)+","+str(wrte)+","+str(rwte)+","+str(Read_Intense_Blocks_M)+","+str(Write_Intense_Blocks_M)+","+str(Read_Intense_Blocks_MDuringMigrate)+","+str(Write_Intense_Blocks_MDuringMigrate)+","+str(M_Count)+","+str(Migrate_During_Write_Count)+","+str(Valid_Block_Evicted)+","+str(SRAM_Read_Before_Migration)+","+str(SRAM_Read_After_Migration)+","+str(SRAM_Write_Before_Migration)+","+str(SRAM_Write_After_Migration)+","+str(STTRAM_Read_Before_Migration)+","+str(STTRAM_Read_After_Migration)+","+str(STTRAM_Write_Before_Migration)+","+str(STTRAM_Write_After_Migration)+","+str(living_SRAM_Blocks_Evicted)+","+str(dead_SRAM_Blocks_Evicted)+","+str(single_Migration_Count)+","+str(double_Migration_Count)+","+str(no_Migration_Count)+","+str(g_STTr_Write)+","+str(g_STTr_Reads)+","+str(g_Sram_Write)+","+str(g_Sram_Reads)+"\n")
     
      
      print "================================"

  Weighted_cpi = cumm_wgt_cpi/cumm_wgt
  Weighted_cpi = format(Weighted_cpi, '.2f')
  Weighted_ipc = cumm_wgt/cumm_wgt_cpi
  Weighted_ipc = format(Weighted_ipc, '.2f')
  Weighted_mpki = cumm_wgt_mpki/cumm_wgt
  Weighted_mpki = format(Weighted_mpki, '.2f')

  #Arindam
  Weighted_l3miss = cumm_wgt_miss/cumm_wgt
  Weighted_l3miss = format(Weighted_l3miss, '.0f')
  
  Weighted_wrte = cumm_wgt_wrte/cumm_wgt
  Weighted_wrte = format(Weighted_wrte, '.0f')

  Weighted_rwte = cumm_wgt_rwte/cumm_wgt
  Weighted_rwte = format(Weighted_rwte, '.0f')



  ####################################################
  ####################################################

  Weighted_Read_Intense_Blocks_M                = cumm_wgt_Read_Intense_Blocks_M/cumm_wgt                     
  Weighted_Read_Intense_Blocks_M               = format(Weighted_Read_Intense_Blocks_M, '.0f')

  Weighted_Write_Intense_Blocks_M               = cumm_wgt_Write_Intense_Blocks_M/cumm_wgt                    
  Weighted_Write_Intense_Blocks_M              = format(Weighted_Write_Intense_Blocks_M, '.0f')

  Weighted_Read_Intense_Blocks_MDuringMigrate    = cumm_wgt_Read_Intense_Blocks_MDuringMigrate/cumm_wgt         
  Weighted_Read_Intense_Blocks_MDuringMigrate   = format(Weighted_Read_Intense_Blocks_MDuringMigrate, '.0f')

  Weighted_Write_Intense_Blocks_MDuringMigrate   = cumm_wgt_Write_Intense_Blocks_MDuringMigrate/cumm_wgt        
  Weighted_Write_Intense_Blocks_MDuringMigrate  = format(Weighted_Write_Intense_Blocks_MDuringMigrate, '.0f')

  

  Weighted_M_Count                              = cumm_wgt_M_Count/cumm_wgt                                   
  Weighted_M_Count                             = format(Weighted_M_Count, '.0f')

  Weighted_Migrate_During_Write_Count            = cumm_wgt_Migrate_During_Write_Count/cumm_wgt                 
  Weighted_Migrate_During_Write_Count           = format(Weighted_Migrate_During_Write_Count, '.0f')

  Weighted_Valid_Block_Evicted            = cumm_wgt_Valid_Block_Evicted/cumm_wgt                 
  Weighted_Valid_Block_Evicted           = format(Weighted_Valid_Block_Evicted, '.0f')




  Weighted_SRAM_Read_Before_Migration     = cumm_wgt_SRAM_Read_Before_Migration/cumm_wgt        
  Weighted_SRAM_Read_Before_Migration     = format(Weighted_SRAM_Read_Before_Migration, '.0f')
  
  Weighted_SRAM_Read_After_Migration      = cumm_wgt_SRAM_Read_After_Migration/cumm_wgt           
  Weighted_SRAM_Read_After_Migration      = format(Weighted_SRAM_Read_After_Migration, '.0f')
  
  Weighted_SRAM_Write_Before_Migration    = cumm_wgt_SRAM_Write_Before_Migration/cumm_wgt         
  Weighted_SRAM_Write_Before_Migration    = format(Weighted_SRAM_Write_Before_Migration, '.0f')
  
  Weighted_SRAM_Write_After_Migration     = cumm_wgt_SRAM_Write_After_Migration/cumm_wgt          
  Weighted_SRAM_Write_After_Migration     = format(Weighted_SRAM_Write_After_Migration, '.0f')
  
  Weighted_STTRAM_Read_Before_Migration   = cumm_wgt_STTRAM_Read_Before_Migration/cumm_wgt        
  Weighted_STTRAM_Read_Before_Migration   = format(Weighted_STTRAM_Read_Before_Migration, '.0f')
  
  Weighted_STTRAM_Read_After_Migration    = cumm_wgt_STTRAM_Read_After_Migration/cumm_wgt         
  Weighted_STTRAM_Read_After_Migration    = format(Weighted_STTRAM_Read_After_Migration, '.0f')
  
  Weighted_STTRAM_Write_Before_Migration  = cumm_wgt_STTRAM_Write_Before_Migration/cumm_wgt       
  Weighted_STTRAM_Write_Before_Migration  = format(Weighted_STTRAM_Write_Before_Migration, '.0f')
  
  Weighted_STTRAM_Write_After_Migration   = cumm_wgt_STTRAM_Write_After_Migration/cumm_wgt        
  Weighted_STTRAM_Write_After_Migration   = format(Weighted_STTRAM_Write_After_Migration, '.0f')



  Weighted_living_SRAM_Blocks_Evicted   = cumm_wgt_living_SRAM_Blocks_Evicted/cumm_wgt        
  Weighted_living_SRAM_Blocks_Evicted   = format(Weighted_living_SRAM_Blocks_Evicted, '.0f')

  Weighted_dead_SRAM_Blocks_Evicted   = cumm_wgt_dead_SRAM_Blocks_Evicted/cumm_wgt        
  Weighted_dead_SRAM_Blocks_Evicted   = format(Weighted_dead_SRAM_Blocks_Evicted, '.0f')



  Weighted_single_Migration_Count   = cumm_wgt_single_Migration_Count/cumm_wgt        
  Weighted_single_Migration_Count   = format(Weighted_single_Migration_Count, '.0f')

  Weighted_double_Migration_Count   = cumm_wgt_double_Migration_Count/cumm_wgt        
  Weighted_double_Migration_Count   = format(Weighted_double_Migration_Count, '.0f')

  Weighted_no_Migration_Count   = cumm_wgt_no_Migration_Count/cumm_wgt        
  Weighted_no_Migration_Count   = format(Weighted_no_Migration_Count, '.0f')




  Weighted_g_STTr_Write = cumm_wgt_g_STTr_Write/cumm_wgt        
  Weighted_g_STTr_Write = format(Weighted_g_STTr_Write, '.0f')
  
  Weighted_g_STTr_Reads = cumm_wgt_g_STTr_Reads/cumm_wgt        
  Weighted_g_STTr_Reads = format(Weighted_g_STTr_Reads, '.0f')
  
  Weighted_g_Sram_Write = cumm_wgt_g_Sram_Write/cumm_wgt        
  Weighted_g_Sram_Write = format(Weighted_g_Sram_Write, '.0f')
  
  Weighted_g_Sram_Reads = cumm_wgt_g_Sram_Reads/cumm_wgt        
  Weighted_g_Sram_Reads = format(Weighted_g_Sram_Reads, '.0f')





  ####################################################
  ####################################################



  Weighted_ribc = cumm_wgt_ribc/cumm_wgt
  Weighted_ribc = format(Weighted_ribc, '.0f')
  Weighted_wibc = cumm_wgt_wibc/cumm_wgt
  Weighted_wibc = format(Weighted_wibc, '.0f')
  Weighted_dbc = cumm_wgt_dbc/cumm_wgt
  Weighted_dbc = format(Weighted_dbc, '.0f')

  fop.write("\n\n")
  #normalizing mpki by dividing it with weight
  fop.write("Benchmark, Weighted IPC, Weighted L3 Miss, Weighted ribc, Weighted wibc, Weighted dbc, Weighted wrte, Weighted rwte, Weighted Read Intense Blocks M, Weighted Write Intense Blocks M, Weighted Read Intense Blocks MDuringMigrate, Weighted Write Intense Blocks MDuringMigrate, Weighted M Count, Weighted Migrate During Write Count, Weighted Valid Blocks Evicted, Weighted SRAM Read Before Migration, Weighted SRAM Read After Migration, Weighted SRAM Write Before Migration, Weighted SRAM Write After Migration, Weighted STTRAM Read Before Migration, Weighted STTRAM Read After Migration, Weighted STTRAM Write Before Migration, Weighted STTRAM Write After Migration, living SRAM Blocks Evicted, dead SRAM Blocks Evicted, single Migration Count, double Migration Count, no Migration Count, Weighted g STTr Write, Weighted g STTr Reads, Weighted g Sram Write, Weighted g Sram Reads\n")
  #fop.write(application+","+str(Weighted_cpi)+","+str(Weighted_ipc)+","+Weighted_mpki+"\n")
  fop.write(application+","+str(Weighted_ipc)+","+str(Weighted_l3miss)+","+str(Weighted_ribc)+","+str(Weighted_wibc)+","+str(Weighted_dbc)+","+str(Weighted_wrte)+","+str(Weighted_rwte)+","+str(Weighted_Read_Intense_Blocks_M)+","+str(Weighted_Write_Intense_Blocks_M)+","+str(Weighted_Read_Intense_Blocks_MDuringMigrate)+","+str(Weighted_Write_Intense_Blocks_MDuringMigrate)+","+str(Weighted_M_Count)+","+str(Weighted_Migrate_During_Write_Count)+","+str(Weighted_Valid_Block_Evicted)+","+str(Weighted_SRAM_Read_Before_Migration)+","+str(Weighted_SRAM_Read_After_Migration)+","+str(Weighted_SRAM_Write_Before_Migration)+","+str(Weighted_SRAM_Write_After_Migration)+","+str(Weighted_STTRAM_Read_Before_Migration)+","+str(Weighted_STTRAM_Read_After_Migration)+","+str(Weighted_STTRAM_Write_Before_Migration)+","+str(Weighted_STTRAM_Write_After_Migration)+","+str(Weighted_living_SRAM_Blocks_Evicted)+","+str(Weighted_dead_SRAM_Blocks_Evicted)+","+str(Weighted_single_Migration_Count)+","+str(Weighted_double_Migration_Count)+","+str(Weighted_no_Migration_Count)+","+str(Weighted_g_STTr_Write)+","+str(Weighted_g_STTr_Reads)+","+str(Weighted_g_Sram_Write)+","+str(Weighted_g_Sram_Reads)+"\n")

  ts = time.time()
  st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
  fop.write("\n\n\n\n"+st)
           
if __name__ == "__main__":
  main()
