#include "cache_set_phc.h"
#include "log.h"
#include "stats.h"


///////// required for dynamic cost change/////////////
UInt8 CplusK = 255;						//C+(K0)
UInt8 CminusK = 0;						//C-(K0)
UInt8 CplusKplus = 255;					//C+(K+)
UInt8 CplusKminus = 255;				//C+(K-)
UInt8 CminusKplus = 0;					//C-(K+)
UInt8 CminusKminus = 0;					//C-(K-)

static UInt16 Mplus = 0;               //counters. Same as M0, M+ and M- in paper
static UInt16 Mminus = 0;
static UInt16 M0 = 0;

static UInt8 C[4096] = {0};            //evicted Cost set for K0
static UInt16 Clength = 0;             //length of Cost set for K0
static UInt8 Cplus[4096] = {0};        //evicted Cost set for K+
static UInt16 Cpluslength = 0;         //length of Cost set for K+
static UInt8 Cminus[4096] = {0};       //evicted Cost set for K-
static UInt16 Cminuslength = 0;        //length of Cost set for K-

UInt8 K = 148;                         //thresholds. Same as k, K+ and K- in paper
UInt8 Kplus = 149;
UInt8 Kminus = 147;


///////// required for static cost change////////////////

static UInt16 threshold_plus_miss_counter = 0;
static UInt16 threshold_minus_miss_counter = 0;
static UInt16 threshold_miss_counter = 0;

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

static UInt8 m_state[256] = {0};             //state table used by sampler set 3-63
static UInt8 m_state_plus[256] = {0};        //state table used by sampler set 1
static UInt8 m_state_minus[256] = {0};       //state table used by sampler set 2


static UInt16 lru_miss_counter = 0;

static UInt16 totalCacheMissCounter = 0;
static UInt16 totalCacheMissCounter_saturation = 4095;

static UInt64 write_transition_counter = 0;
static UInt64 read_transition_counter = 0;
static UInt64 write_threshold = 10;
static UInt64 read_threshold = 10;

static UInt64 total_write_intense_blocks = 0;
static UInt64 total_read_intense_blocks = 0;
static UInt64 total_non_write_intense_blocks = 0;
static UInt64 total_non_read_intense_blocks = 0;



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

   prev_write_array = new UInt16[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      prev_write_array[i] = 0;
   }

   prev_read_array = new UInt16[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      prev_read_array[i] = 0;
   }
   ///////////////////////////////////////////////////////
}



CacheSetPHC::~CacheSetPHC()
{
   delete [] m_lru_bits;
   delete [] m_TI;
   delete [] m_cost;
   delete [] write_array;
   delete [] prev_write_array;
   delete [] read_array;
   delete [] prev_read_array;

}


UInt32
CacheSetPHC::getReplacementIndex(CacheCntlr *cntlr, IntPtr eip, UInt32 set_index)   //implements dynamic threshold;both sampler and non sampler has cost;overhead is high; DYNAMIC COST CHANGE
{
   totalCacheMissCounter++;
   if (totalCacheMissCounter == totalCacheMissCounter_saturation) //phase change
   {
   	  //sorting the array
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


      for(int j=1; j<Clength; j++)           //why starting from j=1?? If we get threshold match at K=0, we cant put CminusK as C[-1]
      {
         if(C[0]>K)                          //all members are greater than K
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
      
      Clength = 0;
      Cpluslength = 0;
      Cminuslength = 0;

      for (int j=0; j<4096; j++)
      {
         C[j]=0;
         Cplus[j]=0;
         Cminus[j]=0;
      }


      
   }
   UInt16 eip_truncated=eip%256;  //truncating eip to last 12bits

   if ((set_index % sampler_fraction)==0) //sampler set, uses lru replacement
   {
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (!m_cache_block_info_array[i]->isValid())
         {
            // Mark our newly-inserted line as most-recently used
   
            m_TI[i]=eip_truncated;
            m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

            if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
               total_write_intense_blocks++;
            else
               total_non_write_intense_blocks++;

            if(read_array[i]>read_threshold)
               total_read_intense_blocks++;
            else
               total_non_read_intense_blocks++;


            write_array[i] = 0;  //reset the counters on eviction
            read_array[i] = 0;
            prev_write_array[i] = 0;
            prev_read_array[i] = 0;
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

         if(write_array[index]>write_threshold)     //counting total read intense/ write intense blocks
            total_write_intense_blocks++;
         else
            total_non_write_intense_blocks++;

         if(read_array[index]>read_threshold)
            total_read_intense_blocks++;
         else
            total_non_read_intense_blocks++;

         write_array[index] = 0;  //reset the counters on eviction
         read_array[index] = 0;
         prev_write_array[index] = 0;
         prev_read_array[index] = 0;

         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);
         
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

               if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
                  total_write_intense_blocks++;
               else
                  total_non_write_intense_blocks++;

               if(read_array[i]>read_threshold)
                  total_read_intense_blocks++;
               else
                  total_non_read_intense_blocks++;

               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               prev_write_array[i] = 0;
               prev_read_array[i] = 0;

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

               if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
                  total_write_intense_blocks++;
               else
                  total_non_write_intense_blocks++;

               if(read_array[i]>read_threshold)
                  total_read_intense_blocks++;
               else
                  total_non_read_intense_blocks++;

               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               prev_write_array[i] = 0;
               prev_read_array[i] = 0;

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

            if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
               total_write_intense_blocks++;
            else
               total_non_write_intense_blocks++;

            if(read_array[i]>read_threshold)
               total_read_intense_blocks++;
            else
               total_non_read_intense_blocks++;

            write_array[i] = 0;  //reset the counters on eviction
            read_array[i] = 0;
            prev_write_array[i] = 0;
            prev_read_array[i] = 0;

               
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
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");
   
      
         // Mark our newly-inserted line as most-recently used
         if((m_cost[index]>Kplus) && (m_state_plus[m_TI[index]]<state_max)) //state should be incremented
            m_state_plus[m_TI[index]]++;
         else if((m_cost[index]<Kplus) && (m_state_plus[m_TI[index]]>0))
            m_state_plus[m_TI[index]]--;

         Cplus[Cpluslength]=m_cost[index];
         Cpluslength++;
         
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

         if(write_array[index]>write_threshold)     //counting total read intense/ write intense blocks
            total_write_intense_blocks++;
         else
            total_non_write_intense_blocks++;

         if(read_array[index]>read_threshold)
            total_read_intense_blocks++;
         else
            total_non_read_intense_blocks++;

         write_array[index] = 0;  //reset the counters on eviction
         read_array[index] = 0;
         prev_write_array[index] = 0;
         prev_read_array[index] = 0;

               
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);            
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

               if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
                  total_write_intense_blocks++;
               else
                  total_non_write_intense_blocks++;

               if(read_array[i]>read_threshold)
                  total_read_intense_blocks++;
               else
                  total_non_read_intense_blocks++;

               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               prev_write_array[i] = 0;
               prev_read_array[i] = 0;

               
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

               if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
                  total_write_intense_blocks++;
               else
                  total_non_write_intense_blocks++;

               if(read_array[i]>read_threshold)
                  total_read_intense_blocks++;
               else
                  total_non_read_intense_blocks++;

               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               prev_write_array[i] = 0;
               prev_read_array[i] = 0;

               
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

            if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
               total_write_intense_blocks++;
            else
               total_non_write_intense_blocks++;

            if(read_array[i]>read_threshold)
               total_read_intense_blocks++;
            else
               total_non_read_intense_blocks++;

            write_array[i] = 0;  //reset the counters on eviction
            read_array[i] = 0;
            prev_write_array[i] = 0;
            prev_read_array[i] = 0;

               
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
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");

         // Mark our newly-inserted line as most-recently used
         if((m_cost[index]>Kminus) && (m_state_minus[m_TI[index]]<state_max)) //state should be incremented
            m_state_minus[m_TI[index]]++;
         else if((m_cost[index]<Kminus) && (m_state_minus[m_TI[index]]>0))
            m_state_minus[m_TI[index]]--;

         Cminus[Cminuslength]=m_cost[index];
         Cminuslength++;
         
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

         if(write_array[index]>write_threshold)     //counting total read intense/ write intense blocks
            total_write_intense_blocks++;
         else
            total_non_write_intense_blocks++;

         if(read_array[index]>read_threshold)
            total_read_intense_blocks++;
         else
            total_non_read_intense_blocks++;

         write_array[index] = 0;  //reset the counters on eviction
         read_array[index] = 0;
         prev_write_array[index] = 0;
         prev_read_array[index] = 0;

               
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);            
         return index;
         
      } 
        
   }

   else //non-sampler set, uses phc
   {
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

               if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
                  total_write_intense_blocks++;
               else
                  total_non_write_intense_blocks++;

               if(read_array[i]>read_threshold)
                  total_read_intense_blocks++;
               else
                  total_non_read_intense_blocks++;

               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               prev_write_array[i] = 0;
               prev_read_array[i] = 0;

               
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

               if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
                  total_write_intense_blocks++;
               else
                  total_non_write_intense_blocks++;

               if(read_array[i]>read_threshold)
                  total_read_intense_blocks++;
               else
                  total_non_read_intense_blocks++;

               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               prev_write_array[i] = 0;
               prev_read_array[i] = 0;

               
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

            if(write_array[i]>write_threshold)     //counting total read intense/ write intense blocks
               total_write_intense_blocks++;
            else
               total_non_write_intense_blocks++;

            if(read_array[i]>read_threshold)
               total_read_intense_blocks++;
            else
               total_non_read_intense_blocks++;

            write_array[i] = 0;  //reset the counters on eviction
            read_array[i] = 0;
            prev_write_array[i] = 0;
            prev_read_array[i] = 0;

               
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
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");
   
            
         if((m_cost[index]>K) && (m_state[m_TI[index]]<state_max)) //state should be incremented
            m_state[m_TI[index]]++;
         else if((m_cost[index]<K) && (m_state[m_TI[index]]>0))
            m_state[m_TI[index]]--;

         C[Clength]=m_cost[index];
         Clength++;
         
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

         if(write_array[index]>write_threshold)     //counting total read intense/ write intense blocks
            total_write_intense_blocks++;
         else
            total_non_write_intense_blocks++;

         if(read_array[index]>read_threshold)
            total_read_intense_blocks++;
         else
            total_non_read_intense_blocks++;

         write_array[index] = 0;  //reset the counters on eviction
         read_array[index] = 0;
         prev_write_array[index] = 0;
         prev_read_array[index] = 0;

             
         // Mark our newly-inserted line as most-recently used
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);            
         return index;
         
      } 

   }

   LOG_PRINT_ERROR("Should not reach here");
}


/*
UInt32
CacheSetPHC::getReplacementIndex(CacheCntlr *cntlr, IntPtr eip, UInt32 set_index)   //implements dynamic threshold;both sampler and non sampler has cost;overhead is high; STATIC COST CHANGE
{
   totalCacheMissCounter++;
   if (totalCacheMissCounter == totalCacheMissCounter_saturation) //phase change
   {
      //printf("totalCacheMissCounter saturates. threshold_minus_miss_counter is %d, threshold_miss_counter is %d and threshold_plus_miss_counter is %d\n", threshold_minus_miss_counter, threshold_miss_counter, threshold_plus_miss_counter);
      if((threshold_minus_miss_counter<threshold_miss_counter) && (threshold_minus_miss_counter<threshold_plus_miss_counter)) //decrement threshold
      {
         //printf("decrement threshold\n");
         cost_threshold--;
         cost_threshold_plus--;
         cost_threshold_minus--;
      }
      else if((threshold_plus_miss_counter<threshold_miss_counter) && (threshold_plus_miss_counter<threshold_minus_miss_counter)) //increment threshold
      {
         //printf("increment threshold\n");
         cost_threshold++;
         cost_threshold_plus++;
         cost_threshold_minus++;
      }
      else if((threshold_miss_counter<threshold_plus_miss_counter) && (threshold_miss_counter<threshold_minus_miss_counter)) //unchanged threshold
      {
         //printf("unchanged threshold\n");
      }
      lru_miss_counter = 0;
      threshold_plus_miss_counter = 0;
      threshold_minus_miss_counter = 0;
      threshold_miss_counter = 0;
      totalCacheMissCounter = 0;
   }
   //printf("cost_threshold is %d, cost_threshold_plus is %d, cost_threshold_minus is %d\n",cost_threshold, cost_threshold_plus, cost_threshold_minus);
   //printf("\ngetReplacementIndex called in l3 \n");
   UInt16 eip_truncated=eip%256;  //truncating eip to last 12bits
   //printf("set_index mod sampler_fraction is %d\n",(set_index % sampler_fraction));

   if ((set_index % sampler_fraction)==0) //sampler set, uses lru replacement
   {
      //printf("\nsampler set \n");   //ns
      //finding out invalid blocks
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (!m_cache_block_info_array[i]->isValid())
         {
            //printf("invalid block found at index %d \n", i);
            // Mark our newly-inserted line as most-recently used
   
            m_TI[i]=eip_truncated;
            m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.
    
            moveToMRU(i);
            return i;
         }
      }

      //INVALID BLOCK NOT FOUND
      //printf("invalid block not found \n");
      // Make m_num_attemps attempts at evicting the block at LRU position
      for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt)
      {
         UInt32 index = 0;
         UInt8 max_bits = 0;

         //for (UInt32 i = 0; i < SRAM_ways; i++)
         for (UInt32 i = 0; i < m_associativity; i++)
         {
            if (m_lru_bits[i] > max_bits && isValidReplacement(i))
            {
               index = i;
               max_bits = m_lru_bits[i];
            }
         }

         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");
            
         // Mark our newly-inserted line as most-recently used
         if((m_cost[index]>cost_threshold) && (m_state[m_TI[index]]<state_max)) //state should be incremented
         {
            //printf("state of eip %d is %d and should be incremented \n", m_TI[index], m_state[m_TI[index]]);
            m_state[m_TI[index]]++;
         }
         else if((m_cost[index]<cost_threshold) && (m_state[m_TI[index]]>0))
         {
            //printf("state of eip %d is %d and should be decremented \n", m_TI[index], m_state[m_TI[index]]);
            m_state[m_TI[index]]--;
         }
         else
         {
            //printf("\n state of eip %d is %d and it is not changed \n", m_TI[index], m_state[m_TI[index]]);
         }
         //printf("new state of eip %d is %d \n",m_TI[index], m_state[m_TI[index]] );
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);
         return index;
         
      }   
      
   }

   else if ((set_index % sampler_fraction)==1)  //sampler set using cost_threshold_plus and m_state_plus
   {
      //printf("threshold_plus_miss_counter is %d, threshold_minus_miss_counter is %d, threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter, threshold_miss_counter);
      threshold_plus_miss_counter++;
      //printf("increment threshold_plus_miss_counter\n");
      
      //finding out invalid blocks
      if(m_state_plus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {
         
         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               m_TI[i]=eip_truncated;
               m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.
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
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");
   
         
         // Mark our newly-inserted line as most-recently used
         if((m_cost[index]>cost_threshold_plus) && (m_state_plus[m_TI[index]]<state_max)) //state should be incremented
            m_state_plus[m_TI[index]]++;
         else if((m_cost[index]<cost_threshold_plus) && (m_state_plus[m_TI[index]]>0))
            m_state_plus[m_TI[index]]--;
         
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);            
         return index;
         
      }   
      
   }

   else if ((set_index % sampler_fraction)==2)  //sampler set using cost_threshold_minus and m_state_minus
   {
      //printf("threshold_plus_miss_counter is %d, threshold_minus_miss_counter is %d, threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter, threshold_miss_counter);
      threshold_minus_miss_counter++;
      //printf("increment threshold_minus_miss_counter\n");

      //finding out invalid blocks
      if(m_state_minus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {
         
         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               m_TI[i]=eip_truncated;
               m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.
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
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");

         // Mark our newly-inserted line as most-recently used
         if((m_cost[index]>cost_threshold_minus) && (m_state_minus[m_TI[index]]<state_max)) //state should be incremented
            m_state_minus[m_TI[index]]++;
         else if((m_cost[index]<cost_threshold_minus) && (m_state_minus[m_TI[index]]>0))
            m_state_minus[m_TI[index]]--;
         
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);            
         return index;
         
      } 
        
   }

   else //non-sampler set, uses phc
   {
      //printf("\ncost threshold is %d\n", cost_threshold);
      if((set_index % sampler_fraction)==3)  //sampler set for calculating misses on current threshold
      {
         //printf("threshold_plus_miss_counter is %d, threshold_minus_miss_counter is %d, threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter, threshold_miss_counter);
         threshold_miss_counter++;
         //printf("increment threshold_miss_counter\n");
      }
      //printf("\nnon-smplr set \n");   //ns
      //finding out invalid blocks
      if(m_state[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {
         
         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               m_TI[i]=eip_truncated;
               m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.
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
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");
   
            
         if((m_cost[index]>cost_threshold) && (m_state[m_TI[index]]<state_max)) //state should be incremented
            m_state[m_TI[index]]++;
         else if((m_cost[index]<cost_threshold) && (m_state[m_TI[index]]>0))
            m_state[m_TI[index]]--;
         
         m_TI[index]=eip_truncated;
         m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.
         // Mark our newly-inserted line as most-recently used
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);            
         return index;
         
      } 

   }

   LOG_PRINT_ERROR("Should not reach here");                                                                                //invalid blocks dont change state of TI on eviction
}
*/




void
CacheSetPHC::updateReplacementIndex(UInt32 accessed_index, UInt8 write_flag, UInt32 set_index)
{
   m_set_info->increment(m_lru_bits[accessed_index]);
   moveToMRU(accessed_index);


   if(write_flag==1)
   {
      //printf("write hit!! prev value of write array was %d\n", write_array[accessed_index]);
      m_cost[accessed_index]=m_cost[accessed_index]+Ew;  //cost modification
      prev_write_array[accessed_index] = write_array[accessed_index];   //for checking transition from write intense to non write intense or vice versa
      write_array[accessed_index]++;
      //printf("current value of write array is %d\n", write_array[accessed_index]);

      if (  (prev_write_array[accessed_index] == write_threshold) &&
            ((write_array[accessed_index]==(write_threshold+1)))
         )
      {
          write_transition_counter++;
          //printf("write transition occurs\n");
          
      }


   }
   else if(write_flag==0)
   {
      //printf("read hit!! prev value of read array was %d\n", read_array[accessed_index]);
      m_cost[accessed_index]=m_cost[accessed_index]-Er;
      prev_read_array[accessed_index] = read_array[accessed_index];     //for checking transition from read intense to non read intense or vice versa
      read_array[accessed_index]++;
      //printf("current value of read array is %d\n", read_array[accessed_index]);

      if (  (prev_read_array[accessed_index] == read_threshold) &&
            ((read_array[accessed_index]==(read_threshold+1)))
         )
      {
          read_transition_counter++;
          //printf("read transition occurs\n");
      }
   }
   else
      printf("error: value of write_flag is %d \n", write_flag);  
}

//////////////////////////////////////////////////////////////////////////////////////////////
//created by arindam to pass writeback information to policy files (required in phc)
void
CacheSetPHC::updateReplacementIndex2(UInt32 accessed_index, UInt32 set_index)
{   
    if ((accessed_index>=0) && (accessed_index<m_associativity))
    {
      //printf("write hit!! prev value of write array was %d\n", write_array[accessed_index]);
      m_cost[accessed_index]=m_cost[accessed_index]+Ew;  //cost modification
      prev_write_array[accessed_index] = write_array[accessed_index];   //for checking transition from write intense to non write intense or vice versa
      write_array[accessed_index]++;
      //printf("current value of write array is %d\n", write_array[accessed_index]);

      if (  (prev_write_array[accessed_index] == write_threshold) &&
            ((write_array[accessed_index]==(write_threshold+1)))
         )
      {
          write_transition_counter++;
          //printf("write transition occurs\n");
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


