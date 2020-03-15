#include "cache_set_phc.h"
#include "log.h"
#include "stats.h"
#include "simulator.h"
#include "config.hpp"
#include "cache.h"
#include "cache_set.h"

/*
#include "cache_cntlr.h"
#include "memory_manager.h"
#include "core_manager.h"
#include "subsecond_time.h"
#include "fault_injection.h"
#include "hooks_manager.h"
#include "cache_atd.h"
#include "shmem_perf.h"
*/


///////// required for dynamic cost change/////////////
UInt8 CplusK = 255;						//C+(K0)
UInt8 CminusK = 0;						//C-(K0)
UInt8 CplusKplus = 255;					//C+(K+)
UInt8 CplusKminus = 255;				//C+(K-)
UInt8 CminusKplus = 0;					//C-(K+)
UInt8 CminusKminus = 0;					//C-(K-)

UInt8 Cmax = 128;
UInt8 Cmin = 128;
UInt8 Cplusmax = 128;
UInt8 Cplusmin = 128;
UInt8 Cminusmax = 128;
UInt8 Cminusmin = 128;

static UInt16 Mplus = 0;               //counters. Same as M0, M+ and M- in paper
static UInt16 Mminus = 0;
static UInt16 M0 = 0;

static UInt8 C[4096] = {0};            //evicted Cost set for K0
UInt16 Clength = 0;                    //length of Cost set for K0
static UInt8 Cplus[4096] = {0};        //evicted Cost set for K+
UInt16 Cpluslength = 0;                //length of Cost set for K+
static UInt8 Cminus[4096] = {0};       //evicted Cost set for K-
UInt16 Cminuslength = 0;               //length of Cost set for K-

UInt8 K = 148;                         //thresholds. Same as k, K+ and K- in paper
UInt8 Kplus = 149;
UInt8 Kminus = 147;


///////// required for static cost change////////////////

//static UInt16 threshold_plus_miss_counter = 0;
//static UInt16 threshold_minus_miss_counter = 0;
//static UInt16 threshold_miss_counter = 0;

UInt8 cost_threshold = 148;
UInt8 cost_threshold_plus = 149;
UInt8 cost_threshold_minus = 147;
//in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me. So threshold of 20 translates to 148.

/////////////////////////////////////////////////////////

UInt8 Ew=24;
UInt8 Er=1;
UInt16 predictor_table_length=256;
UInt8 state_threshold=2;
UInt8 state_max=3;
UInt8 SRAM_ways=4;
UInt32 number_of_sets = 8192;
UInt32 sampler_fraction = 32;

extern UInt64 globalWritebacksToL3counter;   //this is a global counter. this counter will be reset when updateReplacementindex for phc in LLC is called 
UInt8 migrate_flag = 0;

static UInt8 m_state[256] = {0};             //state table used by sampler set 3-63
static UInt8 m_state_plus[256] = {0};        //state table used by sampler set 1
static UInt8 m_state_minus[256] = {0};       //state table used by sampler set 2

static UInt8 m_dcnt[256] = {0};              //deadblock predictor table
static UInt8 dcnt_threshold = 128;           //deadblock predictor table threshold
static UInt8 dcnt_initialization = 0;        //this variable is used to make sure dcnt is initialized only once 

static UInt8 asl2_flag = 0;


static UInt16 lru_miss_counter = 0;

static UInt16 totalCacheMissCounter = 0;
static UInt16 totalCacheMissCounter_saturation = 131071;


static UInt64 readToWriteTransitionsAtInterval = 0;
static UInt64 writeToReadTransitionsAtInterval = 0;
static UInt64 readToWriteTransitionsAtEviction = 0;
static UInt64 writeToReadTransitionsAtEviction = 0;

static UInt16 histogram[1000] = {0};
static UInt8 sf = 4;

static UInt8 N_transition = 1; //interval length for wrti, rwti calculation

static UInt8  g_iteration_count         = 0;    //copied from Udal's file

static UInt64 read_intense_block_counter = 0;
static UInt64 write_intense_block_counter = 0;
static UInt64 deadblock_counter = 0;



CacheSetPHC::CacheSetPHC(
      CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize, CacheSetInfoLRU* set_info, UInt8 num_attempts)
   : CacheSet(cache_type, associativity, blocksize)
   , m_num_attempts(num_attempts)
   , m_set_info(set_info)
{ 

   m_lru_bits = new UInt8[m_associativity];

   for (UInt32 i = 0; i < m_associativity; i++)
      m_lru_bits[i] = i;

   m_TI = new UInt16[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
      m_TI[i] = 0; 

   m_cost = new UInt8[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      m_cost[i] = 128;  //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.
   }

   ////////for checing migration severity////////////////////
   write_array = new UInt16[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      write_array[i] = 0;
   }

   read_array = new UInt16[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      read_array[i] = 0;
   }


   access_counter = new UInt16[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      access_counter[i] = 0;
   }

   if(dcnt_initialization==0)    //Used to initiaize dcnt array to 128 only once. This is a global array, not for a particular set 
   {
      for(UInt32 i = 0; i<256; i++)
      {
         m_dcnt[i] = 128;
      }
      dcnt_initialization = 1;
   }

   m_deadblock = new UInt8[m_associativity];    //deadblock counter using Newton's method
   for (UInt32 i = 0; i < m_associativity; i++)
      m_deadblock[i] = 0; 
   

   ///////////////////////////////////////////////////////
   if (0 == g_iteration_count)   //copied from Udal. This loop ensures that register stat metric is called once instead of for all the sets
   {
      //printf("\n\nInterval length for phase change is %d and interval length for wrti is %d\n", totalCacheMissCounter_saturation, N_transition);
      printf("Associativity is %d, SRAM ways are %d\n\n\n", m_associativity, SRAM_ways);
      g_iteration_count++;
      //registerStatsMetric("interval_timer", 0 , "Read_To_Write_Transitions_At_Interval", &readToWriteTransitionsAtInterval);
      //registerStatsMetric("interval_timer", 0 , "Read_To_Write_Transitions_At_Eviction", &readToWriteTransitionsAtEviction);
      //registerStatsMetric("interval_timer", 0 , "Write_To_Read_Transitions_At_Interval", &writeToReadTransitionsAtInterval);
      registerStatsMetric("interval_timer", 0 , "Write_To_Read_Transitions_At_Eviction", &writeToReadTransitionsAtEviction);
      registerStatsMetric("interval_timer", 0 , "Read_Intense_Block_Counter", &read_intense_block_counter);
      registerStatsMetric("interval_timer", 0 , "Write_Intense_Block_Counter", &write_intense_block_counter);
      registerStatsMetric("interval_timer", 0 , "Deadblock_Counter", &deadblock_counter);

   }
}



CacheSetPHC::~CacheSetPHC()
{
   delete [] m_lru_bits;
   delete [] m_TI;
   delete [] m_cost;
   delete [] write_array;
   delete [] read_array;
   delete [] access_counter;
   delete [] m_deadblock;
}


UInt32
CacheSetPHC::getReplacementIndex(CacheCntlr *cntlr, IntPtr eip, UInt32 set_index)   //implements dynamic threshold;both sampler and non sampler has cost;overhead is high; DYNAMIC COST CHANGE
{
   totalCacheMissCounter++;
   //printf("totalCacheMissCounter is %d, Clength, Cpluslength and Cminuslength are %d, %d, %d \n",totalCacheMissCounter, Clength, Cpluslength, Cminuslength);
   //printf("totalCacheMissCounter is %d, Cplusmax, Cplusmin, Cminusmax, Cminusmin, Cmax, Cmin are %d, %d, %d, %d, %d, %d \n",totalCacheMissCounter, Cplusmax, Cplusmin, Cminusmax, Cminusmin, Cmax, Cmin);
   //printf("K is %d, Kplus is %d and Kminus is %d\n", K, Kplus, Kminus);

   if (totalCacheMissCounter == totalCacheMissCounter_saturation) //phase change
   {







      /*


   	//sorting the c array
      int swap;
      for (int cc = 0 ; cc < (Clength-1); cc++)
      {
         for (int d = 0 ; d < (Clength-cc-1); d++)
         {
            if (C[d] > C[d+1])   // For decreasing order use < 
            {
               swap = C[d];
               C[d] = C[d+1];
               C[d+1] = swap;
            }
         }
      }

      //the c array is sorted at this point
      for(int j=1; j<Clength; j++)           //why starting from j=1?? If we get threshold match at K=0, we cant put CminusK as C[-1]
      {
         if(C[0]>K)                          //all members of c are greater than K
         {
            CminusK = K;
            break;
         }
         else                                //some members are less than K. Find the max among them
         {
            if(C[j]>=K)
            {
               CminusK = C[j-1];
               break;
            }
         }
         CminusK = C[j];
      }
         
      for(int j=(Clength-2); j>=0; j--)      //why starting from Clength-2. Suppose Clength = 5. If we find a match at array index 4 (i.e. first element from back), we cant put CplusK as C[5]
      {
         if(C[Clength-1]<K)                  //all members are smaller than K
         {
            CplusK = K;
            break;
         }
         else                                //some members are bigger than K. Find the min among them
         {
            if(C[j]<=K)
            {
               CplusK = C[j+1];
               break;
            }
         }
         CplusK = C[j];     
      }


      for (int cc = 0 ; cc < (Cpluslength-1); cc++)
      {
         for (int d = 0 ; d < (Cpluslength-cc-1); d++)
         {
            if (Cplus[d] > Cplus[d+1]) // For decreasing order use < 
            {
               swap = Cplus[d];
               Cplus[d] = Cplus[d+1];
               Cplus[d+1] = swap;
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////

      for(int j=1; j<Cpluslength; j++)             //why starting from j=1?? If weger threshold match at K=0, we cant put CminusK  //as C[-1]
      {
         if(Cplus[0]>Kplus)                        //all members are greater than K
         {
            CminusKplus = Kplus;
            break;
         }
         else                                      //some members are less than K. Find the max among them
         {
            if(Cplus[j]>=Kplus)
            {
               CminusKplus = Cplus[j-1];
               break;
            }
         }
         CminusKplus = Cplus[j];
      }
         
      for(int j=(Cpluslength-2); j>=0; j--)        //why starting from Clength-2. Suppose Clength = 5. If we find a match at  //array index 4 (i.e. first element from back), we cant put CplusK as C[5]
      {
         if(Cplus[Cpluslength-1]<Kplus)            //all members are smaller than K
         {
            CplusKplus = Kplus;
            break;
         }
         else                                      //some members are bigger than K. Find the min among them
         {
            if(Cplus[j]<=Kplus)
            {
               CplusKplus = Cplus[j+1];
               break;
            }
         }
         CplusKplus = Cplus[j];
           
      }

      ////////////////////////////////////////////////////////////////////////////////////

      for (int cc = 0 ; cc < (Cminuslength-1); cc++)
      {
         for (int d = 0 ; d < (Cminuslength-cc-1); d++)
         {
            if (Cminus[d] > Cminus[d+1])  // For decreasing order use < 
            {
               swap = Cminus[d];
               Cminus[d] = Cminus[d+1];
               Cminus[d+1] = swap;
            }
         }
      }

      /////////////////////////////////////////////////////////////////////////////////

      for(int j=1; j<Cminuslength; j++)             //why starting from j=1?? If weger threshold match at K=0, we cant put CminusK  //as C[-1]
      {
         if(Cminus[0]>Kminus)                        //all members are greater than K
         {
            CminusKminus = Kminus;
            break;
         }
         else                                      //some members are less than K. Find the max among them
         {
            if(Cminus[j]>=Kminus)
            {
               CminusKminus = Cminus[j-1];
               break;
            }
         }
         CminusKminus = Cminus[j];
      }
         
      for(int j=(Cminuslength-2); j>=0; j--)        //why starting from Clength-2. Suppose Clength = 5. If we find a match at  //array index 4 (i.e. first element from back), we cant put CplusK as C[5]
      {
         if(Cminus[Cminuslength-1]<Kminus)            //all members are smaller than K
         {
            CplusKminus = Kminus;
            break;
         }
         else                                      //some members are bigger than K. Find the min among them
         {
            if(Cminus[j]<=Kminus)
            {
               CplusKminus = Cminus[j+1];
               break;
            }
         }
         CplusKminus = Cminus[j];
           
      }
      
      */





      ///////////////////////////////////////////////////////

      //finding out CminusK
      if(Cmin<K)  //at least one member is smaller than K
         CminusK = Cmax;
      else 
         CminusK = K;

      //finding out CplusK
      if(Cmax>K)  //at least one member is greater than K
         CplusK = Cmin;
      else 
         CplusK = K;

      ///////////////////////////////////////////////////////

      //finding out CminusKminus
      if(Cminusmin<Kminus)
         CminusKminus = Cminusmax;
      else 
         CminusKminus = Kminus;

      //finding out CplusKminus
      if(Cminusmax>Kminus)
         CplusKminus = Cminusmin;
      else 
         CplusKminus = Kminus;

      ///////////////////////////////////////////////////////

      //finding out CminusKplus
      if(Cplusmin<Kplus)
         CminusKplus = Cplusmax;
      else
         CminusKplus = Kplus;

      //finding out CplusKplus
      if(Cplusmax>Kplus)
         CplusKplus = Cplusmin;
      else 
         CplusKplus = Kplus;


















      /////////////////////////////////////////////////////////////////////////////////


      if((Mminus<M0) && (Mminus<Mplus))         //decrement threshold; K should be replaced by Kminus
      {
         K = Kminus;
         Kplus = CplusKminus;
         Kminus = CminusKminus;         
      }
      else if((Mplus<M0) && (Mplus<Mminus))     //increment threshold; K should be replaced by Kplus
      {
         K = Kplus;
         Kplus = CplusKplus;
         Kminus = CminusKplus;
      }
      else                                      //only change K+ and K-                       
      {
         Kminus = CminusK;
         Kplus = CplusK;
      }
      

      //reinitialization
      lru_miss_counter = 0;
      Mplus = 0;
      Mminus = 0;
      M0 = 0;
      totalCacheMissCounter = 0;

      Cmax = 128;
      Cmin = 128;
      Cplusmax = 128;
      Cplusmin = 128;
      Cminusmax = 128;
      Cminusmin = 128;
      
      Clength = 0;
      Cpluslength = 0;
      Cminuslength = 0;

      for (int j=0; j<4096; j++)
      {
         C[j]=0;
         Cplus[j]=0;
         Cminus[j]=0;
      }

      //printf("RI = %d, WI = %d and deadblocks = %d before averaging\n", read_intense_block_counter, write_intense_block_counter, deadblock_counter);
   
      read_intense_block_counter = read_intense_block_counter/2;
      write_intense_block_counter = write_intense_block_counter/2;
      deadblock_counter = deadblock_counter/2;
      //printf("RI = %d, WI = %d and deadblocks = %d after averaging\n", read_intense_block_counter, write_intense_block_counter, deadblock_counter);
   
      //printf("\n/////////////////////////////////\n");

   }
   



   
   //UInt16 eip_truncated=eip%256;  //truncating eip to last 12bits
   UInt16 eip_truncated = truncatedEipCalculation(eip);  //this is done for generating hashed PC. Last 5 bytes (LSB) are XOR-ed

   if ((set_index % sampler_fraction)==0) //sampler set, uses lru replacement
   {
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (!m_cache_block_info_array[i]->isValid())
         {
            // Mark our newly-inserted line as most-recently used
   
            m_TI[i]=eip_truncated;
            m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

            write_array[i] = 0;  //reset the counters on eviction. 
            read_array[i] = 0;
            access_counter[i] = 0;
            m_deadblock[i] = 0;

            moveToMRU(i);
            return i;
         }
      }

      //INVALID BLOCK NOT FOUND
      // Make m_num_attemps attempts at evicting the block at LRU position
      for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt)
      {
         UInt32 index = 0;
         UInt8 max_bits = 0;

         for (UInt32 i = 0; i < m_associativity; i++)
         {
            if (m_lru_bits[i] > max_bits && isValidReplacement(i))
            {
               index = i;
               max_bits = m_lru_bits[i];
            }
         }
         //printf("index is %d and m_associativity is %d\n", index, m_associativity);
         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");
   
      
         // Mark our newly-inserted line as most-recently used

         if((m_cost[index]>K) && (m_state[m_TI[index]]<state_max)) //state should be incremented
         {
            m_state[m_TI[index]]++;
         }
         else if((m_cost[index]<K) && (m_state[m_TI[index]]>0))
         {
            m_state[m_TI[index]]--;
         }
         else
         {

         }
         
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

         if((index>=0) && (index<SRAM_ways) && (read_array[index]>(sf*write_array[index])))  //SRAM ways. Predicted Write intensive
            writeToReadTransitionsAtEviction++; //since number of reads exceed number of writes, there is a transition
         if((index>=SRAM_ways) && (index<m_associativity) && (read_array[index]<(sf*write_array[index])))   //STTRAM ways, predicted read intensive
            readToWriteTransitionsAtEviction++; //since number of writes exceed number of reads, there is a transition

         histogram[access_counter[index]]++;
         access_counter[index] = 0;

         write_array[index] = 0;  //reset the counters on eviction
         read_array[index] = 0;
         m_deadblock[index] = 0;

         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);

         if(m_dcnt[m_TI[index]] != 255)
            m_dcnt[m_TI[index]]++;  //increment deadblock counter on eviction

         
         return index;
         
      }   
      
   }

   else if ((set_index % sampler_fraction)==1)  //sampler set using Kplus and m_state_plus
   {
      Mplus++;
     
      //finding out invalid blocks
      if(m_state_plus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {
         
         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               m_TI[i]=eip_truncated;
               m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

               
               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               access_counter[i] = 0;
               m_deadblock[i] = 0;

               moveToMRU(i);
               return i;
            }
      
         }
      }
   
      else  //TI is hot. select victim from SRAM
      {
         for (UInt32 i = 0; i < SRAM_ways; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            { 
               m_TI[i]=eip_truncated;
               m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

               

               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               access_counter[i] = 0;
               m_deadblock[i] = 0;

               moveToMRU(i);
               return i;
            }
         }
      }
   
      //trying to find an invalid block if it is present but not in the proper partition
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (!m_cache_block_info_array[i]->isValid())
         { 
            m_TI[i]=eip_truncated;
            m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.


            write_array[i] = 0;  //reset the counters on eviction
            read_array[i] = 0;
            access_counter[i] = 0;
            m_deadblock[i] = 0;
               
            moveToMRU(i);
            return i;
         }
      }


      //INVALID BLOCK NOT FOUND
      // Make m_num_attemps attempts at evicting the block at LRU position
      for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt)
      {
         UInt32 index = 0;
         UInt8 max_bits = 0;

         if(m_state_plus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
         {
            for (UInt32 i = SRAM_ways; i < m_associativity; i++)
            {
               if (m_lru_bits[i] > max_bits && isValidReplacement(i))
               {
                  index = i;
                  max_bits = m_lru_bits[i];
               }
            }
         }
   
         else  //TI is hot. select victim from SRAM
         {   
            for (UInt32 i = 0; i < SRAM_ways; i++)
            {
               if (m_lru_bits[i] > max_bits && isValidReplacement(i))
               {
                  index = i;
                  max_bits = m_lru_bits[i];
               }
            }
            //call function (migrate(index)) here
            migrate(index);
            
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");
   
      
         // Mark our newly-inserted line as most-recently used
         if((m_cost[index]>Kplus) && (m_state_plus[m_TI[index]]<state_max)) //state should be incremented
            m_state_plus[m_TI[index]]++;
         else if((m_cost[index]<Kplus) && (m_state_plus[m_TI[index]]>0))
            m_state_plus[m_TI[index]]--;

         Cplus[Cpluslength]=m_cost[index];
         Cpluslength++;

         //Calculating Cplusmax and Cplusmin
         if(m_cost[index]<Cplusmin)
            Cplusmin = m_cost[index];
         if(m_cost[index]>Cplusmax)
            Cplusmax = m_cost[index];


         
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

         if((index>=0) && (index<SRAM_ways) && (read_array[index]>(sf*write_array[index])))  //SRAM ways. Predicted Write intensive
         {
            writeToReadTransitionsAtEviction++;
         }
         if((index>=SRAM_ways) && (index<m_associativity) && (read_array[index]<(sf*write_array[index])))   //STTRAM ways, predicted read intensive
         {
            readToWriteTransitionsAtEviction++;
         }


         histogram[access_counter[index]]++;
         access_counter[index] = 0;

         write_array[index] = 0;  //reset the counters on eviction
         read_array[index] = 0;
         m_deadblock[index] = 0;
               
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt); 

         if(m_dcnt[m_TI[index]] != 255)
            m_dcnt[m_TI[index]]++;  //increment deadblock counter on eviction

         return index;
      
      }   
      
   }

   else if ((set_index % sampler_fraction)==2)  //sampler set using Kminus and m_state_minus
   {
      Mminus++;

      //finding out invalid blocks
      if(m_state_minus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {
         
         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               m_TI[i]=eip_truncated;
               m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

               
               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               access_counter[i] = 0;
               m_deadblock[i] = 0;
               
               moveToMRU(i);
               return i;
            }
      
         }
      }
   
      else  //TI is hot. select victim from SRAM
      {
         for (UInt32 i = 0; i < SRAM_ways; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            { 
               m_TI[i]=eip_truncated;
               m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.


               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               access_counter[i] = 0;
               m_deadblock[i] = 0;
               
               moveToMRU(i);
               return i;
            }
         }
      }
   
      //trying to find an invalid block if it is present but not in the proper partition
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (!m_cache_block_info_array[i]->isValid())
         { 
            m_TI[i]=eip_truncated;
            m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.


            write_array[i] = 0;  //reset the counters on eviction
            read_array[i] = 0;
            access_counter[i] = 0;
            m_deadblock[i] = 0;
               
            moveToMRU(i);
            return i;
         }
      }


      //INVALID BLOCK NOT FOUND
      // Make m_num_attemps attempts at evicting the block at LRU position
      for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt)
      {
         UInt32 index = 0;
         UInt8 max_bits = 0;

         if(m_state_minus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
         {
            for (UInt32 i = SRAM_ways; i < m_associativity; i++)
            {
               if (m_lru_bits[i] > max_bits && isValidReplacement(i))
               {
                  index = i;
                  max_bits = m_lru_bits[i];
               }
            }
         }
   
         else  //TI is hot. select victim from SRAM
         {   
            for (UInt32 i = 0; i < SRAM_ways; i++)
            {
               if (m_lru_bits[i] > max_bits && isValidReplacement(i))
               {
                  index = i;
                  max_bits = m_lru_bits[i];
               }
            }
            migrate(index);
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");

         // Mark our newly-inserted line as most-recently used
         if((m_cost[index]>Kminus) && (m_state_minus[m_TI[index]]<state_max)) //state should be incremented
            m_state_minus[m_TI[index]]++;
         else if((m_cost[index]<Kminus) && (m_state_minus[m_TI[index]]>0))
            m_state_minus[m_TI[index]]--;

         Cminus[Cminuslength]=m_cost[index];
         Cminuslength++;

         //Calculating Cminusmax and Cminusmin
         if(m_cost[index]<Cminusmin)
            Cminusmin = m_cost[index];
         if(m_cost[index]>Cminusmax)
            Cminusmax = m_cost[index];
         


         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

         if((index>=0) && (index<SRAM_ways) && (read_array[index]>(sf*write_array[index])))  //SRAM ways. Predicted Write intensive
            writeToReadTransitionsAtEviction++;
         if((index>=SRAM_ways) && (index<m_associativity) && (read_array[index]<(sf*write_array[index])))   //STTRAM ways, predicted read intensive
            readToWriteTransitionsAtEviction++;

         histogram[access_counter[index]]++;
         access_counter[index] = 0;

         write_array[index] = 0;  //reset the counters on eviction
         read_array[index] = 0;
         m_deadblock[index] = 0;
               
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);

         if(m_dcnt[m_TI[index]] != 255)
            m_dcnt[m_TI[index]]++;  //increment deadblock counter on eviction

         return index;
         
      } 
        
   }

   else //non-sampler set, uses phc
   {  
      //printf("non-sampler\n");
      if((set_index % sampler_fraction)==3)  //sampler set for calculating misses on current threshold
      {
         M0++;
      }

      //finding out invalid blocks
      if(m_state[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      { 
         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               m_TI[i]=eip_truncated;
               m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

               

               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               access_counter[i] = 0;
               m_deadblock[i] = 0;
               
               moveToMRU(i);
               //printf("invalid block found in sttram\n");
               return i;
            }
      
         }
      }
   
      else  //TI is hot. select victim from SRAM
      {
         for (UInt32 i = 0; i < SRAM_ways; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            { 
               m_TI[i]=eip_truncated;
               m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

               
               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               access_counter[i] = 0;
               m_deadblock[i] = 0;
               
               moveToMRU(i);
               //printf("invalid block found in sram\n");
               return i;
            }
         }
      }
      
      //trying to find an invalid block if it is present but not in the proper partition
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (!m_cache_block_info_array[i]->isValid())
         { 
            m_TI[i]=eip_truncated;
            m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

            
            write_array[i] = 0;  //reset the counters on eviction
            read_array[i] = 0;
            access_counter[i] = 0;
            m_deadblock[i] = 0;
               
            moveToMRU(i);
            //printf("invalid block found, but not in proper partition\n");
            return i;
         }
      }
      
      //INVALID BLOCK NOT FOUND
      // Make m_num_attemps attempts at evicting the block at LRU position
      for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt)
      {
         UInt32 index = 0;
         UInt8 max_bits = 0;

         //printf("invalid block not found\n");

         if(m_state[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
         {
            for (UInt32 i = SRAM_ways; i < m_associativity; i++)
            {
               if (m_lru_bits[i] > max_bits && isValidReplacement(i))
               {
                  index = i;
                  max_bits = m_lru_bits[i];
               }
            }
         }
   
         else  //TI is hot. select victim from SRAM
         {   
            for (UInt32 i = 0; i < SRAM_ways; i++)
            {
               if (m_lru_bits[i] > max_bits && isValidReplacement(i))
               {
                  index = i;
                  max_bits = m_lru_bits[i];
               }
            }
            migrate(index);
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");
   
            
         if((m_cost[index]>K) && (m_state[m_TI[index]]<state_max)) //state should be incremented
            m_state[m_TI[index]]++;
         else if((m_cost[index]<K) && (m_state[m_TI[index]]>0))
            m_state[m_TI[index]]--;

         //printf("eviction\n");
         
         
         //check if the victim block is read intense, or write intense or deadblock

         if(m_dcnt[m_TI[index]]>=dcnt_threshold)   //deadblock
            deadblock_counter++;
         else if((m_dcnt[m_TI[index]]<dcnt_threshold)&&(m_cost[index]<K))  //read_intense
            read_intense_block_counter++;
         else if((m_dcnt[m_TI[index]]<dcnt_threshold)&&(m_cost[index]>=K))  //write_intense
            write_intense_block_counter++;
         else
            printf("ERROR!!!!!!\n");



         C[Clength]=m_cost[index];
         Clength++;

         //Calculating Cmax and Cmin
         //printf("m_cost[index] is %d and Cmin is %d\n", m_cost[index], Cmin);
         if(m_cost[index]<Cmin)
            Cmin = m_cost[index];
         if(m_cost[index]>Cmax)
            Cmax = m_cost[index];
         
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

         if((index>=0) && (index<SRAM_ways) && (read_array[index]>(sf*write_array[index])))  //SRAM ways. Predicted Write intensive
            writeToReadTransitionsAtEviction++;
         if((index>=SRAM_ways) && (index<m_associativity) && (read_array[index]<(sf*write_array[index])))   //STTRAM ways, predicted read intensive
            readToWriteTransitionsAtEviction++;

         histogram[access_counter[index]]++;
         access_counter[index] = 0;

         write_array[index] = 0;  //reset the counters on eviction
         read_array[index] = 0;
         m_deadblock[index] = 0;
             
         // Mark our newly-inserted line as most-recently used
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt); 

         if(m_dcnt[m_TI[index]] != 255)
            m_dcnt[m_TI[index]]++;  //increment deadblock counter on eviction
       

         return index;
         
      } 

   }

   LOG_PRINT_ERROR("Should not reach here");

}


void
CacheSetPHC::updateReplacementIndex(UInt32 accessed_index, UInt8 write_flag, UInt32 set_index)
{
   m_set_info->increment(m_lru_bits[accessed_index]);
   moveToMRU(accessed_index);
   access_counter[accessed_index]++;   //number of accesses to a block

   if(m_dcnt[m_TI[accessed_index]] != 0)
      m_dcnt[m_TI[accessed_index]]--;  //not a deadblock if hit

   m_deadblock[accessed_index] = 1;    //Newton's method

   if(write_flag==1)
   {
      m_cost[accessed_index]=m_cost[accessed_index]+Ew;  //cost modification
      write_array[accessed_index]++;   //write_array is the number of writes to a block
   }
   else if(write_flag==0)
   {
      m_cost[accessed_index]=m_cost[accessed_index]-Er;
      read_array[accessed_index]++;    //read_array is the number of reads to a block
   }
   else
      printf("error: value of write_flag is %d \n", write_flag); 


   if(access_counter[accessed_index]==N_transition)  //after N accesses check if a previosuly predicted read intensive block has changed to write intensive block or not
   {
      if((accessed_index>=0) && (accessed_index<SRAM_ways) && (read_array[accessed_index]>(sf*write_array[accessed_index])))  //SRAM partition. predicted write intensive by PHC
      {
         writeToReadTransitionsAtInterval++;
      }
      if((accessed_index>=SRAM_ways) && (accessed_index<m_associativity) && (read_array[accessed_index]<(sf*write_array[accessed_index])))   //STTRAM. Read intensive
      {
         readToWriteTransitionsAtInterval++;
      }

      access_counter[accessed_index] = 0;
   }

}


//////////////////////////////////////////////////////////////////////////////////////////////
//created by arindam to pass writeback information to policy files (required in phc)
void
CacheSetPHC::updateReplacementIndex2(UInt32 accessed_index, UInt32 set_index)
{
   
   if(asl2_flag==0)
   {
      printf("\naccessSingleLine2 is called\n");
      asl2_flag = 1; 
   }
   
   if ((accessed_index>=0) && (accessed_index<m_associativity))
   {
      m_deadblock[accessed_index] = 1;    //Newton's method

      if(m_dcnt[m_TI[accessed_index]] != 0)
         m_dcnt[m_TI[accessed_index]]--;  //not a deadblock if hit

      access_counter[accessed_index]++;

      m_cost[accessed_index]=m_cost[accessed_index]+Ew;  //cost modification
      write_array[accessed_index]++;

      if(access_counter[accessed_index]==N_transition)  //after N accesses check if a previosuly predicted read intensive block has changed to write intensive block or not
      {
         if((accessed_index>=0) && (accessed_index<SRAM_ways) && (read_array[accessed_index]>(sf*write_array[accessed_index])))  //SRAM partition. predicted write intensive by PHC
         {
            writeToReadTransitionsAtInterval++;
         }
         if((accessed_index>=SRAM_ways) && (accessed_index<m_associativity) && (read_array[accessed_index]<(sf*write_array[accessed_index])))   //STTRAM. Predicted Read intensive
         {
            readToWriteTransitionsAtInterval++;
         }
   
         access_counter[accessed_index] = 0;
      }

   }
   else 
   {
      //printf("\nERROR!! accessed_index is %d", accessed_index);
   }
   

}
///////////////////////////////////////////////////////////////////////////////////////////////


void
CacheSetPHC::moveToMRU(UInt32 accessed_index)
{
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (m_lru_bits[i] < m_lru_bits[accessed_index])
         m_lru_bits[i] ++;
   }
   m_lru_bits[accessed_index] = 0;
}

UInt16 
CacheSetPHC::truncatedEipCalculation(IntPtr a)  //hashing
{
   UInt16 temp3, temp2; 
   temp3 = a % 256;
   for(int ii = 0; ii<4; ii++)
   {
       a = a >> 8;
       temp2 = a % 256;
       temp3 = temp3 ^ temp2;
   }
   return temp3;

}


void
CacheSetPHC::migrate(UInt32 sram_index)
{
	UInt32 stt_index = -1;
	UInt8 local_max_bits = 0;
	CacheBlockInfo* temp_cache_block_info = CacheBlockInfo::create(CacheBase::SHARED_CACHE);

   UInt8 temp_lru_bits = 0;
   UInt16 temp_TI = 0;
   UInt8 temp_cost = 0;
   UInt16 temp_write_array = 0;
   UInt16 temp_read_array = 0;
   UInt16 temp_access_counter = 0;
   UInt8 temp_deadblock = 0;
	
	if(m_dcnt[m_TI[sram_index]]<dcnt_threshold)     //PC based deadblock prediction
   //if(m_deadblock[sram_index]!=0)                 //Newton's deadblock prediction
   {
      //find stt_index
      for (UInt32 i = SRAM_ways; i < m_associativity; i++)
      {
         if (m_lru_bits[i] > local_max_bits && isValidReplacement(i))
         {
            stt_index = i;
            local_max_bits = m_lru_bits[i];
         }
      }
      
      if((stt_index>=SRAM_ways) && (stt_index<m_associativity))
      //swapping the blocks with index as stt_index and sram_index   
      {
         temp_cache_block_info->clone(m_cache_block_info_array[stt_index]);
         m_cache_block_info_array[stt_index]->clone(m_cache_block_info_array[sram_index]);
         m_cache_block_info_array[sram_index]->clone(temp_cache_block_info);

         temp_lru_bits = m_lru_bits[sram_index];
         temp_TI = m_TI[sram_index]; 
         temp_cost = m_cost[sram_index]; 
         temp_write_array = write_array[sram_index];
         temp_read_array = read_array[sram_index];
         temp_access_counter = access_counter[sram_index];
         temp_deadblock = m_deadblock[sram_index];

         m_lru_bits[sram_index] = m_lru_bits[stt_index];
         m_TI[sram_index] = m_TI[stt_index]; 
         m_cost[sram_index] = m_cost[stt_index]; 
         write_array[sram_index] = write_array[stt_index];
         read_array[sram_index] = read_array[stt_index];
         access_counter[sram_index] = access_counter[stt_index];
         m_deadblock[sram_index] = m_deadblock[stt_index];

         m_lru_bits[stt_index] = temp_lru_bits;
         m_TI[stt_index] = temp_TI; 
         m_cost[stt_index] = temp_cost; 
         write_array[stt_index] = temp_write_array;
         read_array[stt_index] = temp_read_array;
         access_counter[stt_index] = temp_access_counter;
         m_deadblock[stt_index] = temp_deadblock;

         //migrate_flag = 1; //This will be cross checked in cache controller inside insertCacheBlock function, to account for penalty
      }

   }
   else
   {

   }
	
	
}


