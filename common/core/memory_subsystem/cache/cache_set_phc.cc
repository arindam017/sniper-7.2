#include "cache_set_phc.h"
#include "log.h"
#include "stats.h"


UInt8 cost_threshold = 20;
UInt8 Ew=24;
UInt8 Er=1;
UInt16 predictor_table_length=256;
UInt8 state_threshold=2;
UInt8 state_max=3;
UInt8 SRAM_ways=4;
UInt32 number_of_sets = 8192;
UInt32 sampler_fraction = 32;

extern UInt64 globalWritebacksToL3counter;   //this is a global counter. this counter will be reset when updateReplacementindex for phc in LLC is called 



static UInt8 m_state[256] = {0};


CacheSetPHC::CacheSetPHC(
      CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize, CacheSetInfoLRU* set_info, UInt8 num_attempts)
   : CacheSet(cache_type, associativity, blocksize)
   , m_num_attempts(num_attempts)
   , m_set_info(set_info)
{ 

   //printf("set number %d \n", counter);
   //counter++;

   m_lru_bits = new UInt8[m_associativity];

   for (UInt32 i = 0; i < m_associativity; i++)
      m_lru_bits[i] = i;

   m_TI = new UInt16[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
      m_TI[i] = 0; 

   m_cost = new UInt8[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      //printf("i is %d \n",i);
      m_cost[i] = 0; 
   }

}

CacheSetPHC::~CacheSetPHC()
{
   delete [] m_lru_bits;
   delete [] m_TI;
   delete [] m_cost;
}


UInt32
CacheSetPHC::getReplacementIndex(CacheCntlr *cntlr, IntPtr eip, UInt32 set_index)
{
   //printf("\ngetReplacementIndex called in l3 \n");
   UInt16 eip_truncated=eip%256;  //truncating eip to last 12bits

   if ((set_index % sampler_fraction)==0) //sampler set
   {
      //printf("\nsampler set \n");   //ns

      //finding out invalid blocks
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (!m_cache_block_info_array[i]->isValid())
         {
            //printf("invalid block found at index %d \n", i);
            // Mark our newly-inserted line as most-recently used
   
            if((m_cost[i]>cost_threshold) && (m_state[m_TI[i]]<state_max)) //state should be incremented
            {
               //printf("state of eip %d is %d and should be incremented \n", m_TI[i], m_state[m_TI[i]]);
               m_state[m_TI[i]]++;
            }
            else if((m_cost[i]<cost_threshold) && (m_state[m_TI[i]]>0))
            {
               //printf("state of eip %d is %d and should be decremented \n", m_TI[i], m_state[m_TI[i]]);
               m_state[m_TI[i]]--;
            }
            else 
            {
               //printf("state of eip %d is not changed \n", m_TI[i]);
            }
   
            //printf("new state of eip %d is %d \n",m_TI[i], m_state[m_TI[i]] );
   
            m_TI[i]=eip_truncated;
            m_cost[i]=0;
    
            moveToMRU(i);

            //printf("\n cost is \n");
            for (UInt32 i=0; i< m_associativity; i++)
            {
               //printf("%d ", m_cost[i]);
            }
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
   
         bool qbs_reject = false;
         if (attempt < m_num_attempts - 1)
         {
            LOG_ASSERT_ERROR(cntlr != NULL, "CacheCntlr == NULL, QBS can only be used when cntlr is passed in");
            qbs_reject = cntlr->isInLowerLevelCache(m_cache_block_info_array[index]);
         }
         if (qbs_reject)
         {
            // Block is contained in lower-level cache, and we have more tries remaining.
            // Move this block to MRU and try again
            moveToMRU(index);
            cntlr->incrementQBSLookupCost();
            continue;
         }
         else
         {
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
            m_cost[index]=0;
   
            moveToMRU(index);
            m_set_info->incrementAttempt(attempt);

            //printf("\n cost is \n");
            for (UInt32 i=0; i< m_associativity; i++)
            {
               //printf("%d ", m_cost[i]);
            }
            
            return index;
         }
      }   
      
   }
   else  //non-sampler set
   {
      //printf("\nnon-smplr set \n");   //ns
      //finding invalid blocks
      //printf("state of eip %d is %d \n", eip_truncated, m_state[eip_truncated]); //ns
      if(m_state[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {
         
         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               //printf("invalid block found in sttam and its index is %d \n", i);   //ns 
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
               //printf("invalid block found in sram and its index is %d \n", i);   //ns
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
            //printf("invalid block found but not in proper partition and its index is %d \n", i);  //ns
            moveToMRU(i);
            return i;
         }
      }

      //INVALID BLOCK NOT FOUND
      //printf("invalid block not found \n");  //ns

      //Make m_num_attemps attempts at evicting the block at LRU position
      for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt)
      {
         UInt32 index = 0;
         UInt8 max_bits = 0;
   
         if(m_state[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
         {
            //printf("victim selected from sttram \n"); //ns
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
            //printf("victim selected from sram \n"); //ns           
            for (UInt32 i = 0; i < SRAM_ways; i++)
            {
               if (m_lru_bits[i] > max_bits && isValidReplacement(i))
               {
                  index = i;
                  max_bits = m_lru_bits[i];
               }
            }
         }
   
         //index already decided at this point
         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");
   
         bool qbs_reject = false;
         if (attempt < m_num_attempts - 1)
         {
            LOG_ASSERT_ERROR(cntlr != NULL, "CacheCntlr == NULL, QBS can only be used when cntlr is passed in");
            qbs_reject = cntlr->isInLowerLevelCache(m_cache_block_info_array[index]);
         }
         if (qbs_reject)
         {
            // Block is contained in lower-level cache, and we have more tries remaining.
            // Move this block to MRU and try again
            moveToMRU(index);
            cntlr->incrementQBSLookupCost();
            continue;
         }
         else
         {  
            // Mark our newly-inserted line as most-recently used
            moveToMRU(index);
            m_set_info->incrementAttempt(attempt);

            //printf("index of victim is %d", index);
            return index;
         }
      }

   }

   LOG_PRINT_ERROR("Should not reach here");
}



void
CacheSetPHC::updateReplacementIndex(UInt32 accessed_index, UInt8 write_flag, UInt32 set_index)
{
   //printf("\nupdateReplacementIndex called in l3. set index is %d and write flag is %d \n", set_index, write_flag);
   m_set_info->increment(m_lru_bits[accessed_index]);
   moveToMRU(accessed_index);

   if ((set_index % sampler_fraction)==0) //sampler sets
   {
      //printf("\nsampler set (%d)\n", set_index);   //ns
      if(write_flag==1)
      {
         //printf("write hit!!\naccessed index is %d", accessed_index);

         //printf("\nprevious cost is \n");
         for (UInt32 i=0; i< m_associativity; i++)
         {
            //printf("%d ", m_cost[i]);
         }
         //printf("\n");

         m_cost[accessed_index]=m_cost[accessed_index]+Ew;  //cost modification


         //printf("new cost is \n");
         for (UInt32 i=0; i< m_associativity; i++)
         {
            //printf("%d ", m_cost[i]);
         }
         //printf("\n");
      }
      else if(write_flag==0)
      {
         //printf("read hit!!\naccessed index is %d", accessed_index);

         //printf("\nprevious cost is \n");
         for (UInt32 i=0; i< m_associativity; i++)
         {
            //printf("%d ", m_cost[i]);
         }
         //printf("\n");


         m_cost[accessed_index]=m_cost[accessed_index]-Er;


         //printf("new cost is \n");
         for (UInt32 i=0; i< m_associativity; i++)
         {
            //printf("%d ", m_cost[i]);
         }
         //printf("\n");
      }
      else
         printf("error: value of write_flag is %d \n", write_flag);
   }
   
}

//////////////////////////////////////////////////////////////////////////////////////////////
//created by arindam to pass writeback information to policy files (required in phc)
void
CacheSetPHC::updateReplacementIndex2(UInt32 accessed_index, UInt32 set_index)
{
   
   if ((set_index % sampler_fraction)==0)    //sampler sets
   {
      //printf("\nsampler set, called from writeback, ");
      //printf("accessed index is %d", accessed_index);
   
      //printf("\nprevious cost is \n");
      for (UInt32 i=0; i< m_associativity; i++)
      {
         //printf("%d ", m_cost[i]);
      }
      //printf("\n");    
      m_cost[accessed_index]=m_cost[accessed_index]+Ew;  //cost modification
      //printf("new cost is \n");
      for (UInt32 i=0; i< m_associativity; i++)
      {
         //printf("%d ", m_cost[i]);
      }
      //printf("\n");

   
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


