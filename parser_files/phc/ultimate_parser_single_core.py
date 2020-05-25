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
    
    
    
    tmp_Blocks_Read_Before_MA              =re.findall('Blocks Read Before MA[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                
    
    Blocks_Read_Before_MA              =re.findall('[\d]+',tmp_Blocks_Read_Before_MA[0])                
    
    paras.append(float(Blocks_Read_Before_MA[0]))   #14
    
    
    
    tmp_Blocks_Read_Before_MB             =re.findall('Blocks Read Before MB[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)               
    
    Blocks_Read_Before_MB             =re.findall('[\d]+',tmp_Blocks_Read_Before_MB[0])               
    
    paras.append(float(Blocks_Read_Before_MB[0]))   #15
    
    
    
    tmp_Blocks_Read_Before_MC             =re.findall('Blocks Read Before MC[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)               
    
    Blocks_Read_Before_MC             =re.findall('[\d]+',tmp_Blocks_Read_Before_MC[0])               
    
    paras.append(float(Blocks_Read_Before_MC[0]))   #16
    
    
    
    tmp_Blocks_Read_Before_MD             =re.findall('Blocks Read Before MD[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)               
    
    Blocks_Read_Before_MD             =re.findall('[\d]+',tmp_Blocks_Read_Before_MD[0])               
    
    paras.append(float(Blocks_Read_Before_MD[0]))   #17
    
    
    
    tmp_Blocks_Read_Before_ME            =re.findall('Blocks Read Before ME[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)              
    
    Blocks_Read_Before_ME            =re.findall('[\d]+',tmp_Blocks_Read_Before_ME[0])              
    
    paras.append(float(Blocks_Read_Before_ME[0]))   #18
    
    
    
    tmp_Blocks_Read_Before_MDuringWriteA    =re.findall('Blocks Read Before MDuringWriteA[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)      
    
    Blocks_Read_Before_MDuringWriteA    =re.findall('[\d]+',tmp_Blocks_Read_Before_MDuringWriteA[0])      
    
    paras.append(float(Blocks_Read_Before_MDuringWriteA[0])) #19
    
    
    
    tmp_Blocks_Read_Before_MDuringWriteB   =re.findall('Blocks Read Before MDuringWriteB[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    
    Blocks_Read_Before_MDuringWriteB   =re.findall('[\d]+',tmp_Blocks_Read_Before_MDuringWriteB[0])     
    
    paras.append(float(Blocks_Read_Before_MDuringWriteB[0]))    #20
    
    
    
    tmp_Blocks_Read_Before_MDuringWriteC   =re.findall('Blocks Read Before MDuringWriteC[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    
    Blocks_Read_Before_MDuringWriteC   =re.findall('[\d]+',tmp_Blocks_Read_Before_MDuringWriteC[0])     
    
    paras.append(float(Blocks_Read_Before_MDuringWriteC[0]))    #21
    
    
    
    tmp_Blocks_Read_Before_MDuringWriteD   =re.findall('Blocks Read Before MDuringWriteD[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    
    Blocks_Read_Before_MDuringWriteD   =re.findall('[\d]+',tmp_Blocks_Read_Before_MDuringWriteD[0])     
    
    paras.append(float(Blocks_Read_Before_MDuringWriteD[0]))#22
    
    
    
    tmp_Blocks_Read_Before_MDuringWriteE  =re.findall('Blocks Read Before MDuringWriteE[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)    
    
    Blocks_Read_Before_MDuringWriteE  =re.findall('[\d]+',tmp_Blocks_Read_Before_MDuringWriteE[0])    
    
    paras.append(float(Blocks_Read_Before_MDuringWriteE[0]))#23
    
    
    
    tmp_Blocks_Write_Before_MA              =re.findall('Blocks Write Before MA[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)              
    
    Blocks_Write_Before_MA              =re.findall('[\d]+',tmp_Blocks_Write_Before_MA[0])              
    
    paras.append(float(Blocks_Write_Before_MA[0]))#24
    
    
    
    tmp_Blocks_Write_Before_MB             =re.findall('Blocks Write Before MB[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)             
    
    Blocks_Write_Before_MB             =re.findall('[\d]+',tmp_Blocks_Write_Before_MB[0])             
    
    paras.append(float(Blocks_Write_Before_MB[0]))#25
    
    
    
    tmp_Blocks_Write_Before_MC             =re.findall('Blocks Write Before MC[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)             
    
    Blocks_Write_Before_MC             =re.findall('[\d]+',tmp_Blocks_Write_Before_MC[0])             
    
    paras.append(float(Blocks_Write_Before_MC[0]))#26
    
    
    
    tmp_Blocks_Write_Before_MD             =re.findall('Blocks Write Before MD[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)             
    
    Blocks_Write_Before_MD             =re.findall('[\d]+',tmp_Blocks_Write_Before_MD[0])             
    
    paras.append(float(Blocks_Write_Before_MD[0]))#27
    
    
    
    tmp_Blocks_Write_Before_ME            =re.findall('Blocks Write Before ME[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)            
    
    Blocks_Write_Before_ME            =re.findall('[\d]+',tmp_Blocks_Write_Before_ME[0])            
    
    paras.append(float(Blocks_Write_Before_ME[0]))#28
    
    
    
    tmp_Blocks_Write_Before_MDuringWriteA    =re.findall('Blocks Write Before MDuringWriteA[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)    
    
    Blocks_Write_Before_MDuringWriteA    =re.findall('[\d]+',tmp_Blocks_Write_Before_MDuringWriteA[0])    
    
    paras.append(float(Blocks_Write_Before_MDuringWriteA[0]))#29
    
    
    
    tmp_Blocks_Write_Before_MDuringWriteB   =re.findall('Blocks Write Before MDuringWriteB[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)   
    
    Blocks_Write_Before_MDuringWriteB   =re.findall('[\d]+',tmp_Blocks_Write_Before_MDuringWriteB[0])   
    
    paras.append(float(Blocks_Write_Before_MDuringWriteB[0]))#30
    
    
    
    tmp_Blocks_Write_Before_MDuringWriteC   =re.findall('Blocks Write Before MDuringWriteC[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)   
    
    Blocks_Write_Before_MDuringWriteC   =re.findall('[\d]+',tmp_Blocks_Write_Before_MDuringWriteC[0])   
    
    paras.append(float(Blocks_Write_Before_MDuringWriteC[0]))#31
    
    
    
    tmp_Blocks_Write_Before_MDuringWriteD   =re.findall('Blocks Write Before MDuringWriteD[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)   
    
    Blocks_Write_Before_MDuringWriteD   =re.findall('[\d]+',tmp_Blocks_Write_Before_MDuringWriteD[0])   
    
    paras.append(float(Blocks_Write_Before_MDuringWriteD[0]))#32
    
    
    
    tmp_Blocks_Write_Before_MDuringWriteE  =re.findall('Blocks Write Before MDuringWriteE[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    
    Blocks_Write_Before_MDuringWriteE  =re.findall('[\d]+',tmp_Blocks_Write_Before_MDuringWriteE[0])  
    
    paras.append(float(Blocks_Write_Before_MDuringWriteE[0]))#33
    
    
    
    tmp_Blocks_Read_After_MA              =re.findall('Blocks Read After MA[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                  
    
    Blocks_Read_After_MA              =re.findall('[\d]+',tmp_Blocks_Read_After_MA[0])                  
    
    paras.append(float(Blocks_Read_After_MA[0]))#34
    
    
    
    tmp_Blocks_Read_After_MB             =re.findall('Blocks Read After MB[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                 
    
    Blocks_Read_After_MB             =re.findall('[\d]+',tmp_Blocks_Read_After_MB[0])                 
    
    paras.append(float(Blocks_Read_After_MB[0]))#5
    
    
    
    tmp_Blocks_Read_After_MC             =re.findall('Blocks Read After MC[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                 
    
    Blocks_Read_After_MC             =re.findall('[\d]+',tmp_Blocks_Read_After_MC[0])                 
    
    paras.append(float(Blocks_Read_After_MC[0]))#36
    
    
    
    tmp_Blocks_Read_After_MD             =re.findall('Blocks Read After MD[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                 
    
    Blocks_Read_After_MD             =re.findall('[\d]+',tmp_Blocks_Read_After_MD[0])                 
    
    paras.append(float(Blocks_Read_After_MD[0]))#37
    
    
    
    tmp_Blocks_Read_After_ME            =re.findall('Blocks Read After ME[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                
    
    Blocks_Read_After_ME            =re.findall('[\d]+',tmp_Blocks_Read_After_ME[0])                
    
    paras.append(float(Blocks_Read_After_ME[0]))#38
    
    
    
    tmp_Blocks_Read_After_MDuringWriteA    =re.findall('Blocks Read After MDuringWriteA[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)        
    
    Blocks_Read_After_MDuringWriteA    =re.findall('[\d]+',tmp_Blocks_Read_After_MDuringWriteA[0])        
    
    paras.append(float(Blocks_Read_After_MDuringWriteA[0]))#39
    
    
    
    tmp_Blocks_Read_After_MDuringWriteB   =re.findall('Blocks Read After MDuringWriteB[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)       
    
    Blocks_Read_After_MDuringWriteB   =re.findall('[\d]+',tmp_Blocks_Read_After_MDuringWriteB[0])       
    
    paras.append(float(Blocks_Read_After_MDuringWriteB[0]))#40
    
    
    
    tmp_Blocks_Read_After_MDuringWriteC   =re.findall('Blocks Read After MDuringWriteC[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)       
    
    Blocks_Read_After_MDuringWriteC   =re.findall('[\d]+',tmp_Blocks_Read_After_MDuringWriteC[0])       
    
    paras.append(float(Blocks_Read_After_MDuringWriteC[0]))#41
    
    
    
    tmp_Blocks_Read_After_MDuringWriteD   =re.findall('Blocks Read After MDuringWriteD[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)       
    
    Blocks_Read_After_MDuringWriteD   =re.findall('[\d]+',tmp_Blocks_Read_After_MDuringWriteD[0])       
    
    paras.append(float(Blocks_Read_After_MDuringWriteD[0]))#42
    
    
    
    tmp_Blocks_Read_After_MDuringWriteE  =re.findall('Blocks Read After MDuringWriteE[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)      
    
    Blocks_Read_After_MDuringWriteE  =re.findall('[\d]+',tmp_Blocks_Read_After_MDuringWriteE[0])      
    
    paras.append(float(Blocks_Read_After_MDuringWriteE[0]))#43
    
    
    
    tmp_Blocks_Write_After_MA              =re.findall('Blocks Write After MA[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                
    
    Blocks_Write_After_MA              =re.findall('[\d]+',tmp_Blocks_Write_After_MA[0])                
    
    paras.append(float(Blocks_Write_After_MA[0]))#44




    tmp_Blocks_Write_After_MB             =re.findall('Blocks Write After MB[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)               
    Blocks_Write_After_MB             =re.findall('[\d]+',tmp_Blocks_Write_After_MB[0])               
    paras.append(float(Blocks_Write_After_MB[0]))#45
    
    tmp_Blocks_Write_After_MC             =re.findall('Blocks Write After MC[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)               
    Blocks_Write_After_MC             =re.findall('[\d]+',tmp_Blocks_Write_After_MC[0])               
    paras.append(float(Blocks_Write_After_MC[0]))#46
    
    tmp_Blocks_Write_After_MD             =re.findall('Blocks Write After MD[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)               
    Blocks_Write_After_MD             =re.findall('[\d]+',tmp_Blocks_Write_After_MD[0])               
    paras.append(float(Blocks_Write_After_MD[0]))#47
    
    tmp_Blocks_Write_After_ME            =re.findall('Blocks Write After ME[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)              
    Blocks_Write_After_ME            =re.findall('[\d]+',tmp_Blocks_Write_After_ME[0])              
    paras.append(float(Blocks_Write_After_ME[0]))#48
    
    tmp_Blocks_Write_After_MDuringWriteA    =re.findall('Blocks Write After MDuringWriteA[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)      
    Blocks_Write_After_MDuringWriteA    =re.findall('[\d]+',tmp_Blocks_Write_After_MDuringWriteA[0])      
    paras.append(float(Blocks_Write_After_MDuringWriteA[0]))#49
    
    tmp_Blocks_Write_After_MDuringWriteB   =re.findall('Blocks Write After MDuringWriteB[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    Blocks_Write_After_MDuringWriteB   =re.findall('[\d]+',tmp_Blocks_Write_After_MDuringWriteB[0])     
    paras.append(float(Blocks_Write_After_MDuringWriteB[0]))#50
    
    tmp_Blocks_Write_After_MDuringWriteC   =re.findall('Blocks Write After MDuringWriteC[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    Blocks_Write_After_MDuringWriteC   =re.findall('[\d]+',tmp_Blocks_Write_After_MDuringWriteC[0])     
    paras.append(float(Blocks_Write_After_MDuringWriteC[0]))#51
    
    tmp_Blocks_Write_After_MDuringWriteD   =re.findall('Blocks Write After MDuringWriteD[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)     
    Blocks_Write_After_MDuringWriteD   =re.findall('[\d]+',tmp_Blocks_Write_After_MDuringWriteD[0])     
    paras.append(float(Blocks_Write_After_MDuringWriteD[0]))#52
    
    tmp_Blocks_Write_After_MDuringWriteE  =re.findall('Blocks Write After MDuringWriteE[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)    
    Blocks_Write_After_MDuringWriteE  =re.findall('[\d]+',tmp_Blocks_Write_After_MDuringWriteE[0])    
    paras.append(float(Blocks_Write_After_MDuringWriteE[0]))#53
    
    tmp_M_Count                              =re.findall('M Count[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)                                
    M_Count                              =re.findall('[\d]+',tmp_M_Count[0])                                
    paras.append(float(M_Count[0]))#54
    
    tmp_Migrate_During_Write_Count            =re.findall('Migrate During Write Count[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)              
    Migrate_During_Write_Count            =re.findall('[\d]+',tmp_Migrate_During_Write_Count[0])              
    paras.append(float(Migrate_During_Write_Count[0]))  #55

    tmp_Valid_Block_Evicted            =re.findall('Valid Blocks Evicted[\s]+[|]+[\s]+[\d]+[\s|\d]+',str)              
    Valid_Block_Evicted            =re.findall('[\d]+',tmp_Valid_Block_Evicted[0])              
    paras.append(float(Valid_Block_Evicted[0]))  #56



    tmp_SRAM_Read_Before_Migration    =re.findall('SRAM Read Before Migration    [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    SRAM_Read_Before_Migration    =re.findall('[\d]+',tmp_SRAM_Read_Before_Migration[0])    
    paras.append(float(SRAM_Read_Before_Migration[0]))      #57
    
    tmp_SRAM_Read_After_Migration     =re.findall('SRAM Read After Migration     [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    SRAM_Read_After_Migration     =re.findall('[\d]+',tmp_SRAM_Read_After_Migration[0])     
    paras.append(float(SRAM_Read_After_Migration[0]))       #58
    
    tmp_SRAM_Write_Before_Migration   =re.findall('SRAM Write Before Migration   [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    SRAM_Write_Before_Migration   =re.findall('[\d]+',tmp_SRAM_Write_Before_Migration[0])   
    paras.append(float(SRAM_Write_Before_Migration[0]))     #59
    
    tmp_SRAM_Write_After_Migration    =re.findall('SRAM Write After Migration    [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    SRAM_Write_After_Migration    =re.findall('[\d]+',tmp_SRAM_Write_After_Migration[0])    
    paras.append(float(SRAM_Write_After_Migration[0]))      #60
    
    tmp_STTRAM_Read_Before_Migration  =re.findall('STTRAM Read Before Migration  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    STTRAM_Read_Before_Migration  =re.findall('[\d]+',tmp_STTRAM_Read_Before_Migration[0])  
    paras.append(float(STTRAM_Read_Before_Migration[0]))    #61
    
    tmp_STTRAM_Read_After_Migration   =re.findall('STTRAM Read After Migration   [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    STTRAM_Read_After_Migration   =re.findall('[\d]+',tmp_STTRAM_Read_After_Migration[0])   
    paras.append(float(STTRAM_Read_After_Migration[0]))     #62
    
    tmp_STTRAM_Write_Before_Migration =re.findall('STTRAM Write Before Migration [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    STTRAM_Write_Before_Migration =re.findall('[\d]+',tmp_STTRAM_Write_Before_Migration[0]) 
    paras.append(float(STTRAM_Write_Before_Migration[0]))   #63
    
    tmp_STTRAM_Write_After_Migration  =re.findall('STTRAM Write After Migration  [\s]+[|]+[\s]+[\d]+[\s|\d]+',str)  
    STTRAM_Write_After_Migration  =re.findall('[\d]+',tmp_STTRAM_Write_After_Migration[0])  
    paras.append(float(STTRAM_Write_After_Migration[0]))    #64






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
  cumm_wgt_Blocks_Read_Before_MA                = 0
  cumm_wgt_Blocks_Read_Before_MB               = 0
  cumm_wgt_Blocks_Read_Before_MC               = 0
  cumm_wgt_Blocks_Read_Before_MD               = 0
  cumm_wgt_Blocks_Read_Before_ME              = 0
  cumm_wgt_Blocks_Read_Before_MDuringWriteA      = 0
  cumm_wgt_Blocks_Read_Before_MDuringWriteB     = 0
  cumm_wgt_Blocks_Read_Before_MDuringWriteC     = 0
  cumm_wgt_Blocks_Read_Before_MDuringWriteD     = 0
  cumm_wgt_Blocks_Read_Before_MDuringWriteE    = 0
  cumm_wgt_Blocks_Write_Before_MA               = 0
  cumm_wgt_Blocks_Write_Before_MB              = 0
  cumm_wgt_Blocks_Write_Before_MC              = 0
  cumm_wgt_Blocks_Write_Before_MD              = 0
  cumm_wgt_Blocks_Write_Before_ME             = 0
  cumm_wgt_Blocks_Write_Before_MDuringWriteA     = 0
  cumm_wgt_Blocks_Write_Before_MDuringWriteB    = 0
  cumm_wgt_Blocks_Write_Before_MDuringWriteC    = 0
  cumm_wgt_Blocks_Write_Before_MDuringWriteD    = 0
  cumm_wgt_Blocks_Write_Before_MDuringWriteE   = 0
  cumm_wgt_Blocks_Read_After_MA                 = 0
  cumm_wgt_Blocks_Read_After_MB                = 0
  cumm_wgt_Blocks_Read_After_MC                = 0
  cumm_wgt_Blocks_Read_After_MD                = 0
  cumm_wgt_Blocks_Read_After_ME               = 0
  cumm_wgt_Blocks_Read_After_MDuringWriteA       = 0
  cumm_wgt_Blocks_Read_After_MDuringWriteB      = 0
  cumm_wgt_Blocks_Read_After_MDuringWriteC      = 0
  cumm_wgt_Blocks_Read_After_MDuringWriteD      = 0
  cumm_wgt_Blocks_Read_After_MDuringWriteE     = 0
  cumm_wgt_Blocks_Write_After_MA                = 0
  cumm_wgt_Blocks_Write_After_MB               = 0
  cumm_wgt_Blocks_Write_After_MC               = 0
  cumm_wgt_Blocks_Write_After_MD               = 0
  cumm_wgt_Blocks_Write_After_ME              = 0
  cumm_wgt_Blocks_Write_After_MDuringWriteA      = 0
  cumm_wgt_Blocks_Write_After_MDuringWriteB     = 0
  cumm_wgt_Blocks_Write_After_MDuringWriteC     = 0
  cumm_wgt_Blocks_Write_After_MDuringWriteD     = 0
  cumm_wgt_Blocks_Write_After_MDuringWriteE    = 0
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
  

  print "results directory: "+ip_dir
  
  if num_apps == '2':
    fop.write("app1\tmpki\tmisr\tcpi\t\tdeadB\t\tinsB\t\tapp2\tmpki\tmisr\tcpi\t\tdeadB\t\tinsB\tinvB\n")
  else:  
    fop.write("app1, CPI, IPC, l3_access, l3_miss, l3_miss_r, l3_mpki, wgt, ribc, wibc, dbc, wrte, rwte, Read_Intense_Blocks_M, Write_Intense_Blocks_M, Read_Intense_Blocks_MDuringMigrate, Write_Intense_Blocks_MDuringMigrate, Blocks_Read_Before_MA, Blocks_Read_Before_MB, Blocks_Read_Before_MC, Blocks_Read_Before_MD, Blocks_Read_Before_ME, Blocks_Read_Before_MDuringWriteA, Blocks_Read_Before_MDuringWriteB, Blocks_Read_Before_MDuringWriteC, Blocks_Read_Before_MDuringWriteD, Blocks_Read_Before_MDuringWriteE, Blocks_Write_Before_MA, Blocks_Write_Before_MB, Blocks_Write_Before_MC, Blocks_Write_Before_MD, Blocks_Write_Before_ME, Blocks_Write_Before_MDuringWriteA, Blocks_Write_Before_MDuringWriteB, Blocks_Write_Before_MDuringWriteC, Blocks_Write_Before_MDuringWriteD, Blocks_Write_Before_MDuringWriteE, Blocks_Read_After_MA, Blocks_Read_After_MB, Blocks_Read_After_MC, Blocks_Read_After_MD, Blocks_Read_After_ME, Blocks_Read_After_MDuringWriteA, Blocks_Read_After_MDuringWriteB, Blocks_Read_After_MDuringWriteC, Blocks_Read_After_MDuringWriteD, Blocks_Read_After_MDuringWriteE, Blocks_Write_After_MA, Blocks_Write_After_MB, Blocks_Write_After_MC, Blocks_Write_After_MD, Blocks_Write_After_ME, Blocks_Write_After_MDuringWriteA, Blocks_Write_After_MDuringWriteB, Blocks_Write_After_MDuringWriteC, Blocks_Write_After_MDuringWriteD, Blocks_Write_After_MDuringWriteE, M_Count, Migrate_During_Write_Count, Valid_Block_Evicted, SRAM_Read_Before_Migration, SRAM_Read_After_Migration, SRAM_Write_Before_Migration, SRAM_Write_After_Migration, STTRAM_Read_Before_Migration, STTRAM_Read_After_Migration, STTRAM_Write_Before_Migration, STTRAM_Write_After_Migration\n")
  

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



      Blocks_Read_Before_MA                  = paras[14]

      print   Blocks_Read_Before_MA



      Blocks_Read_Before_MB                 = paras[15]

      print   Blocks_Read_Before_MB



      Blocks_Read_Before_MC                 = paras[16]

      print   Blocks_Read_Before_MC



      Blocks_Read_Before_MD                 = paras[17]

      print   Blocks_Read_Before_MD



      Blocks_Read_Before_ME                = paras[18]

      print   Blocks_Read_Before_ME



      Blocks_Read_Before_MDuringWriteA        = paras[19]

      print   Blocks_Read_Before_MDuringWriteA



      Blocks_Read_Before_MDuringWriteB       = paras[20]

      print   Blocks_Read_Before_MDuringWriteB



      Blocks_Read_Before_MDuringWriteC       = paras[21]

      print   Blocks_Read_Before_MDuringWriteC



      Blocks_Read_Before_MDuringWriteD       = paras[22]

      print   Blocks_Read_Before_MDuringWriteD



      Blocks_Read_Before_MDuringWriteE      = paras[23]

      print   Blocks_Read_Before_MDuringWriteE



      Blocks_Write_Before_MA                 = paras[24]

      print   Blocks_Write_Before_MA



      Blocks_Write_Before_MB                = paras[25]

      print   Blocks_Write_Before_MB



      Blocks_Write_Before_MC                = paras[26]

      print   Blocks_Write_Before_MC



      Blocks_Write_Before_MD                = paras[27]

      print   Blocks_Write_Before_MD



      Blocks_Write_Before_ME               = paras[28]

      print   Blocks_Write_Before_ME



      Blocks_Write_Before_MDuringWriteA       = paras[29]

      print   Blocks_Write_Before_MDuringWriteA



      Blocks_Write_Before_MDuringWriteB      = paras[30]

      print   Blocks_Write_Before_MDuringWriteB



      Blocks_Write_Before_MDuringWriteC      = paras[31]

      print   Blocks_Write_Before_MDuringWriteC



      Blocks_Write_Before_MDuringWriteD      = paras[32]

      print   Blocks_Write_Before_MDuringWriteD



      Blocks_Write_Before_MDuringWriteE     = paras[33]

      print   Blocks_Write_Before_MDuringWriteE



      Blocks_Read_After_MA                   = paras[34]

      print   Blocks_Read_After_MA



      Blocks_Read_After_MB                  = paras[35]

      print   Blocks_Read_After_MB



      Blocks_Read_After_MC                  = paras[36]

      print   Blocks_Read_After_MC



      Blocks_Read_After_MD                  = paras[37]

      print   Blocks_Read_After_MD



      Blocks_Read_After_ME                 = paras[38]

      print   Blocks_Read_After_ME



      Blocks_Read_After_MDuringWriteA         = paras[39]

      print   Blocks_Read_After_MDuringWriteA



      Blocks_Read_After_MDuringWriteB        = paras[40]

      print   Blocks_Read_After_MDuringWriteB



      Blocks_Read_After_MDuringWriteC        = paras[41]

      print   Blocks_Read_After_MDuringWriteC



      Blocks_Read_After_MDuringWriteD        = paras[42]

      print   Blocks_Read_After_MDuringWriteD



      Blocks_Read_After_MDuringWriteE       = paras[43]

      print   Blocks_Read_After_MDuringWriteE



      Blocks_Write_After_MA                  = paras[44]

      print   Blocks_Write_After_MA



      Blocks_Write_After_MB                 = paras[45]

      print   Blocks_Write_After_MB



      Blocks_Write_After_MC                 = paras[46]

      print   Blocks_Write_After_MC



      Blocks_Write_After_MD                 = paras[47]

      print   Blocks_Write_After_MD



      Blocks_Write_After_ME                = paras[48]

      print   Blocks_Write_After_ME



      Blocks_Write_After_MDuringWriteA        = paras[49]

      print   Blocks_Write_After_MDuringWriteA


      Blocks_Write_After_MDuringWriteB       = paras[50]

      print   Blocks_Write_After_MDuringWriteB     



      Blocks_Write_After_MDuringWriteC       = paras[51]

      print   Blocks_Write_After_MDuringWriteC



      Blocks_Write_After_MDuringWriteD       = paras[52]

      print   Blocks_Write_After_MDuringWriteD



      Blocks_Write_After_MDuringWriteE      = paras[53]

      print   Blocks_Write_After_MDuringWriteE



      M_Count                                  = paras[54]

      print   M_Count



      Migrate_During_Write_Count                = paras[55]

      print   Migrate_During_Write_Count



      Valid_Block_Evicted                = paras[56]

      print   Valid_Block_Evicted


      SRAM_Read_Before_Migration      = paras[57]   
      print  SRAM_Read_Before_Migration    
      
      SRAM_Read_After_Migration       = paras[58] 
      print  SRAM_Read_After_Migration     
      
      SRAM_Write_Before_Migration     = paras[59] 
      print  SRAM_Write_Before_Migration   
      
      SRAM_Write_After_Migration      = paras[60] 
      print  SRAM_Write_After_Migration    
      
      STTRAM_Read_Before_Migration    = paras[61] 
      print  STTRAM_Read_Before_Migration  
      
      STTRAM_Read_After_Migration     = paras[62] 
      print  STTRAM_Read_After_Migration   
      
      STTRAM_Write_Before_Migration   = paras[63] 
      print  STTRAM_Write_Before_Migration 
      
      STTRAM_Write_After_Migration    = paras[64] 
      print  STTRAM_Write_After_Migration  






      ##################################
      ##################################



      access = paras[65]
      print access

      miss = paras[66]
      print miss

      miss_r = paras[67]
      print miss_r

      mpki = paras[68]
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

      

      wgt_Blocks_Read_Before_MA  = wgt*Blocks_Read_Before_MA                           

      print   "Weighted Blocks Read Before MA = "+str(wgt_Blocks_Read_Before_MA)                             

      cumm_wgt_Blocks_Read_Before_MA += wgt_Blocks_Read_Before_MA                                      

      print "Cummulative Weighted Blocks Read Before MA = "+str(cumm_wgt_Blocks_Read_Before_MA)                

      

      wgt_Blocks_Read_Before_MB = wgt*Blocks_Read_Before_MB                          

      print   "Weighted Blocks Read Before MB  = "+str(wgt_Blocks_Read_Before_MB)                          

      cumm_wgt_Blocks_Read_Before_MB += wgt_Blocks_Read_Before_MB                                    

      print "Cummulative Weighted Blocks Read Before MB = "+str(cumm_wgt_Blocks_Read_Before_MB)                

      

      wgt_Blocks_Read_Before_MC = wgt*Blocks_Read_Before_MC                          

      print   "Weighted Blocks Read Before MC  = "+str(wgt_Blocks_Read_Before_MC)                          

      cumm_wgt_Blocks_Read_Before_MC += wgt_Blocks_Read_Before_MC                                    

      print "Cummulative Weighted Blocks Read Before MC = "+str(cumm_wgt_Blocks_Read_Before_MC)                

      

      wgt_Blocks_Read_Before_MD = wgt*Blocks_Read_Before_MD                          

      print   "Weighted Blocks Read Before MD  = "+str(wgt_Blocks_Read_Before_MD)                          

      cumm_wgt_Blocks_Read_Before_MD += wgt_Blocks_Read_Before_MD                                    

      print "Cummulative Weighted Blocks Read Before MD = "+str(cumm_wgt_Blocks_Read_Before_MD)                

      

      wgt_Blocks_Read_Before_ME  = wgt*Blocks_Read_Before_ME                       

      print   "Weighted Blocks Read Before ME = "+str(wgt_Blocks_Read_Before_ME)                         

      cumm_wgt_Blocks_Read_Before_ME += wgt_Blocks_Read_Before_ME                                  

      print "Cummulative Weighted Blocks Read Before ME = "+str(cumm_wgt_Blocks_Read_Before_ME)                

      

      wgt_Blocks_Read_Before_MDuringWriteA  = wgt*Blocks_Read_Before_MDuringWriteA       

      print   "Weighted Blocks Read Before MDuringWriteA = "+str(wgt_Blocks_Read_Before_MDuringWriteA)         

      cumm_wgt_Blocks_Read_Before_MDuringWriteA += wgt_Blocks_Read_Before_MDuringWriteA                  

      print "Cummulative Weighted Blocks Read Before MDuringWriteA = "+str(cumm_wgt_Blocks_Read_Before_MDuringWriteA)                

      

      wgt_Blocks_Read_Before_MDuringWriteB = wgt*Blocks_Read_Before_MDuringWriteB      

      print   "Weighted Blocks Read Before MDuringWriteB  = "+str(wgt_Blocks_Read_Before_MDuringWriteB)      

      cumm_wgt_Blocks_Read_Before_MDuringWriteB += wgt_Blocks_Read_Before_MDuringWriteB                

      print "Cummulative Weighted Blocks Read Before MDuringWriteB = "+str(cumm_wgt_Blocks_Read_Before_MDuringWriteB)                

      

      wgt_Blocks_Read_Before_MDuringWriteC = wgt*Blocks_Read_Before_MDuringWriteC      

      print   "Weighted Blocks Read Before MDuringWriteC  = "+str(wgt_Blocks_Read_Before_MDuringWriteC)      

      cumm_wgt_Blocks_Read_Before_MDuringWriteC += wgt_Blocks_Read_Before_MDuringWriteC                

      print "Cummulative Weighted Blocks Read Before MDuringWriteC = "+str(cumm_wgt_Blocks_Read_Before_MDuringWriteC)                

      

      wgt_Blocks_Read_Before_MDuringWriteD = wgt*Blocks_Read_Before_MDuringWriteD      

      print   "Weighted Blocks Read Before MDuringWriteD  = "+str(wgt_Blocks_Read_Before_MDuringWriteD)      

      cumm_wgt_Blocks_Read_Before_MDuringWriteD += wgt_Blocks_Read_Before_MDuringWriteD                

      print "Cummulative Weighted Blocks Read Before MDuringWriteD = "+str(cumm_wgt_Blocks_Read_Before_MDuringWriteD)                

      

      wgt_Blocks_Read_Before_MDuringWriteE  = wgt*Blocks_Read_Before_MDuringWriteE   

      print   "Weighted Blocks Read Before MDuringWriteE = "+str(wgt_Blocks_Read_Before_MDuringWriteE)     

      cumm_wgt_Blocks_Read_Before_MDuringWriteE += wgt_Blocks_Read_Before_MDuringWriteE              

      print "Cummulative Weighted Blocks Read Before MDuringWriteE = "+str(cumm_wgt_Blocks_Read_Before_MDuringWriteE)                

      

      wgt_Blocks_Write_Before_MA = wgt*Blocks_Write_Before_MA                          

      print   "Weighted Blocks Write Before MA  = "+str(wgt_Blocks_Write_Before_MA)                          

      cumm_wgt_Blocks_Write_Before_MA += wgt_Blocks_Write_Before_MA                                    

      print "Cummulative Weighted Blocks Write Before MA = "+str(cumm_wgt_Blocks_Write_Before_MA)                

      

      wgt_Blocks_Write_Before_MB  = wgt*Blocks_Write_Before_MB                       

      print   "Weighted Blocks Write Before MB = "+str(wgt_Blocks_Write_Before_MB)                         

      cumm_wgt_Blocks_Write_Before_MB += wgt_Blocks_Write_Before_MB                                  

      print "Cummulative Weighted Blocks Write Before MB = "+str(cumm_wgt_Blocks_Write_Before_MB)                

      

      wgt_Blocks_Write_Before_MC  = wgt*Blocks_Write_Before_MC                       

      print   "Weighted Blocks Write Before MC = "+str(wgt_Blocks_Write_Before_MC)                         

      cumm_wgt_Blocks_Write_Before_MC += wgt_Blocks_Write_Before_MC                                  

      print "Cummulative Weighted Blocks Write Before MC = "+str(cumm_wgt_Blocks_Write_Before_MC)                

      

      wgt_Blocks_Write_Before_MD  = wgt*Blocks_Write_Before_MD                       

      print   "Weighted Blocks Write Before MD = "+str(wgt_Blocks_Write_Before_MD)                         

      cumm_wgt_Blocks_Write_Before_MD += wgt_Blocks_Write_Before_MD                                  

      print "Cummulative Weighted Blocks Write Before MD = "+str(cumm_wgt_Blocks_Write_Before_MD)                

      

      wgt_Blocks_Write_Before_ME = wgt*Blocks_Write_Before_ME                      

      print   "Weighted Blocks Write Before ME  = "+str(wgt_Blocks_Write_Before_ME)                      

      cumm_wgt_Blocks_Write_Before_ME += wgt_Blocks_Write_Before_ME                                

      print "Cummulative Weighted Blocks Write Before ME = "+str(cumm_wgt_Blocks_Write_Before_ME)                 

      

      wgt_Blocks_Write_Before_MDuringWriteA = wgt*Blocks_Write_Before_MDuringWriteA      

      print   "Weighted Blocks Write Before MDuringWriteA  = "+str(wgt_Blocks_Write_Before_MDuringWriteA)      

      cumm_wgt_Blocks_Write_Before_MDuringWriteA += wgt_Blocks_Write_Before_MDuringWriteA                

      print "Cummulative Weighted Blocks Write Before MDuringWriteA = "+str(cumm_wgt_Blocks_Write_Before_MDuringWriteA)                

      

      wgt_Blocks_Write_Before_MDuringWriteB  = wgt*Blocks_Write_Before_MDuringWriteB   

      print   "Weighted Blocks Write Before MDuringWriteB = "+str(wgt_Blocks_Write_Before_MDuringWriteB)     

      cumm_wgt_Blocks_Write_Before_MDuringWriteB += wgt_Blocks_Write_Before_MDuringWriteB              

      print "Cummulative Weighted Blocks Write Before MDuringWriteB = "+str(cumm_wgt_Blocks_Write_Before_MDuringWriteB)                



      wgt_Blocks_Write_Before_MDuringWriteC  = wgt*Blocks_Write_Before_MDuringWriteC   

      print   "Weighted Blocks Write Before MDuringWriteC = "+str(wgt_Blocks_Write_Before_MDuringWriteC)     

      cumm_wgt_Blocks_Write_Before_MDuringWriteC += wgt_Blocks_Write_Before_MDuringWriteC              

      print "Cummulative Weighted Blocks Write Before MDuringWriteC = "+str(cumm_wgt_Blocks_Write_Before_MDuringWriteC)                

      

      wgt_Blocks_Write_Before_MDuringWriteD  = wgt*Blocks_Write_Before_MDuringWriteD   

      print   "Weighted Blocks Write Before MDuringWriteD = "+str(wgt_Blocks_Write_Before_MDuringWriteD)     

      cumm_wgt_Blocks_Write_Before_MDuringWriteD += wgt_Blocks_Write_Before_MDuringWriteD              

      print "Cummulative Weighted Blocks Write Before MDuringWriteD = "+str(cumm_wgt_Blocks_Write_Before_MDuringWriteD)                

      

      wgt_Blocks_Write_Before_MDuringWriteE = wgt*Blocks_Write_Before_MDuringWriteE  

      print   "Weighted Blocks Write Before MDuringWriteE  = "+str(wgt_Blocks_Write_Before_MDuringWriteE)  

      cumm_wgt_Blocks_Write_Before_MDuringWriteE += wgt_Blocks_Write_Before_MDuringWriteE            

      print "Cummulative Weighted Blocks Write Before MDuringWriteE = "+str(cumm_wgt_Blocks_Write_Before_MDuringWriteE)                

      

      wgt_Blocks_Read_After_MA = wgt*Blocks_Read_After_MA                              

      print   "Weighted Blocks Read After MA  = "+str(wgt_Blocks_Read_After_MA)                              

      cumm_wgt_Blocks_Read_After_MA += wgt_Blocks_Read_After_MA                                        

      print "Cummulative Weighted Blocks Read After MA = "+str(cumm_wgt_Blocks_Read_After_MA)                

      

      wgt_Blocks_Read_After_MB  = wgt*Blocks_Read_After_MB                           

      print   "Weighted Blocks Read After MB = "+str(wgt_Blocks_Read_After_MB)                             

      cumm_wgt_Blocks_Read_After_MB += wgt_Blocks_Read_After_MB                                      

      print "Cummulative Weighted Blocks Read After MB = "+str(cumm_wgt_Blocks_Read_After_MB)                

      

      wgt_Blocks_Read_After_MC  = wgt*Blocks_Read_After_MC                           

      print   "Weighted Blocks Read After MC = "+str(wgt_Blocks_Read_After_MC)                             

      cumm_wgt_Blocks_Read_After_MC += wgt_Blocks_Read_After_MC                                      

      print "Cummulative Weighted Blocks Read After MC = "+str(cumm_wgt_Blocks_Read_After_MC)                

      

      wgt_Blocks_Read_After_MD  = wgt*Blocks_Read_After_MD                           

      print   "Weighted Blocks Read After MD = "+str(wgt_Blocks_Read_After_MD)                             

      cumm_wgt_Blocks_Read_After_MD += wgt_Blocks_Read_After_MD                                      

      print "Cummulative Weighted Blocks Read After MD = "+str(cumm_wgt_Blocks_Read_After_MD)                

      

      wgt_Blocks_Read_After_ME = wgt*Blocks_Read_After_ME                          

      print   "Weighted Blocks Read After ME  = "+str(wgt_Blocks_Read_After_ME)                          

      cumm_wgt_Blocks_Read_After_ME += wgt_Blocks_Read_After_ME                                    

      print "Cummulative Weighted Blocks Read After ME = "+str(cumm_wgt_Blocks_Read_After_ME)                

      

      wgt_Blocks_Read_After_MDuringWriteA = wgt*Blocks_Read_After_MDuringWriteA          

      print   "Weighted Blocks Read After MDuringWriteA  = "+str(wgt_Blocks_Read_After_MDuringWriteA)          

      cumm_wgt_Blocks_Read_After_MDuringWriteA += wgt_Blocks_Read_After_MDuringWriteA                    

      print "Cummulative Weighted Blocks Read After MDuringWriteA = "+str(cumm_wgt_Blocks_Read_After_MDuringWriteA)                

      

      wgt_Blocks_Read_After_MDuringWriteB  = wgt*Blocks_Read_After_MDuringWriteB       

      print   "Weighted Blocks Read After MDuringWriteB = "+str(wgt_Blocks_Read_After_MDuringWriteB)         

      cumm_wgt_Blocks_Read_After_MDuringWriteB += wgt_Blocks_Read_After_MDuringWriteB                  

      print "Cummulative Weighted Blocks Read After MDuringWriteB = "+str(cumm_wgt_Blocks_Read_After_MDuringWriteB)                

      

      wgt_Blocks_Read_After_MDuringWriteC  = wgt*Blocks_Read_After_MDuringWriteC       

      print   "Weighted Blocks Read After MDuringWriteC = "+str(wgt_Blocks_Read_After_MDuringWriteC)         

      cumm_wgt_Blocks_Read_After_MDuringWriteC += wgt_Blocks_Read_After_MDuringWriteC                  

      print "Cummulative Weighted Blocks Read After MDuringWriteC = "+str(cumm_wgt_Blocks_Read_After_MDuringWriteC)                

      

      wgt_Blocks_Read_After_MDuringWriteD  = wgt*Blocks_Read_After_MDuringWriteD       

      print   "Weighted Blocks Read After MDuringWriteD = "+str(wgt_Blocks_Read_After_MDuringWriteD)         

      cumm_wgt_Blocks_Read_After_MDuringWriteD += wgt_Blocks_Read_After_MDuringWriteD                  

      print "Cummulative Weighted Blocks Read After MDuringWriteD = "+str(cumm_wgt_Blocks_Read_After_MDuringWriteD)                

      

      wgt_Blocks_Read_After_MDuringWriteE = wgt*Blocks_Read_After_MDuringWriteE      

      print   "Weighted Blocks Read After MDuringWriteE  = "+str(wgt_Blocks_Read_After_MDuringWriteE)      

      cumm_wgt_Blocks_Read_After_MDuringWriteE += wgt_Blocks_Read_After_MDuringWriteE                

      print "Cummulative Weighted Blocks Read After MDuringWriteE = "+str(cumm_wgt_Blocks_Read_After_MDuringWriteE)                

      

      wgt_Blocks_Write_After_MA  = wgt*Blocks_Write_After_MA                           

      print   "Weighted Blocks Write After MA = "+str(wgt_Blocks_Write_After_MA)                             

      cumm_wgt_Blocks_Write_After_MA += wgt_Blocks_Write_After_MA                                      

      print "Cummulative Weighted Blocks Write After MA = "+str(cumm_wgt_Blocks_Write_After_MA)                



      wgt_Blocks_Write_After_MB = wgt*Blocks_Write_After_MB                          

      print   "Weighted Blocks Write After MB  = "+str(wgt_Blocks_Write_After_MB)                          

      cumm_wgt_Blocks_Write_After_MB += wgt_Blocks_Write_After_MB                                    

      print "Cummulative Weighted Blocks Write After MB = "+str(cumm_wgt_Blocks_Write_After_MB)                

      

      wgt_Blocks_Write_After_MC = wgt*Blocks_Write_After_MC                          

      print   "Weighted Blocks Write After MC  = "+str(wgt_Blocks_Write_After_MC)                          

      cumm_wgt_Blocks_Write_After_MC += wgt_Blocks_Write_After_MC                                    

      print "Cummulative Weighted Blocks Write After MC = "+str(cumm_wgt_Blocks_Write_After_MC)                

      

      wgt_Blocks_Write_After_MD = wgt*Blocks_Write_After_MD                          

      print   "Weighted Blocks Write After MD  = "+str(wgt_Blocks_Write_After_MD)                          

      cumm_wgt_Blocks_Write_After_MD += wgt_Blocks_Write_After_MD                                    

      print "Cummulative Weighted Blocks Write After MD = "+str(cumm_wgt_Blocks_Write_After_MD)                

      

      wgt_Blocks_Write_After_ME  = wgt*Blocks_Write_After_ME                       

      print   "Weighted Blocks Write After ME = "+str(wgt_Blocks_Write_After_ME)                         

      cumm_wgt_Blocks_Write_After_ME += wgt_Blocks_Write_After_ME                                  

      print "Cummulative Weighted Blocks Write After ME = "+str(cumm_wgt_Blocks_Write_After_ME)                

      

      wgt_Blocks_Write_After_MDuringWriteA  = wgt*Blocks_Write_After_MDuringWriteA       

      print   "Weighted Blocks Write After MDuringWriteA = "+str(wgt_Blocks_Write_After_MDuringWriteA)         

      cumm_wgt_Blocks_Write_After_MDuringWriteA += wgt_Blocks_Write_After_MDuringWriteA                  

      print "Cummulative Weighted Blocks Write After MDuringWriteA = "+str(cumm_wgt_Blocks_Write_After_MDuringWriteA)                

      

      wgt_Blocks_Write_After_MDuringWriteB = wgt*Blocks_Write_After_MDuringWriteB      

      print   "Weighted Blocks Write After MDuringWriteB  = "+str(wgt_Blocks_Write_After_MDuringWriteB)      

      cumm_wgt_Blocks_Write_After_MDuringWriteB += wgt_Blocks_Write_After_MDuringWriteB                

      print "Cummulative Weighted Blocks Write After MDuringWriteB = "+str(cumm_wgt_Blocks_Write_After_MDuringWriteB)                

      

      wgt_Blocks_Write_After_MDuringWriteC = wgt*Blocks_Write_After_MDuringWriteC      

      print   "Weighted Blocks Write After MDuringWriteC  = "+str(wgt_Blocks_Write_After_MDuringWriteC)      

      cumm_wgt_Blocks_Write_After_MDuringWriteC += wgt_Blocks_Write_After_MDuringWriteC                

      print "Cummulative Weighted Blocks Write After MDuringWriteC = "+str(cumm_wgt_Blocks_Write_After_MDuringWriteC)                

      

      wgt_Blocks_Write_After_MDuringWriteD = wgt*Blocks_Write_After_MDuringWriteD      

      print   "Weighted Blocks Write After MDuringWriteD  = "+str(wgt_Blocks_Write_After_MDuringWriteD)      

      cumm_wgt_Blocks_Write_After_MDuringWriteD += wgt_Blocks_Write_After_MDuringWriteD                

      print "Cummulative Weighted Blocks Write After MDuringWriteD = "+str(cumm_wgt_Blocks_Write_After_MDuringWriteD)                

      

      wgt_Blocks_Write_After_MDuringWriteE  = wgt*Blocks_Write_After_MDuringWriteE   

      print   "Weighted Blocks Write After MDuringWriteE = "+str(wgt_Blocks_Write_After_MDuringWriteE)     

      cumm_wgt_Blocks_Write_After_MDuringWriteE += wgt_Blocks_Write_After_MDuringWriteE              

      print "Cummulative Weighted Blocks Write After MDuringWriteE = "+str(cumm_wgt_Blocks_Write_After_MDuringWriteE)                

      

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
              




      #######################################
      #######################################







      wgt_dbc = dbc*wgt
      print "Weighted dbc = "+str(wgt_dbc)
      cumm_wgt_dbc += wgt_dbc
      print "Cummulative Weighted dbc = "+str(cumm_wgt_dbc)


      print "================================================"      


      print "CPI: "+format((float(paras[2])/float(paras[1])),'.2f')
      print "IPC: "+format((float(paras[1])/float(paras[2])),'.2f')
      
      fop.write(files+","+str(cpi)+","+str(ipc)+","+str(access)+","+str(miss)+","+str(miss_r)+","+str(mpki)+","+str(wgt)+","+str(ribc)+","+str(wibc)+","+str(dbc)+","+str(wrte)+","+str(rwte)+","+str(Read_Intense_Blocks_M)+","+str(Write_Intense_Blocks_M)+","+str(Read_Intense_Blocks_MDuringMigrate)+","+str(Write_Intense_Blocks_MDuringMigrate)+","+str(Blocks_Read_Before_MA)+","+str(Blocks_Read_Before_MB)+","+str(Blocks_Read_Before_MC)+","+str(Blocks_Read_Before_MD)+","+str(Blocks_Read_Before_ME)+","+str(Blocks_Read_Before_MDuringWriteA)+","+str(Blocks_Read_Before_MDuringWriteB)+","+str(Blocks_Read_Before_MDuringWriteC)+","+str(Blocks_Read_Before_MDuringWriteD)+","+str(Blocks_Read_Before_MDuringWriteE)+","+str(Blocks_Write_Before_MA)+","+str(Blocks_Write_Before_MB)+","+str(Blocks_Write_Before_MC)+","+str(Blocks_Write_Before_MD)+","+str(Blocks_Write_Before_ME)+","+str(Blocks_Write_Before_MDuringWriteA)+","+str(Blocks_Write_Before_MDuringWriteB)+","+str(Blocks_Write_Before_MDuringWriteC)+","+str(Blocks_Write_Before_MDuringWriteD)+","+str(Blocks_Write_Before_MDuringWriteE)+","+str(Blocks_Read_After_MA)+","+str(Blocks_Read_After_MB)+","+str(Blocks_Read_After_MC)+","+str(Blocks_Read_After_MD)+","+str(Blocks_Read_After_ME)+","+str(Blocks_Read_After_MDuringWriteA)+","+str(Blocks_Read_After_MDuringWriteB)+","+str(Blocks_Read_After_MDuringWriteC)+","+str(Blocks_Read_After_MDuringWriteD)+","+str(Blocks_Read_After_MDuringWriteE)+","+str(Blocks_Write_After_MA)+","+str(Blocks_Write_After_MB)+","+str(Blocks_Write_After_MC)+","+str(Blocks_Write_After_MD)+","+str(Blocks_Write_After_ME)+","+str(Blocks_Write_After_MDuringWriteA)+","+str(Blocks_Write_After_MDuringWriteB)+","+str(Blocks_Write_After_MDuringWriteC)+","+str(Blocks_Write_After_MDuringWriteD)+","+str(Blocks_Write_After_MDuringWriteE)+","+str(M_Count)+","+str(Migrate_During_Write_Count)+","+str(Valid_Block_Evicted)+","+str(SRAM_Read_Before_Migration)+","+str(SRAM_Read_After_Migration)+","+str(SRAM_Write_Before_Migration)+","+str(SRAM_Write_After_Migration)+","+str(STTRAM_Read_Before_Migration)+","+str(STTRAM_Read_After_Migration)+","+str(STTRAM_Write_Before_Migration)+","+str(STTRAM_Write_After_Migration)+"\n")
     
      
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

  Weighted_Blocks_Read_Before_MA              = cumm_wgt_Blocks_Read_Before_MA/cumm_wgt                   
  Weighted_Blocks_Read_Before_MA             = format(Weighted_Blocks_Read_Before_MA, '.0f')

  Weighted_Blocks_Read_Before_MB             = cumm_wgt_Blocks_Read_Before_MB/cumm_wgt                  
  Weighted_Blocks_Read_Before_MB            = format(Weighted_Blocks_Read_Before_MB, '.0f')

  Weighted_Blocks_Read_Before_MC             = cumm_wgt_Blocks_Read_Before_MC/cumm_wgt                  
  Weighted_Blocks_Read_Before_MC            = format(Weighted_Blocks_Read_Before_MC, '.0f')

  Weighted_Blocks_Read_Before_MD             = cumm_wgt_Blocks_Read_Before_MD/cumm_wgt                  
  Weighted_Blocks_Read_Before_MD            = format(Weighted_Blocks_Read_Before_MD, '.0f')

  Weighted_Blocks_Read_Before_ME            = cumm_wgt_Blocks_Read_Before_ME/cumm_wgt                 
  Weighted_Blocks_Read_Before_ME           = format(Weighted_Blocks_Read_Before_ME, '.0f')

  Weighted_Blocks_Read_Before_MDuringWriteA    = cumm_wgt_Blocks_Read_Before_MDuringWriteA/cumm_wgt         
  Weighted_Blocks_Read_Before_MDuringWriteA   = format(Weighted_Blocks_Read_Before_MDuringWriteA, '.0f')

  Weighted_Blocks_Read_Before_MDuringWriteB   = cumm_wgt_Blocks_Read_Before_MDuringWriteB/cumm_wgt        
  Weighted_Blocks_Read_Before_MDuringWriteB  = format(Weighted_Blocks_Read_Before_MDuringWriteB, '.0f')

  Weighted_Blocks_Read_Before_MDuringWriteC   = cumm_wgt_Blocks_Read_Before_MDuringWriteC/cumm_wgt        
  Weighted_Blocks_Read_Before_MDuringWriteC  = format(Weighted_Blocks_Read_Before_MDuringWriteC, '.0f')

  Weighted_Blocks_Read_Before_MDuringWriteD   = cumm_wgt_Blocks_Read_Before_MDuringWriteD/cumm_wgt        
  Weighted_Blocks_Read_Before_MDuringWriteD  = format(Weighted_Blocks_Read_Before_MDuringWriteD, '.0f')

  Weighted_Blocks_Read_Before_MDuringWriteE  = cumm_wgt_Blocks_Read_Before_MDuringWriteE/cumm_wgt       
  Weighted_Blocks_Read_Before_MDuringWriteE = format(Weighted_Blocks_Read_Before_MDuringWriteE, '.0f')

  Weighted_Blocks_Write_Before_MA             = cumm_wgt_Blocks_Write_Before_MA/cumm_wgt                  
  Weighted_Blocks_Write_Before_MA            = format(Weighted_Blocks_Write_Before_MA, '.0f')

  Weighted_Blocks_Write_Before_MB            = cumm_wgt_Blocks_Write_Before_MB/cumm_wgt                 
  Weighted_Blocks_Write_Before_MB           = format(Weighted_Blocks_Write_Before_MB, '.0f')

  Weighted_Blocks_Write_Before_MC            = cumm_wgt_Blocks_Write_Before_MC/cumm_wgt                 
  Weighted_Blocks_Write_Before_MC           = format(Weighted_Blocks_Write_Before_MC, '.0f')

  Weighted_Blocks_Write_Before_MD            = cumm_wgt_Blocks_Write_Before_MD/cumm_wgt                 
  Weighted_Blocks_Write_Before_MD           = format(Weighted_Blocks_Write_Before_MD, '.0f')

  Weighted_Blocks_Write_Before_ME           = cumm_wgt_Blocks_Write_Before_ME/cumm_wgt                
  Weighted_Blocks_Write_Before_ME          = format(Weighted_Blocks_Write_Before_ME, '.0f')

  Weighted_Blocks_Write_Before_MDuringWriteA   = cumm_wgt_Blocks_Write_Before_MDuringWriteA/cumm_wgt        
  Weighted_Blocks_Write_Before_MDuringWriteA  = format(Weighted_Blocks_Write_Before_MDuringWriteA, '.0f')

  Weighted_Blocks_Write_Before_MDuringWriteB  = cumm_wgt_Blocks_Write_Before_MDuringWriteB/cumm_wgt       
  Weighted_Blocks_Write_Before_MDuringWriteB = format(Weighted_Blocks_Write_Before_MDuringWriteB, '.0f')

  Weighted_Blocks_Write_Before_MDuringWriteC  = cumm_wgt_Blocks_Write_Before_MDuringWriteC/cumm_wgt       
  Weighted_Blocks_Write_Before_MDuringWriteC = format(Weighted_Blocks_Write_Before_MDuringWriteC, '.0f')

  Weighted_Blocks_Write_Before_MDuringWriteD  = cumm_wgt_Blocks_Write_Before_MDuringWriteD/cumm_wgt       
  Weighted_Blocks_Write_Before_MDuringWriteD = format(Weighted_Blocks_Write_Before_MDuringWriteD, '.0f')

  Weighted_Blocks_Write_Before_MDuringWriteE = cumm_wgt_Blocks_Write_Before_MDuringWriteE/cumm_wgt      
  Weighted_Blocks_Write_Before_MDuringWriteE= format(Weighted_Blocks_Write_Before_MDuringWriteE, '.0f')

  Weighted_Blocks_Read_After_MA               = cumm_wgt_Blocks_Read_After_MA/cumm_wgt                    
  Weighted_Blocks_Read_After_MA              = format(Weighted_Blocks_Read_After_MA, '.0f')

  Weighted_Blocks_Read_After_MB              = cumm_wgt_Blocks_Read_After_MB/cumm_wgt                   
  Weighted_Blocks_Read_After_MB             = format(Weighted_Blocks_Read_After_MB, '.0f')

  Weighted_Blocks_Read_After_MC              = cumm_wgt_Blocks_Read_After_MC/cumm_wgt                   
  Weighted_Blocks_Read_After_MC             = format(Weighted_Blocks_Read_After_MC, '.0f')

  Weighted_Blocks_Read_After_MD              = cumm_wgt_Blocks_Read_After_MD/cumm_wgt                   
  Weighted_Blocks_Read_After_MD             = format(Weighted_Blocks_Read_After_MD, '.0f')

  Weighted_Blocks_Read_After_ME             = cumm_wgt_Blocks_Read_After_ME/cumm_wgt                  
  Weighted_Blocks_Read_After_ME            = format(Weighted_Blocks_Read_After_ME, '.0f')

  Weighted_Blocks_Read_After_MDuringWriteA     = cumm_wgt_Blocks_Read_After_MDuringWriteA/cumm_wgt          
  Weighted_Blocks_Read_After_MDuringWriteA    = format(Weighted_Blocks_Read_After_MDuringWriteA, '.0f')

  Weighted_Blocks_Read_After_MDuringWriteB    = cumm_wgt_Blocks_Read_After_MDuringWriteB/cumm_wgt         
  Weighted_Blocks_Read_After_MDuringWriteB   = format(Weighted_Blocks_Read_After_MDuringWriteB, '.0f')

  Weighted_Blocks_Read_After_MDuringWriteC    = cumm_wgt_Blocks_Read_After_MDuringWriteC/cumm_wgt         
  Weighted_Blocks_Read_After_MDuringWriteC   = format(Weighted_Blocks_Read_After_MDuringWriteC, '.0f')

  Weighted_Blocks_Read_After_MDuringWriteD    = cumm_wgt_Blocks_Read_After_MDuringWriteD/cumm_wgt         
  Weighted_Blocks_Read_After_MDuringWriteD   = format(Weighted_Blocks_Read_After_MDuringWriteD, '.0f')

  Weighted_Blocks_Read_After_MDuringWriteE   = cumm_wgt_Blocks_Read_After_MDuringWriteE/cumm_wgt        
  Weighted_Blocks_Read_After_MDuringWriteE  = format(Weighted_Blocks_Read_After_MDuringWriteE, '.0f')

  Weighted_Blocks_Write_After_MA              = cumm_wgt_Blocks_Write_After_MA/cumm_wgt                   
  Weighted_Blocks_Write_After_MA             = format(Weighted_Blocks_Write_After_MA, '.0f')

  Weighted_Blocks_Write_After_MB             = cumm_wgt_Blocks_Write_After_MB/cumm_wgt                  
  Weighted_Blocks_Write_After_MB            = format(Weighted_Blocks_Write_After_MB, '.0f')

  Weighted_Blocks_Write_After_MC             = cumm_wgt_Blocks_Write_After_MC/cumm_wgt                  
  Weighted_Blocks_Write_After_MC            = format(Weighted_Blocks_Write_After_MC, '.0f')

  Weighted_Blocks_Write_After_MD             = cumm_wgt_Blocks_Write_After_MD/cumm_wgt                  
  Weighted_Blocks_Write_After_MD            = format(Weighted_Blocks_Write_After_MD, '.0f')

  Weighted_Blocks_Write_After_ME            = cumm_wgt_Blocks_Write_After_ME/cumm_wgt                 
  Weighted_Blocks_Write_After_ME           = format(Weighted_Blocks_Write_After_ME, '.0f')

  Weighted_Blocks_Write_After_MDuringWriteA    = cumm_wgt_Blocks_Write_After_MDuringWriteA/cumm_wgt         
  Weighted_Blocks_Write_After_MDuringWriteA   = format(Weighted_Blocks_Write_After_MDuringWriteA, '.0f')

  Weighted_Blocks_Write_After_MDuringWriteB   = cumm_wgt_Blocks_Write_After_MDuringWriteB/cumm_wgt        
  Weighted_Blocks_Write_After_MDuringWriteB  = format(Weighted_Blocks_Write_After_MDuringWriteB, '.0f')

  Weighted_Blocks_Write_After_MDuringWriteC   = cumm_wgt_Blocks_Write_After_MDuringWriteC/cumm_wgt        
  Weighted_Blocks_Write_After_MDuringWriteC  = format(Weighted_Blocks_Write_After_MDuringWriteC, '.0f')

  Weighted_Blocks_Write_After_MDuringWriteD   = cumm_wgt_Blocks_Write_After_MDuringWriteD/cumm_wgt        
  Weighted_Blocks_Write_After_MDuringWriteD  = format(Weighted_Blocks_Write_After_MDuringWriteD, '.0f')

  Weighted_Blocks_Write_After_MDuringWriteE  = cumm_wgt_Blocks_Write_After_MDuringWriteE/cumm_wgt       
  Weighted_Blocks_Write_After_MDuringWriteE = format(Weighted_Blocks_Write_After_MDuringWriteE, '.0f')

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
  fop.write("Benchmark, Weighted IPC, Weighted L3 Miss, Weighted ribc, Weighted wibc, Weighted dbc, Weighted wrte, Weighted rwte, Weighted Read Intense Blocks M, Weighted Write Intense Blocks M, Weighted Read Intense Blocks MDuringMigrate, Weighted Write Intense Blocks MDuringMigrate, Weighted Blocks Read Before MA, Weighted Blocks Read Before MB, Weighted Blocks Read Before MC, Weighted Blocks Read Before MD, Weighted Blocks Read Before ME, Weighted Blocks Read Before MDuringWriteA, Weighted Blocks Read Before MDuringWriteB, Weighted Blocks Read Before MDuringWriteC, Weighted Blocks Read Before MDuringWriteD, Weighted Blocks Read Before MDuringWriteE, Weighted Blocks Write Before MA, Weighted Blocks Write Before MB, Weighted Blocks Write Before MC, Weighted Blocks Write Before MD, Weighted Blocks Write Before ME, Weighted Blocks Write Before MDuringWriteA, Weighted Blocks Write Before MDuringWriteB, Weighted Blocks Write Before MDuringWriteC, Weighted Blocks Write Before MDuringWriteD, Weighted Blocks Write Before MDuringWriteE, Weighted Blocks Read After MA, Weighted Blocks Read After MB, Weighted Blocks Read After MC, Weighted Blocks Read After MD, Weighted Blocks Read After ME, Weighted Blocks Read After MDuringWriteA, Weighted Blocks Read After MDuringWriteB, Weighted Blocks Read After MDuringWriteC, Weighted Blocks Read After MDuringWriteD, Weighted Blocks Read After MDuringWriteE, Weighted Blocks Write After MA, Weighted Blocks Write After MB, Weighted Blocks Write After MC, Weighted Blocks Write After MD, Weighted Blocks Write After ME, Weighted Blocks Write After MDuringWriteA, Weighted Blocks Write After MDuringWriteB, Weighted Blocks Write After MDuringWriteC, Weighted Blocks Write After MDuringWriteD, Weighted Blocks Write After MDuringWriteE, Weighted M Count, Weighted Migrate During Write Count, Weighted Valid Blocks Evicted, Weighted SRAM Read Before Migration, Weighted SRAM Read After Migration, Weighted SRAM Write Before Migration, Weighted SRAM Write After Migration, Weighted STTRAM Read Before Migration, Weighted STTRAM Read After Migration, Weighted STTRAM Write Before Migration, Weighted STTRAM Write After Migration\n")
  #fop.write(application+","+str(Weighted_cpi)+","+str(Weighted_ipc)+","+Weighted_mpki+"\n")
  fop.write(application+","+str(Weighted_ipc)+","+str(Weighted_l3miss)+","+str(Weighted_ribc)+","+str(Weighted_wibc)+","+str(Weighted_dbc)+","+str(Weighted_wrte)+","+str(Weighted_rwte)+","+str(Weighted_Read_Intense_Blocks_M)+","+str(Weighted_Write_Intense_Blocks_M)+","+str(Weighted_Read_Intense_Blocks_MDuringMigrate)+","+str(Weighted_Write_Intense_Blocks_MDuringMigrate)+","+str(Weighted_Blocks_Read_Before_MA)+","+str(Weighted_Blocks_Read_Before_MB)+","+str(Weighted_Blocks_Read_Before_MC)+","+str(Weighted_Blocks_Read_Before_MD)+","+str(Weighted_Blocks_Read_Before_ME)+","+str(Weighted_Blocks_Read_Before_MDuringWriteA)+","+str(Weighted_Blocks_Read_Before_MDuringWriteB)+","+str(Weighted_Blocks_Read_Before_MDuringWriteC)+","+str(Weighted_Blocks_Read_Before_MDuringWriteD)+","+str(Weighted_Blocks_Read_Before_MDuringWriteE)+","+str(Weighted_Blocks_Write_Before_MA)+","+str(Weighted_Blocks_Write_Before_MB)+","+str(Weighted_Blocks_Write_Before_MC)+","+str(Weighted_Blocks_Write_Before_MD)+","+str(Weighted_Blocks_Write_Before_ME)+","+str(Weighted_Blocks_Write_Before_MDuringWriteA)+","+str(Weighted_Blocks_Write_Before_MDuringWriteB)+","+str(Weighted_Blocks_Write_Before_MDuringWriteC)+","+str(Weighted_Blocks_Write_Before_MDuringWriteD)+","+str(Weighted_Blocks_Write_Before_MDuringWriteE)+","+str(Weighted_Blocks_Read_After_MA)+","+str(Weighted_Blocks_Read_After_MB)+","+str(Weighted_Blocks_Read_After_MC)+","+str(Weighted_Blocks_Read_After_MD)+","+str(Weighted_Blocks_Read_After_ME)+","+str(Weighted_Blocks_Read_After_MDuringWriteA)+","+str(Weighted_Blocks_Read_After_MDuringWriteB)+","+str(Weighted_Blocks_Read_After_MDuringWriteC)+","+str(Weighted_Blocks_Read_After_MDuringWriteD)+","+str(Weighted_Blocks_Read_After_MDuringWriteE)+","+str(Weighted_Blocks_Write_After_MA)+","+str(Weighted_Blocks_Write_After_MB)+","+str(Weighted_Blocks_Write_After_MC)+","+str(Weighted_Blocks_Write_After_MD)+","+str(Weighted_Blocks_Write_After_ME)+","+str(Weighted_Blocks_Write_After_MDuringWriteA)+","+str(Weighted_Blocks_Write_After_MDuringWriteB)+","+str(Weighted_Blocks_Write_After_MDuringWriteC)+","+str(Weighted_Blocks_Write_After_MDuringWriteD)+","+str(Weighted_Blocks_Write_After_MDuringWriteE)+","+str(Weighted_M_Count)+","+str(Weighted_Migrate_During_Write_Count)+","+str(Weighted_Valid_Block_Evicted)+","+str(Weighted_SRAM_Read_Before_Migration)+","+str(Weighted_SRAM_Read_After_Migration)+","+str(Weighted_SRAM_Write_Before_Migration)+","+str(Weighted_SRAM_Write_After_Migration)+","+str(Weighted_STTRAM_Read_Before_Migration)+","+str(Weighted_STTRAM_Read_After_Migration)+","+str(Weighted_STTRAM_Write_Before_Migration)+","+str(Weighted_STTRAM_Write_After_Migration)+"\n")

  ts = time.time()
  st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
  fop.write("\n\n\n\n"+st)
           
if __name__ == "__main__":
  main()
