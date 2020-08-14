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



    tmp_a_SRAM  =re.findall('a SRAM  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    a_SRAM  =re.findall('[\d]+',tmp_a_SRAM[0])      
    paras.append(float(a_SRAM[0]))    #34
    
    tmp_b_SRAM  =re.findall('b SRAM  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    b_SRAM  =re.findall('[\d]+',tmp_b_SRAM[0])      
    paras.append(float(b_SRAM[0]))    #35
    
    tmp_c_SRAM  =re.findall('c SRAM  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    c_SRAM  =re.findall('[\d]+',tmp_c_SRAM[0])      
    paras.append(float(c_SRAM[0]))    #36
    
    tmp_d_SRAM  =re.findall('d SRAM  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    d_SRAM  =re.findall('[\d]+',tmp_d_SRAM[0])      
    paras.append(float(d_SRAM[0]))    #37
    
    tmp_e_SRAM  =re.findall('e SRAM  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    e_SRAM  =re.findall('[\d]+',tmp_e_SRAM[0])      
    paras.append(float(e_SRAM[0]))    #38
    
    tmp_f_SRAM  =re.findall('f SRAM  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    f_SRAM  =re.findall('[\d]+',tmp_f_SRAM[0])      
    paras.append(float(f_SRAM[0]))    #39
    
    tmp_g_SRAM  =re.findall('g SRAM  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    g_SRAM  =re.findall('[\d]+',tmp_g_SRAM[0])      
    paras.append(float(g_SRAM[0]))    #40
    
    tmp_a_STTR  =re.findall('a STTR  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    a_STTR  =re.findall('[\d]+',tmp_a_STTR[0])      
    paras.append(float(a_STTR[0]))    #41
    
    tmp_b_STTR  =re.findall('b STTR  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    b_STTR  =re.findall('[\d]+',tmp_b_STTR[0])      
    paras.append(float(b_STTR[0]))    #42
    
    tmp_c_STTR  =re.findall('c STTR  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    c_STTR  =re.findall('[\d]+',tmp_c_STTR[0])      
    paras.append(float(c_STTR[0]))    #43
    
    tmp_d_STTR  =re.findall('d STTR  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    d_STTR  =re.findall('[\d]+',tmp_d_STTR[0])      
    paras.append(float(d_STTR[0]))    #44
    
    tmp_e_STTR  =re.findall('e STTR  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    e_STTR  =re.findall('[\d]+',tmp_e_STTR[0])      
    paras.append(float(e_STTR[0]))    #45
    
    tmp_f_STTR  =re.findall('f STTR  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    f_STTR  =re.findall('[\d]+',tmp_f_STTR[0])      
    paras.append(float(f_STTR[0]))    #46
    
    tmp_g_STTR  =re.findall('g STTR  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    g_STTR  =re.findall('[\d]+',tmp_g_STTR[0])      
    paras.append(float(g_STTR[0]))    #47



    tmp_force_Migration_SramToSTTram  =re.findall('force Migration SramToSTTram  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    force_Migration_SramToSTTram  =re.findall('[\d]+',tmp_force_Migration_SramToSTTram[0])      
    paras.append(float(force_Migration_SramToSTTram[0]))    #48
    
    tmp_force_Migration_STTramToSram  =re.findall('force Migration STTramToSram  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    force_Migration_STTramToSram  =re.findall('[\d]+',tmp_force_Migration_STTramToSram[0])      
    paras.append(float(force_Migration_STTramToSram[0]))    #49



    tmp_SRAM_reads_hits  =re.findall('SRAM reads hits  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    SRAM_reads_hits  =re.findall('[\d]+',tmp_SRAM_reads_hits[0])      
    paras.append(float(SRAM_reads_hits[0]))    #50
    
    tmp_SRAM_write_hits  =re.findall('SRAM write hits  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    SRAM_write_hits  =re.findall('[\d]+',tmp_SRAM_write_hits[0])      
    paras.append(float(SRAM_write_hits[0]))    #51
    
    tmp_STTR_reads_hits  =re.findall('STTR reads hits  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    STTR_reads_hits  =re.findall('[\d]+',tmp_STTR_reads_hits[0])      
    paras.append(float(STTR_reads_hits[0]))    #52
    
    tmp_STTR_write_hits  =re.findall('STTR write hits  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    STTR_write_hits  =re.findall('[\d]+',tmp_STTR_write_hits[0])      
    paras.append(float(STTR_write_hits[0]))    #53




    ###########################################################################
    ############################################################################

    
    ########


    caches = ["L1-D", "L2", "L3"]
    str1 = 'Cache '
    str2 = '[\s]+[|]+[\s]+[\n]+[\s]+[\w\s\d|\.\%]+'
    for cache in caches:
        string = str1 + cache + str2

        tmp=re.findall(string,str) # extract parameters of the L3
        tmp=tmp[0]
        print 'Newton' + cache
        print tmp
        access = re.findall('num cache accesses[\s]+[|]+[\s\d]+',tmp) #extract num accesses
        access = access[0].split('|')[1].strip()
        miss   = re.findall('num cache misses[\s]+[|]+[\s\d]+',tmp)  #extract num miss
        miss   = miss[0].split('|')[1].strip()
        miss_r = re.findall('miss rate[\s]+[|]+[\s\d".%"]+',tmp)   #extract num miss
        miss_r = miss_r[0].split('|')[1].strip()
        miss_r = miss_r.split('%')[0].strip()
        mpki   = re.findall('mpki[[\s]+[|]+[\s\d"."]+',tmp) #extract mpki
        mpki   = mpki[0].split('|')[1].strip()

        paras.append(int(access))
        paras.append(int(miss))
        paras.append(float(miss_r))
        paras.append(float(mpki))
        print paras
    print 'done'

    #L1-D goes in paras 54 55 56 57
    #L2 goes in paras 58 59 60 61
    #L3 goes in paras 62 63 64 65


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

  wgt_accessl1 = 0
  wgt_missl1 = 0
  wgt_miss_rl1 = 0
  wgt_mpkil1 = 0
  wgt_accessl2 = 0
  wgt_missl2 = 0
  wgt_miss_rl2 = 0
  wgt_mpkil2 = 0
  wgt_accessl3 = 0
  wgt_missl3 = 0
  wgt_miss_rl3 = 0
  wgt_mpkil3 = 0
  
  cumm_wgt_accessl1 = 0
  cumm_wgt_missl1 = 0
  cumm_wgt_miss_rl1 = 0
  cumm_wgt_mpkil1 = 0
  cumm_wgt_accessl2 = 0
  cumm_wgt_missl2 = 0
  cumm_wgt_miss_rl2 = 0
  cumm_wgt_mpkil2 = 0
  cumm_wgt_accessl3 = 0
  cumm_wgt_missl3 = 0
  cumm_wgt_miss_rl3 = 0
  cumm_wgt_mpkil3 = 0


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


  cumm_wgt_a_SRAM   = 0
  cumm_wgt_b_SRAM   = 0
  cumm_wgt_c_SRAM   = 0
  cumm_wgt_d_SRAM   = 0
  cumm_wgt_e_SRAM   = 0
  cumm_wgt_f_SRAM   = 0
  cumm_wgt_g_SRAM   = 0
  cumm_wgt_a_STTR   = 0
  cumm_wgt_b_STTR   = 0
  cumm_wgt_c_STTR   = 0
  cumm_wgt_d_STTR   = 0
  cumm_wgt_e_STTR   = 0
  cumm_wgt_f_STTR   = 0
  cumm_wgt_g_STTR   = 0

  cumm_wgt_force_Migration_SramToSTTram   = 0
  cumm_wgt_force_Migration_STTramToSram   = 0


  cumm_wgt_SRAM_reads_hits   = 0
  cumm_wgt_SRAM_write_hits   = 0
  cumm_wgt_STTR_reads_hits   = 0
  cumm_wgt_STTR_write_hits   = 0
  

  print "results directory: "+ip_dir
  
  if num_apps == '2':
    fop.write("app1\tmpki\tmisr\tcpi\t\tdeadB\t\tinsB\t\tapp2\tmpki\tmisr\tcpi\t\tdeadB\t\tinsB\tinvB\n")
  else:  
    fop.write("app1, CPI, IPC, l3_access, l3_miss, l3_miss_r, l3_mpki,l2_access, l2_miss, l2_miss_r, l2_mpki, wgt, ribc, wibc, dbc, wrte, rwte, Read_Intense_Blocks_M, Write_Intense_Blocks_M, Read_Intense_Blocks_MDuringMigrate, Write_Intense_Blocks_MDuringMigrate, M_Count, Migrate_During_Write_Count, Valid_Block_Evicted, SRAM_Read_Before_Migration, SRAM_Read_After_Migration, SRAM_Write_Before_Migration, SRAM_Write_After_Migration, STTRAM_Read_Before_Migration, STTRAM_Read_After_Migration, STTRAM_Write_Before_Migration, STTRAM_Write_After_Migration, living_SRAM_Blocks_Evicted, dead_SRAM_Blocks_Evicted, single_Migration_Count, double_Migration_Count, no_Migration_Count, g_STTr_Write, g_STTr_Reads, g_Sram_Write, g_Sram_Reads, a_SRAM, b_SRAM, c_SRAM, d_SRAM, e_SRAM, f_SRAM, g_SRAM, a_STTR, b_STTR, c_STTR, d_STTR, e_STTR, f_STTR, g_STTR, force_Migration_SramToSTTram, force_Migration_STTramToSram, SRAM_reads_hits, SRAM_write_hits, STTR_reads_hits, STTR_write_hits\n")
  

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



      a_SRAM  =  paras[34]           
      print a_SRAM
      
      b_SRAM  =  paras[35]           
      print b_SRAM
      
      c_SRAM  =  paras[36]           
      print c_SRAM
      
      d_SRAM  =  paras[37]           
      print d_SRAM
      
      e_SRAM  =  paras[38]           
      print e_SRAM
      
      f_SRAM  =  paras[39]           
      print f_SRAM
      
      g_SRAM  =  paras[40]           
      print g_SRAM
      
      a_STTR  =  paras[41]           
      print a_STTR
      
      b_STTR  =  paras[42]           
      print b_STTR
      
      c_STTR  =  paras[43]           
      print c_STTR
      
      d_STTR  =  paras[44]           
      print d_STTR
      
      e_STTR  =  paras[45]           
      print e_STTR
      
      f_STTR  =  paras[46]           
      print f_STTR
      
      g_STTR  =  paras[47]           
      print g_STTR



      force_Migration_SramToSTTram  =  paras[48]           
      print force_Migration_SramToSTTram
      
      force_Migration_STTramToSram  =  paras[49]           
      print force_Migration_STTramToSram




      SRAM_reads_hits  =  paras[50]           
      print SRAM_reads_hits
      
      SRAM_write_hits  =  paras[51]           
      print SRAM_write_hits
      
      STTR_reads_hits  =  paras[52]           
      print STTR_reads_hits
      
      STTR_write_hits  =  paras[53]           
      print STTR_write_hits






      ##################################
      ##################################

      #L1-D goes in paras 54 55 56 57
    #L2 goes in paras 58 59 60 61
    #L3 goes in paras 62 63 64 65



      accessl1 = paras[54]
      print accessl1

      missl1 = paras[55]
      print missl1

      miss_rl1 = paras[56]
      print miss_rl1

      mpkil1 = paras[57]
      print mpkil1



      accessl2 = paras[58]
      print accessl2

      missl2 = paras[59]
      print missl2

      miss_rl2 = paras[60]
      print miss_rl2

      mpkil2 = paras[61]
      print mpkil2


      accessl3 = paras[62]
      print accessl3

      missl3 = paras[63]
      print missl3

      miss_rl3 = paras[64]
      print miss_rl3

      mpkil3 = paras[65]
      print mpkil3


      ##################################

      

      
      print "================================================"
      cumm_wgt += wgt
      print "Cummulative Weights = "+str(cumm_wgt)

      wgt_cpi = cpi*wgt
      print "Weighted CPI = "+str(wgt_cpi)
      cumm_wgt_cpi += wgt_cpi
      print "Cummulative Weighted CPI = "+str(cumm_wgt_cpi)





      wgt_accessl1    = accessl1*wgt  
      print "Weighted accessl1    = "+str(wgt_accessl1)       
      cumm_wgt_accessl1   += wgt_accessl1     
      print "Cummulative Weighted accessl1    = "+str(cumm_wgt_accessl1)
      
      wgt_missl1      = missl1*wgt    
      print "Weighted missl1      = "+str(wgt_missl1)         
      cumm_wgt_missl1     += wgt_missl1       
      print "Cummulative Weighted missl1      = "+str(cumm_wgt_missl1)
      
      wgt_miss_rl1    = miss_rl1*wgt  
      print "Weighted miss_rl1    = "+str(wgt_miss_rl1)       
      cumm_wgt_miss_rl1   += wgt_miss_rl1     
      print "Cummulative Weighted miss_rl1    = "+str(cumm_wgt_miss_rl1)
      
      wgt_mpkil1      = mpkil1*wgt    
      print "Weighted mpkil1      = "+str(wgt_mpkil1)         
      cumm_wgt_mpkil1     += wgt_mpkil1       
      print "Cummulative Weighted mpkil1      = "+str(cumm_wgt_mpkil1)
      
      
      
      wgt_accessl2    = accessl2*wgt  
      print "Weighted accessl2    = "+str(wgt_accessl2)       
      cumm_wgt_accessl2   += wgt_accessl2     
      print "Cummulative Weighted accessl2    = "+str(cumm_wgt_accessl2)
      
      wgt_missl2      = missl2*wgt    
      print "Weighted missl2      = "+str(wgt_missl2)         
      cumm_wgt_missl2     += wgt_missl2       
      print "Cummulative Weighted missl2      = "+str(cumm_wgt_missl2)
      
      wgt_miss_rl2    = miss_rl2*wgt  
      print "Weighted miss_rl2    = "+str(wgt_miss_rl2)       
      cumm_wgt_miss_rl2   += wgt_miss_rl2     
      print "Cummulative Weighted miss_rl2    = "+str(cumm_wgt_miss_rl2)
      
      wgt_mpkil2      = mpkil2*wgt    
      print "Weighted mpkil2      = "+str(wgt_mpkil2)         
      cumm_wgt_mpkil2     += wgt_mpkil2       
      print "Cummulative Weighted mpkil2      = "+str(cumm_wgt_mpkil2)
      
      
      
      wgt_accessl3    = accessl3*wgt  
      print "Weighted accessl3    = "+str(wgt_accessl3)       
      cumm_wgt_accessl3   += wgt_accessl3     
      print "Cummulative Weighted accessl3    = "+str(cumm_wgt_accessl3)
      
      wgt_missl3      = missl3*wgt    
      print "Weighted missl3      = "+str(wgt_missl3)         
      cumm_wgt_missl3     += wgt_missl3       
      print "Cummulative Weighted missl3      = "+str(cumm_wgt_missl3)
      
      wgt_miss_rl3    = miss_rl3*wgt  
      print "Weighted miss_rl3    = "+str(wgt_miss_rl3)       
      cumm_wgt_miss_rl3   += wgt_miss_rl3     
      print "Cummulative Weighted miss_rl3    = "+str(cumm_wgt_miss_rl3)
      
      wgt_mpkil3      = mpkil3*wgt    
      print "Weighted mpkil3      = "+str(wgt_mpkil3)         
      cumm_wgt_mpkil3     += wgt_mpkil3       
      print "Cummulative Weighted mpkil3      = "+str(cumm_wgt_mpkil3)
 






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



      wgt_a_SRAM = wgt*a_SRAM      
      print "Weighted a SRAM   = "+str(wgt_a_SRAM)    
      cumm_wgt_a_SRAM  += wgt_a_SRAM       
      print "Cummulative Weighted a SRAM   = "+str(cumm_wgt_a_SRAM)
      
      wgt_b_SRAM = wgt*b_SRAM      
      print "Weighted b SRAM   = "+str(wgt_b_SRAM)    
      cumm_wgt_b_SRAM  += wgt_b_SRAM       
      print "Cummulative Weighted b SRAM   = "+str(cumm_wgt_b_SRAM)
      
      wgt_c_SRAM = wgt*c_SRAM      
      print "Weighted c SRAM   = "+str(wgt_c_SRAM)    
      cumm_wgt_c_SRAM  += wgt_c_SRAM       
      print "Cummulative Weighted c SRAM   = "+str(cumm_wgt_c_SRAM)
      
      wgt_d_SRAM = wgt*d_SRAM      
      print "Weighted d SRAM   = "+str(wgt_d_SRAM)    
      cumm_wgt_d_SRAM  += wgt_d_SRAM       
      print "Cummulative Weighted d SRAM   = "+str(cumm_wgt_d_SRAM)
      
      wgt_e_SRAM = wgt*e_SRAM      
      print "Weighted e SRAM   = "+str(wgt_e_SRAM)    
      cumm_wgt_e_SRAM  += wgt_e_SRAM       
      print "Cummulative Weighted e SRAM   = "+str(cumm_wgt_e_SRAM)
      
      wgt_f_SRAM = wgt*f_SRAM      
      print "Weighted f SRAM   = "+str(wgt_f_SRAM)    
      cumm_wgt_f_SRAM  += wgt_f_SRAM       
      print "Cummulative Weighted f SRAM   = "+str(cumm_wgt_f_SRAM)
      
      wgt_g_SRAM = wgt*g_SRAM      
      print "Weighted g SRAM   = "+str(wgt_g_SRAM)    
      cumm_wgt_g_SRAM  += wgt_g_SRAM       
      print "Cummulative Weighted g SRAM   = "+str(cumm_wgt_g_SRAM)
      
      wgt_a_STTR = wgt*a_STTR      
      print "Weighted a STTR   = "+str(wgt_a_STTR)    
      cumm_wgt_a_STTR  += wgt_a_STTR       
      print "Cummulative Weighted a STTR   = "+str(cumm_wgt_a_STTR)
      
      wgt_b_STTR = wgt*b_STTR      
      print "Weighted b STTR   = "+str(wgt_b_STTR)    
      cumm_wgt_b_STTR  += wgt_b_STTR       
      print "Cummulative Weighted b STTR   = "+str(cumm_wgt_b_STTR)
      
      wgt_c_STTR = wgt*c_STTR      
      print "Weighted c STTR   = "+str(wgt_c_STTR)    
      cumm_wgt_c_STTR  += wgt_c_STTR       
      print "Cummulative Weighted c STTR   = "+str(cumm_wgt_c_STTR)
      
      wgt_d_STTR = wgt*d_STTR      
      print "Weighted d STTR   = "+str(wgt_d_STTR)    
      cumm_wgt_d_STTR  += wgt_d_STTR       
      print "Cummulative Weighted d STTR   = "+str(cumm_wgt_d_STTR)
      
      wgt_e_STTR = wgt*e_STTR      
      print "Weighted e STTR   = "+str(wgt_e_STTR)    
      cumm_wgt_e_STTR  += wgt_e_STTR       
      print "Cummulative Weighted e STTR   = "+str(cumm_wgt_e_STTR)

      wgt_f_STTR = wgt*f_STTR      
      print "Weighted f STTR   = "+str(wgt_f_STTR)    
      cumm_wgt_f_STTR  += wgt_f_STTR       
      print "Cummulative Weighted f STTR   = "+str(cumm_wgt_f_STTR)
      
      wgt_g_STTR = wgt*g_STTR      
      print "Weighted g STTR   = "+str(wgt_g_STTR)    
      cumm_wgt_g_STTR  += wgt_g_STTR       
      print "Cummulative Weighted g STTR   = "+str(cumm_wgt_g_STTR)  




      wgt_force_Migration_SramToSTTram = wgt*force_Migration_SramToSTTram      
      print "Weighted force Migration SramToSTTram   = "+str(wgt_force_Migration_SramToSTTram)    
      cumm_wgt_force_Migration_SramToSTTram  += wgt_force_Migration_SramToSTTram       
      print "Cummulative Weighted force Migration SramToSTTram   = "+str(cumm_wgt_force_Migration_SramToSTTram) 
      
      wgt_force_Migration_STTramToSram = wgt*force_Migration_STTramToSram      
      print "Weighted force Migration STTramToSram   = "+str(wgt_force_Migration_STTramToSram)    
      cumm_wgt_force_Migration_STTramToSram  += wgt_force_Migration_STTramToSram       
      print "Cummulative Weighted force Migration STTramToSram   = "+str(cumm_wgt_force_Migration_STTramToSram) 




      wgt_SRAM_reads_hits = wgt*SRAM_reads_hits      
      print "Weighted SRAM reads hits   = "+str(wgt_SRAM_reads_hits)    
      cumm_wgt_SRAM_reads_hits  += wgt_SRAM_reads_hits       
      print "Cummulative Weighted SRAM reads hits   = "+str(cumm_wgt_SRAM_reads_hits)     
      
      wgt_SRAM_write_hits = wgt*SRAM_write_hits      
      print "Weighted SRAM write hits   = "+str(wgt_SRAM_write_hits)    
      cumm_wgt_SRAM_write_hits  += wgt_SRAM_write_hits       
      print "Cummulative Weighted SRAM write hits   = "+str(cumm_wgt_SRAM_write_hits)     
      
      wgt_STTR_reads_hits = wgt*STTR_reads_hits      
      print "Weighted STTR reads hits   = "+str(wgt_STTR_reads_hits)    
      cumm_wgt_STTR_reads_hits  += wgt_STTR_reads_hits       
      print "Cummulative Weighted STTR reads hits   = "+str(cumm_wgt_STTR_reads_hits)     
      
      wgt_STTR_write_hits = wgt*STTR_write_hits      
      print "Weighted STTR write hits   = "+str(wgt_STTR_write_hits)    
      cumm_wgt_STTR_write_hits  += wgt_STTR_write_hits       
      print "Cummulative Weighted STTR write hits   = "+str(cumm_wgt_STTR_write_hits)       


              




      #######################################
      #######################################







      wgt_dbc = dbc*wgt
      print "Weighted dbc = "+str(wgt_dbc)
      cumm_wgt_dbc += wgt_dbc
      print "Cummulative Weighted dbc = "+str(cumm_wgt_dbc)


      print "================================================"      


      print "CPI: "+format((float(paras[2])/float(paras[1])),'.2f')
      print "IPC: "+format((float(paras[1])/float(paras[2])),'.2f')
      
      fop.write(files+","+str(cpi)+","+str(ipc)+","+str(accessl1)+","+str(missl1)+","+str(miss_rl1)+","+str(mpkil1)+","+str(accessl2)+","+str(missl2)+","+str(miss_rl2)+","+str(mpkil2)+","+str(accessl3)+","+str(missl3)+","+str(miss_rl3)+","+str(mpkil3)+","+str(wgt)+","+str(ribc)+","+str(wibc)+","+str(dbc)+","+str(wrte)+","+str(rwte)+","+str(Read_Intense_Blocks_M)+","+str(Write_Intense_Blocks_M)+","+str(Read_Intense_Blocks_MDuringMigrate)+","+str(Write_Intense_Blocks_MDuringMigrate)+","+str(M_Count)+","+str(Migrate_During_Write_Count)+","+str(Valid_Block_Evicted)+","+str(SRAM_Read_Before_Migration)+","+str(SRAM_Read_After_Migration)+","+str(SRAM_Write_Before_Migration)+","+str(SRAM_Write_After_Migration)+","+str(STTRAM_Read_Before_Migration)+","+str(STTRAM_Read_After_Migration)+","+str(STTRAM_Write_Before_Migration)+","+str(STTRAM_Write_After_Migration)+","+str(living_SRAM_Blocks_Evicted)+","+str(dead_SRAM_Blocks_Evicted)+","+str(single_Migration_Count)+","+str(double_Migration_Count)+","+str(no_Migration_Count)+","+str(g_STTr_Write)+","+str(g_STTr_Reads)+","+str(g_Sram_Write)+","+str(g_Sram_Reads)+","+str(a_SRAM)+","+str(b_SRAM)+","+str(c_SRAM)+","+str(d_SRAM)+","+str(e_SRAM)+","+str(f_SRAM)+","+str(g_SRAM)+","+str(a_STTR)+","+str(b_STTR)+","+str(c_STTR)+","+str(d_STTR)+","+str(e_STTR)+","+str(f_STTR)+","+str(g_STTR)+","+str(force_Migration_SramToSTTram)+","+str(force_Migration_STTramToSram)+","+str(SRAM_reads_hits)+","+str(SRAM_write_hits)+","+str(STTR_reads_hits)+","+str(STTR_write_hits)+"\n")
     
      
      print "================================"

  Weighted_cpi = cumm_wgt_cpi/cumm_wgt
  Weighted_cpi = format(Weighted_cpi, '.2f')
  Weighted_ipc = cumm_wgt/cumm_wgt_cpi
  Weighted_ipc = format(Weighted_ipc, '.2f')






  Weighted_accessl1 = cumm_wgt_accessl1/cumm_wgt    
  Weighted_accessl1 = format(Weighted_accessl1, '.2f')
  
  Weighted_missl1 = cumm_wgt_missl1/cumm_wgt      
  Weighted_missl1 = format(Weighted_missl1, '.2f')
  
  Weighted_miss_rl1 = cumm_wgt_miss_rl1/cumm_wgt  
  Weighted_miss_rl1 = format(Weighted_miss_rl1, '.2f')
  
  Weighted_mpkil1 = cumm_wgt_mpkil1/cumm_wgt      
  Weighted_mpkil1 = format(Weighted_mpkil1, '.2f')
  
  
  
  Weighted_accessl2 = cumm_wgt_accessl2/cumm_wgt  
  Weighted_accessl2 = format(Weighted_accessl2, '.2f')
  
  Weighted_missl2 = cumm_wgt_missl2/cumm_wgt      
  Weighted_missl2 = format(Weighted_missl2, '.2f')
  
  Weighted_miss_rl2 = cumm_wgt_miss_rl2/cumm_wgt  
  Weighted_miss_rl2 = format(Weighted_miss_rl2, '.2f')
  
  Weighted_mpkil2 = cumm_wgt_mpkil2/cumm_wgt      
  Weighted_mpkil2 = format(Weighted_mpkil2, '.2f')
  
  
  
  Weighted_accessl3 = cumm_wgt_accessl3/cumm_wgt  
  Weighted_accessl3 = format(Weighted_accessl3, '.2f')
  
  Weighted_missl3 = cumm_wgt_missl3/cumm_wgt      
  Weighted_missl3 = format(Weighted_missl3, '.2f')
  
  Weighted_miss_rl3 = cumm_wgt_miss_rl3/cumm_wgt  
  Weighted_miss_rl3 = format(Weighted_miss_rl3, '.2f')
  
  Weighted_mpkil3 = cumm_wgt_mpkil3/cumm_wgt      
  Weighted_mpkil3 = format(Weighted_mpkil3, '.2f')





  
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



  Weighted_a_SRAM = cumm_wgt_a_SRAM/cumm_wgt        
  Weighted_a_SRAM = format(Weighted_a_SRAM, '.0f')
  
  Weighted_b_SRAM = cumm_wgt_b_SRAM/cumm_wgt        
  Weighted_b_SRAM = format(Weighted_b_SRAM, '.0f')
  
  Weighted_c_SRAM = cumm_wgt_c_SRAM/cumm_wgt        
  Weighted_c_SRAM = format(Weighted_c_SRAM, '.0f')
  
  Weighted_d_SRAM = cumm_wgt_d_SRAM/cumm_wgt        
  Weighted_d_SRAM = format(Weighted_d_SRAM, '.0f')
  
  Weighted_e_SRAM = cumm_wgt_e_SRAM/cumm_wgt        
  Weighted_e_SRAM = format(Weighted_e_SRAM, '.0f')
  
  Weighted_f_SRAM = cumm_wgt_f_SRAM/cumm_wgt        
  Weighted_f_SRAM = format(Weighted_f_SRAM, '.0f')
  
  Weighted_g_SRAM = cumm_wgt_g_SRAM/cumm_wgt        
  Weighted_g_SRAM = format(Weighted_g_SRAM, '.0f')
  
  Weighted_a_STTR = cumm_wgt_a_STTR/cumm_wgt        
  Weighted_a_STTR = format(Weighted_a_STTR, '.0f')
  
  Weighted_b_STTR = cumm_wgt_b_STTR/cumm_wgt        
  Weighted_b_STTR = format(Weighted_b_STTR, '.0f')
  
  Weighted_c_STTR = cumm_wgt_c_STTR/cumm_wgt        
  Weighted_c_STTR = format(Weighted_c_STTR, '.0f')
  
  Weighted_d_STTR = cumm_wgt_d_STTR/cumm_wgt        
  Weighted_d_STTR = format(Weighted_d_STTR, '.0f')
  
  Weighted_e_STTR = cumm_wgt_e_STTR/cumm_wgt        
  Weighted_e_STTR = format(Weighted_e_STTR, '.0f')
  
  Weighted_f_STTR = cumm_wgt_f_STTR/cumm_wgt        
  Weighted_f_STTR = format(Weighted_f_STTR, '.0f')
  
  Weighted_g_STTR = cumm_wgt_g_STTR/cumm_wgt        
  Weighted_g_STTR = format(Weighted_g_STTR, '.0f')





  Weighted_force_Migration_SramToSTTram = cumm_wgt_force_Migration_SramToSTTram/cumm_wgt        
  Weighted_force_Migration_SramToSTTram = format(Weighted_force_Migration_SramToSTTram, '.0f')
  
  Weighted_force_Migration_STTramToSram = cumm_wgt_force_Migration_STTramToSram/cumm_wgt        
  Weighted_force_Migration_STTramToSram = format(Weighted_force_Migration_STTramToSram, '.0f')




  Weighted_SRAM_reads_hits = cumm_wgt_SRAM_reads_hits/cumm_wgt        
  Weighted_SRAM_reads_hits = format(Weighted_SRAM_reads_hits, '.0f')
  
  Weighted_SRAM_write_hits = cumm_wgt_SRAM_write_hits/cumm_wgt        
  Weighted_SRAM_write_hits = format(Weighted_SRAM_write_hits, '.0f')
  
  Weighted_STTR_reads_hits = cumm_wgt_STTR_reads_hits/cumm_wgt        
  Weighted_STTR_reads_hits = format(Weighted_STTR_reads_hits, '.0f')
  
  Weighted_STTR_write_hits = cumm_wgt_STTR_write_hits/cumm_wgt        
  Weighted_STTR_write_hits = format(Weighted_STTR_write_hits, '.0f')




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
  fop.write("Benchmark, Weighted IPC, Weighted_accessl1, Weighted_missl1, Weighted_miss_rl1, Weighted_mpkil1, Weighted_accessl2, Weighted_missl2, Weighted_miss_rl2, Weighted_mpkil2, Weighted_accessl3, Weighted_missl3, Weighted_miss_rl3, Weighted_mpkil3, Weighted ribc, Weighted wibc, Weighted dbc, Weighted wrte, Weighted rwte, Weighted Read Intense Blocks M, Weighted Write Intense Blocks M, Weighted Read Intense Blocks MDuringMigrate, Weighted Write Intense Blocks MDuringMigrate, Weighted M Count, Weighted Migrate During Write Count, Weighted Valid Blocks Evicted, Weighted SRAM Read Before Migration, Weighted SRAM Read After Migration, Weighted SRAM Write Before Migration, Weighted SRAM Write After Migration, Weighted STTRAM Read Before Migration, Weighted STTRAM Read After Migration, Weighted STTRAM Write Before Migration, Weighted STTRAM Write After Migration, living SRAM Blocks Evicted, dead SRAM Blocks Evicted, single Migration Count, double Migration Count, no Migration Count, Weighted g STTr Write, Weighted g STTr Reads, Weighted g Sram Write, Weighted g Sram Reads, Weighted a SRAM, Weighted b SRAM, Weighted c SRAM, Weighted d SRAM, Weighted e SRAM, Weighted f SRAM, Weighted g SRAM, Weighted a STTR, Weighted b STTR, Weighted c STTR, Weighted d STTR, Weighted e STTR, Weighted f STTR, Weighted g STTR, Weighted force Migration SramToSTTram, Weighted force Migration STTramToSram, Weighted SRAM_reads_hits, Weighted SRAM_write_hits, Weighted STTR_reads_hits, Weighted STTR_write_hits\n")
  #fop.write(application+","+str(Weighted_cpi)+","+str(Weighted_ipc)+","+Weighted_mpki+"\n")
  fop.write(application+","+str(Weighted_ipc)+","+str(Weighted_accessl1)+","+str(Weighted_missl1)+","+str(Weighted_miss_rl1)+","+str(Weighted_mpkil1)+","+str(Weighted_accessl2)+","+str(Weighted_missl2)+","+str(Weighted_miss_rl2)+","+str(Weighted_mpkil2)+","+str(Weighted_accessl3)+","+str(Weighted_missl3)+","+str(Weighted_miss_rl3)+","+str(Weighted_mpkil3)+","+str(Weighted_ribc)+","+str(Weighted_wibc)+","+str(Weighted_dbc)+","+str(Weighted_wrte)+","+str(Weighted_rwte)+","+str(Weighted_Read_Intense_Blocks_M)+","+str(Weighted_Write_Intense_Blocks_M)+","+str(Weighted_Read_Intense_Blocks_MDuringMigrate)+","+str(Weighted_Write_Intense_Blocks_MDuringMigrate)+","+str(Weighted_M_Count)+","+str(Weighted_Migrate_During_Write_Count)+","+str(Weighted_Valid_Block_Evicted)+","+str(Weighted_SRAM_Read_Before_Migration)+","+str(Weighted_SRAM_Read_After_Migration)+","+str(Weighted_SRAM_Write_Before_Migration)+","+str(Weighted_SRAM_Write_After_Migration)+","+str(Weighted_STTRAM_Read_Before_Migration)+","+str(Weighted_STTRAM_Read_After_Migration)+","+str(Weighted_STTRAM_Write_Before_Migration)+","+str(Weighted_STTRAM_Write_After_Migration)+","+str(Weighted_living_SRAM_Blocks_Evicted)+","+str(Weighted_dead_SRAM_Blocks_Evicted)+","+str(Weighted_single_Migration_Count)+","+str(Weighted_double_Migration_Count)+","+str(Weighted_no_Migration_Count)+","+str(Weighted_g_STTr_Write)+","+str(Weighted_g_STTr_Reads)+","+str(Weighted_g_Sram_Write)+","+str(Weighted_g_Sram_Reads)+","+str(Weighted_a_SRAM)+","+str(Weighted_b_SRAM)+","+str(Weighted_c_SRAM)+","+str(Weighted_d_SRAM)+","+str(Weighted_e_SRAM)+","+str(Weighted_f_SRAM)+","+str(Weighted_g_SRAM)+","+str(Weighted_a_STTR)+","+str(Weighted_b_STTR)+","+str(Weighted_c_STTR)+","+str(Weighted_d_STTR)+","+str(Weighted_e_STTR)+","+str(Weighted_f_STTR)+","+str(Weighted_g_STTR)+","+str(Weighted_force_Migration_SramToSTTram)+","+str(Weighted_force_Migration_STTramToSram)+","+str(Weighted_SRAM_reads_hits)+","+str(Weighted_SRAM_write_hits)+","+str(Weighted_STTR_reads_hits)+","+str(Weighted_STTR_write_hits)+"\n")

  ts = time.time()
  st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
  fop.write("\n\n\n\n"+st)
           
if __name__ == "__main__":
  main()
