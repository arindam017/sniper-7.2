#include "cache_set_phc.h"
#include "log.h"
#include "stats.h"

// Implements PHC paper IEEE TC 2016
// Prediction Hybrid Cache: An Energy-Efficient STT-RAM Cache Architecture

/* Steps to be followed to implement PHC
 * 1. Every block in the set will store information about PC which inserted
      the block in the cache. This will be done using getReplacementIndex()
 * 2. Every block will also store a cost variable to maintain the cost (write/read)
      of the block.
      Cost will be updated on every read and write operation.
        updateReplacementIndex() function can be do this.
 * 3. On eviction of a cache block, we need to see its cost. If the blocks cost
      is more than a threshold, then the PC which brought the block to cache is
      marked as Hot Triggering Instruction (HTI). TI here is basically PC. The
      status of all the sampled TI blocks is stored in a prediction_table.
 * 4. On insertion of a new cache block, it is checked that if the cache block
      has been brought by a HTI. If so, the block is assumed to be write-intesive
      and should be inserted to the SRAM part of the hybrid cache. Else the block
      should be inserted to the STTRAM part. If all the SRAM/STTRAM ways are full
      a block is evicted from the corresponding part using LRU policy.

      TODO: Which blocks should participate in determining the LRU stack is not
      clearyly mentioned in PHC paper. Are there two LRU stacks one each for SRAM
      and STTRAM ways?
 * 5. Actions need to be performed on Insertion (write placement), Eviction (Checking
      if the TI is HTI), and Updation (block's cost) of the cache block.
 */

/* Steps to implement dynamic HPC
 * Aim: To dynamically adjust the threshold as per the phase of the application
 * 1. Hardware: 6 16 bit counters, one each for measureing M_0, M_RLU, M_+, M_-
 *    partial-tag-array, predictor table, one each for M_+, M_-
 *    6 8-bit counters to measure cost of evicted block from partial-tag-arrays
 *    with cost threshold k_0, k_+, k_-. Both least and max cost has to be measured
 *    for each k_n
 * 2. M_0: number of misses with hybrid cache using threshold k_0
 *    M_LRU: # of misses in the sampler
 *    M_+: # of misses in the partial-tag-arrary with threshold k_+
 *    M_-: # of misses in the partial-tag-arrary with threshold k_-
 * 3. For start k_+ = k_0 + 1 and k_- = k_0 - 1
 * TODO: Need to discuss. In my understanding k_- = c_-(k_n) = max of C_- for all k_n
 */

UInt8 Ew=24; /* write cost */
UInt8 Er=-1; /* read cost */
UInt16 predictor_table_length=256;
UInt8 state_threshold=2;
UInt8 state_max=3; //a cache block can have 0-3 states. 0,1 'cold' and 2,3 hold state
UInt8 SRAM_ways=4; //number of ways for SRAM. This we should take from configuration
//UInt32 counter=0;
UInt32 number_of_sets = 8192;
UInt32 sampler_fraction = 32; //number of sets sampled

UInt8 cost_threshold = 148;

UInt8 cost_threshold_plus = 149;
UInt8 cost_threshold_minus = 147;
//in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me. So threshold of 20 translates to 148.

///////// required for dynamic cost change/////////////
UInt8 highestBelowThresholdCost = 0;
UInt8 lowestAboveThresholdCost = 255;
UInt8 highestBelowThresholdCostPlus = 0;  //C-K+
UInt8 lowestAboveThresholdCostPlus = 255; //C+K+
UInt8 highestBelowThresholdCostMinus = 0; //C-K-
UInt8 lowestAboveThresholdCostMinus = 255;   //C+K-
//////////////////////////////////////////////////////

extern UInt64 globalWritebacksToL3counter;
//this is a global counter. this counter will be reset when updateReplacementindex for phc in LLC is called

//State contains information about status of each Trigging instruction (TI).
//State of the TI can be hot/cold based on the cost of the evicted block marked with TI
static UInt8 m_state[256] = {0};

static UInt8 m_state_plus[256] = {0};
static UInt8 m_state_minus[256] = {0};

static UInt16 lru_miss_counter = 0;
static UInt16 threshold_plus_miss_counter = 0;
static UInt16 threshold_minus_miss_counter = 0;
static UInt16 threshold_miss_counter = 0;
static UInt16 totalCacheMissCounter = 0;
static UInt16 totalCacheMissCounter_saturation = 4095;


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

   //To capture information about triggering instructions (instruction which
   //resulted in bringing of the cache block to the cache on miss) PC of the
   //instruction will be stored
   m_TI = new UInt16[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
      m_TI[i] = 0;

   //each block will have a write-cost to determine if the block is write-intensive
   m_cost = new UInt8[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      //printf("i is %d \n",i);
     m_cost[i] = 128;  //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.
   }
}

CacheSetPHC::~CacheSetPHC()
{
   delete [] m_lru_bits;
   delete [] m_TI;
   delete [] m_cost;
}


/*
UInt32
CacheSetPHC::getReplacementIndex(CacheCntlr *cntlr, IntPtr eip, UInt32 set_index)   //does not implement dynamic threshold
{
   //printf("\ngetReplacementIndex called in l3 \n");
   UInt16 eip_truncated=eip%256;  //truncating eip to last 12bits

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
            //TODO: If a block is invalid why to bother about TI and state
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
<<<<<<< HEAD
            m_cost[i]=24;

=======
            m_cost[i]=0;

>>>>>>> dfb488184ed656c7c3a093c903ca70221af0c964
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
<<<<<<< HEAD
            m_cost[index]=24;

=======
            m_cost[index]=0;

>>>>>>> dfb488184ed656c7c3a093c903ca70221af0c964
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
   else  //non-sampler set, uses phc
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
<<<<<<< HEAD
               //printf("invalid block found in sttam \n");   //ns
=======
               //printf("invalid block found in sttam and its index is %d \n", i);   //ns
>>>>>>> dfb488184ed656c7c3a093c903ca70221af0c964
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
<<<<<<< HEAD
            {
               //printf("invalid block found in sram \n");   //ns
=======
            {
               //printf("invalid block found in sram and its index is %d \n", i);   //ns
>>>>>>> dfb488184ed656c7c3a093c903ca70221af0c964
               moveToMRU(i);
               return i;
            }
         }
      }

      //trying to find an invalid block if it is present but not in the proper partition
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (!m_cache_block_info_array[i]->isValid())
<<<<<<< HEAD
         {
            //printf("invalid block found but not in proper partition \n");  //ns
           moveToMRU(i);
=======
         {
            //printf("invalid block found but not in proper partition and its index is %d \n", i);  //ns
            moveToMRU(i);
>>>>>>> dfb488184ed656c7c3a093c903ca70221af0c964
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
*/

/*
UInt32
CacheSetPHC::getReplacementIndex(CacheCntlr *cntlr, IntPtr eip, UInt32 set_index)   //implements dynamic threshold; only sampler has cost
{
   //printf("\ngetReplacementIndex called in l3 \n");
   UInt16 eip_truncated=eip%256;  //truncating eip to last 12bits
   //printf("set_index mod sampler_fraction is %d\n",(set_index % sampler_fraction));

   if ((set_index % sampler_fraction)==0) //sampler set, uses lru replacement
   {
      //printf("\nsampler set \n");   //ns
      printf("cost_threshold is %d, cost_threshold_plus is %d, cost_threshold_minus is %d\n",cost_threshold, cost_threshold_plus, cost_threshold_minus);
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

   else if ((set_index % sampler_fraction)==1)  //sampler set using cost_threshold_plus and m_state_plus
   {
      //printf("threshold_plus_miss_counter is %d, threshold_minus_miss_counter is %d, threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter, threshold_miss_counter);
      threshold_plus_miss_counter++;
      if (totalCacheMissCounter == totalCacheMissCounter_saturation) //phase change
      {
         //printf("threshold_plus_miss_counter saturates. threshold_minus_miss_counter is %d and threshold_miss_counter is %d\n", threshold_minus_miss_counter, threshold_miss_counter);
         if(threshold_minus_miss_counter<threshold_miss_counter)  //decrement threshold
         {
            //printf("decrement threshold\n");
            cost_threshold--;
            cost_threshold_plus--;
            cost_threshold_minus--;
         }
         else
            //printf("unchnaged threshold\n");

         lru_miss_counter = 0;
         threshold_plus_miss_counter = 0;
         threshold_minus_miss_counter = 0;
         threshold_miss_counter = 0;
         totalCacheMissCounter = 0;
      }

      //finding out invalid blocks
      if(m_state_plus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {

         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               if((m_cost[i]>cost_threshold_plus) && (m_state_plus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_plus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_plus) && (m_state_plus[m_TI[i]]>0))
                  m_state_plus[m_TI[i]]--;

               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
               if((m_cost[i]>cost_threshold_plus) && (m_state_plus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_plus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_plus) && (m_state_plus[m_TI[i]]>0))
                  m_state_plus[m_TI[i]]--;

               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
            if((m_cost[i]>cost_threshold_plus) && (m_state_plus[m_TI[i]]<state_max)) //state should be incremented
               m_state_plus[m_TI[i]]++;
            else if((m_cost[i]<cost_threshold_plus) && (m_state_plus[m_TI[i]]>0))
               m_state_plus[m_TI[i]]--;

            m_TI[i]=eip_truncated;
            m_cost[i]=0;
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
            if((m_cost[index]>cost_threshold_plus) && (m_state_plus[m_TI[index]]<state_max)) //state should be incremented
               m_state_plus[m_TI[index]]++;
            else if((m_cost[index]<cost_threshold_plus) && (m_state_plus[m_TI[index]]>0))
               m_state_plus[m_TI[index]]--;


            m_TI[index]=eip_truncated;
            m_cost[index]=0;
            moveToMRU(index);
            m_set_info->incrementAttempt(attempt);
            return index;
         }
      }

   }
<<<<<<< HEAD
}
=======

   else if ((set_index % sampler_fraction)==2)  //sampler set using cost_threshold_minus and m_state_minus
   {
      //printf("threshold_plus_miss_counter is %d, threshold_minus_miss_counter is %d, threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter, threshold_miss_counter);
      threshold_minus_miss_counter++;
      if (totalCacheMissCounter == totalCacheMissCounter_saturation)
      {
         //printf("threshold_minus_miss_counter saturates. threshold_plus_miss_counter is %d and threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_miss_counter);
         if(threshold_plus_miss_counter<threshold_miss_counter)  //increment threshold
         {
            //printf("increment threshold\n");
            cost_threshold++;
            cost_threshold_plus++;
            cost_threshold_minus++;
         }
         else
            //printf("threshold unchanged\n");
         lru_miss_counter = 0;
         threshold_plus_miss_counter = 0;
         threshold_minus_miss_counter = 0;
         threshold_miss_counter = 0;
         totalCacheMissCounter = 0;
      }

      //finding out invalid blocks
      if(m_state_minus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {

         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               if((m_cost[i]>cost_threshold_minus) && (m_state_minus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_minus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_minus) && (m_state_minus[m_TI[i]]>0))
                  m_state_minus[m_TI[i]]--;

               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
               if((m_cost[i]>cost_threshold_minus) && (m_state_minus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_minus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_minus) && (m_state_minus[m_TI[i]]>0))
                  m_state_minus[m_TI[i]]--;

               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
            if((m_cost[i]>cost_threshold_minus) && (m_state_minus[m_TI[i]]<state_max)) //state should be incremented
               m_state_minus[m_TI[i]]++;
            else if((m_cost[i]<cost_threshold_minus) && (m_state_minus[m_TI[i]]>0))
               m_state_minus[m_TI[i]]--;

            m_TI[i]=eip_truncated;
            m_cost[i]=0;
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
            if((m_cost[index]>cost_threshold_minus) && (m_state_minus[m_TI[index]]<state_max)) //state should be incremented
               m_state_minus[m_TI[index]]++;
            else if((m_cost[index]<cost_threshold_minus) && (m_state_minus[m_TI[index]]>0))
               m_state_minus[m_TI[index]]--;


            m_TI[index]=eip_truncated;
            m_cost[index]=0;
            moveToMRU(index);
            m_set_info->incrementAttempt(attempt);
            return index;
         }
      }

   }

   else //non-sampler set, uses phc
   {
      if((set_index % sampler_fraction)==3)
      {
         //printf("threshold_plus_miss_counter is %d, threshold_minus_miss_counter is %d, threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter, threshold_miss_counter);
         threshold_miss_counter++;
         if (totalCacheMissCounter == totalCacheMissCounter_saturation)
         {
            //printf("threshold_miss_counter saturates. threshold_plus_miss_counter is %d and threshold_minus_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter);
            if(threshold_minus_miss_counter<threshold_plus_miss_counter)  //decrement threshold
            {
               //printf("decrement threshold\n");
               cost_threshold--;
               cost_threshold_plus--;
               cost_threshold_minus--;
            }
            else if (threshold_minus_miss_counter>threshold_plus_miss_counter)   //increment threshold
            {
               //printf("increment threshold\n");
               cost_threshold++;
               cost_threshold_plus++;
               cost_threshold_minus++;
            }
            else if (threshold_minus_miss_counter==threshold_plus_miss_counter)  //both are same, choose lower threshold to reduce write energy consumption
            {
               //printf("decrement threshold to reduce energy consumption\n");
               cost_threshold--;
               cost_threshold_plus--;
               cost_threshold_minus--;
            }
            lru_miss_counter = 0;
            threshold_plus_miss_counter = 0;
            threshold_minus_miss_counter = 0;
            threshold_miss_counter = 0;
            totalCacheMissCounter = 0;
         }
      }
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
*/


UInt32
CacheSetPHC::getReplacementIndex(CacheCntlr *cntlr, IntPtr eip, UInt32 set_index)   //implements dynamic threshold;both sampler and non sampler has cost;overhead is high; static cost change
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
            m_cost[i]=128; //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

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
            m_cost[index]=128;   //in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me.

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
               if((m_cost[i]>cost_threshold_plus) && (m_state_plus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_plus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_plus) && (m_state_plus[m_TI[i]]>0))
                  m_state_plus[m_TI[i]]--;

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
               if((m_cost[i]>cost_threshold_plus) && (m_state_plus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_plus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_plus) && (m_state_plus[m_TI[i]]>0))
                  m_state_plus[m_TI[i]]--;

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
            if((m_cost[i]>cost_threshold_plus) && (m_state_plus[m_TI[i]]<state_max)) //state should be incremented
               m_state_plus[m_TI[i]]++;
            else if((m_cost[i]<cost_threshold_plus) && (m_state_plus[m_TI[i]]>0))
               m_state_plus[m_TI[i]]--;

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
               if((m_cost[i]>cost_threshold_minus) && (m_state_minus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_minus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_minus) && (m_state_minus[m_TI[i]]>0))
                  m_state_minus[m_TI[i]]--;

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
               if((m_cost[i]>cost_threshold_minus) && (m_state_minus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_minus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_minus) && (m_state_minus[m_TI[i]]>0))
                  m_state_minus[m_TI[i]]--;

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
            if((m_cost[i]>cost_threshold_minus) && (m_state_minus[m_TI[i]]<state_max)) //state should be incremented
               m_state_minus[m_TI[i]]++;
            else if((m_cost[i]<cost_threshold_minus) && (m_state_minus[m_TI[i]]>0))
               m_state_minus[m_TI[i]]--;

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

   }

   else //non-sampler set, uses phc
   {
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
               if((m_cost[i]>cost_threshold) && (m_state[m_TI[i]]<state_max)) //state should be incremented
                  m_state[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold) && (m_state[m_TI[i]]>0))
                  m_state[m_TI[i]]--;

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
               if((m_cost[i]>cost_threshold) && (m_state[m_TI[i]]<state_max)) //state should be incremented
                  m_state[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold) && (m_state[m_TI[i]]>0))
                  m_state[m_TI[i]]--;

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
            if((m_cost[i]>cost_threshold) && (m_state[m_TI[i]]<state_max)) //state should be incremented
               m_state[m_TI[i]]++;
            else if((m_cost[i]<cost_threshold) && (m_state[m_TI[i]]>0))
               m_state[m_TI[i]]--;

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

   }

   LOG_PRINT_ERROR("Should not reach here");
}


/*
UInt32
CacheSetPHC::getReplacementIndex(CacheCntlr *cntlr, IntPtr eip, UInt32 set_index)   //implements dynamic threshold; both sampler and non sampler has cost i.e overhead is high; dynamic cost change
{
   totalCacheMissCounter++;
   if (totalCacheMissCounter == totalCacheMissCounter_saturation) //phase change
   {
      printf("totalCacheMissCounter saturates. threshold_minus_miss_counter is %d, threshold_miss_counter is %d and threshold_plus_miss_counter is %d\n", threshold_minus_miss_counter, threshold_miss_counter, threshold_plus_miss_counter);
      if((threshold_minus_miss_counter<threshold_miss_counter) && (threshold_minus_miss_counter<threshold_plus_miss_counter)) //decrement threshold, kn = k-
      {
         printf("decrement threshold\n");
         cost_threshold = cost_threshold_minus;
         cost_threshold_plus = lowestAboveThresholdCostMinus;
         cost_threshold_minus = highestBelowThresholdCostMinus;
      }
      else if((threshold_plus_miss_counter<threshold_miss_counter) && (threshold_plus_miss_counter<threshold_minus_miss_counter)) //increment threshold, kn = k+
      {
         printf("increment threshold\n");
         cost_threshold = cost_threshold_plus;
         cost_threshold_plus = lowestAboveThresholdCostPlus;
         cost_threshold_minus = highestBelowThresholdCostPlus;
      }
      else if((threshold_miss_counter<threshold_plus_miss_counter) && (threshold_miss_counter<threshold_minus_miss_counter)) //unchanged threshold
      {
         printf("unchanged threshold\n");
      }
      lru_miss_counter = 0;
      threshold_plus_miss_counter = 0;
      threshold_minus_miss_counter = 0;
      threshold_miss_counter = 0;
      totalCacheMissCounter = 0;
   }
   printf("cost_threshold is %d, cost_threshold_plus is %d, cost_threshold_minus is %d\n",cost_threshold, cost_threshold_plus, cost_threshold_minus);
   //printf("\ngetReplacementIndex called in l3 \n");
   UInt16 eip_truncated=eip%256;  //truncating eip to last 12bits
   //printf("set_index mod sampler_fraction is %d\n",(set_index % sampler_fraction));

   if ((set_index % sampler_fraction)==0) //sampler set, uses lru replacement
   {
      //printf("\nsampler set \n");   //ns
      //finding out invalid blocks
      printf("cost_threshold is %d\n", cost_threshold);
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



            //printf("\n cost is \n");
            for (UInt32 i=0; i< m_associativity; i++)
            {
               //printf("%d ", m_cost[i]);
            }

            printf("before eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[i]);
            //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
            if(m_cost[i]< cost_threshold)   //find highestBelowThresholdCost
            {
               if(m_cost[i]>highestBelowThresholdCost)
                  highestBelowThresholdCost = m_cost[i];
            }
            else if (m_cost[i]> cost_threshold) //find lowestAboveThresholdCost
            {
               if(m_cost[i]<lowestAboveThresholdCost)
                  lowestAboveThresholdCost = m_cost[i];
            }
            printf("after eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[i]);

            m_TI[i]=eip_truncated;
            m_cost[i]=0;
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



            //printf("\n cost is \n");
            for (UInt32 i=0; i< m_associativity; i++)
            {
               //printf("%d ", m_cost[i]);
            }

            printf("before eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[index]);
            //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
            if(m_cost[index]< cost_threshold)   //find highestBelowThresholdCost
            {
               if(m_cost[index]>highestBelowThresholdCost)
                  highestBelowThresholdCost = m_cost[index];
            }
            else if (m_cost[index]> cost_threshold) //find lowestAboveThresholdCost
            {
               if(m_cost[index]<lowestAboveThresholdCost)
                  lowestAboveThresholdCost = m_cost[index];
            }
            printf("after eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[index]);

            m_TI[index]=eip_truncated;
            m_cost[index]=0;

            moveToMRU(index);
            m_set_info->incrementAttempt(attempt);

            return index;
         }
      }

   }

   else if ((set_index % sampler_fraction)==1)  //sampler set using cost_threshold_plus and m_state_plus
   {
      printf("threshold_plus_miss_counter is %d, threshold_minus_miss_counter is %d, threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter, threshold_miss_counter);
      threshold_plus_miss_counter++;
      printf("increment threshold_plus_miss_counter\n");
      printf("cost_threshold_plus is %d\n", cost_threshold_plus);

      //finding out invalid blocks
      if(m_state_plus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {

         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               if((m_cost[i]>cost_threshold_plus) && (m_state_plus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_plus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_plus) && (m_state_plus[m_TI[i]]>0))
                  m_state_plus[m_TI[i]]--;



               printf("before eviction highestBelowThresholdCostPlus is %d, lowestAboveThresholdCostPlus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostPlus, lowestAboveThresholdCostPlus, m_cost[i]);
               //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
               if(m_cost[i]< cost_threshold_plus)   //find highestBelowThresholdCost
               {
                  if(m_cost[i]>highestBelowThresholdCostPlus)
                     highestBelowThresholdCostPlus = m_cost[i];
               }
               else if (m_cost[i]> cost_threshold_plus) //find lowestAboveThresholdCost
               {
                  if(m_cost[i]<lowestAboveThresholdCostPlus)
                     lowestAboveThresholdCostPlus = m_cost[i];
               }
               printf("after eviction highestBelowThresholdCostPlus is %d, lowestAboveThresholdCostPlus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostPlus, lowestAboveThresholdCostPlus, m_cost[i]);

               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
               if((m_cost[i]>cost_threshold_plus) && (m_state_plus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_plus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_plus) && (m_state_plus[m_TI[i]]>0))
                  m_state_plus[m_TI[i]]--;




               printf("before eviction highestBelowThresholdCostPlus is %d, lowestAboveThresholdCostPlus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostPlus, lowestAboveThresholdCostPlus, m_cost[i]);
               //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
               if(m_cost[i]< cost_threshold_plus)   //find highestBelowThresholdCost
               {
                  if(m_cost[i]>highestBelowThresholdCostPlus)
                     highestBelowThresholdCostPlus = m_cost[i];
               }
               else if (m_cost[i]> cost_threshold_plus) //find lowestAboveThresholdCost
               {
                  if(m_cost[i]<lowestAboveThresholdCostPlus)
                     lowestAboveThresholdCostPlus = m_cost[i];
               }
               printf("after eviction highestBelowThresholdCostPlus is %d, lowestAboveThresholdCostPlus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostPlus, lowestAboveThresholdCostPlus, m_cost[i]);

               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
            if((m_cost[i]>cost_threshold_plus) && (m_state_plus[m_TI[i]]<state_max)) //state should be incremented
               m_state_plus[m_TI[i]]++;
            else if((m_cost[i]<cost_threshold_plus) && (m_state_plus[m_TI[i]]>0))
               m_state_plus[m_TI[i]]--;



            printf("before eviction highestBelowThresholdCostPlus is %d, lowestAboveThresholdCostPlus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostPlus, lowestAboveThresholdCostPlus, m_cost[i]);
            //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
            if(m_cost[i]< cost_threshold_plus)   //find highestBelowThresholdCost
            {
               if(m_cost[i]>highestBelowThresholdCostPlus)
                  highestBelowThresholdCostPlus = m_cost[i];
            }
            else if (m_cost[i]> cost_threshold_plus) //find lowestAboveThresholdCost
            {
               if(m_cost[i]<lowestAboveThresholdCostPlus)
                  lowestAboveThresholdCostPlus = m_cost[i];
            }
            printf("after eviction highestBelowThresholdCostPlus is %d, lowestAboveThresholdCostPlus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostPlus, lowestAboveThresholdCostPlus, m_cost[i]);

            m_TI[i]=eip_truncated;
            m_cost[i]=0;
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
            if((m_cost[index]>cost_threshold_plus) && (m_state_plus[m_TI[index]]<state_max)) //state should be incremented
               m_state_plus[m_TI[index]]++;
            else if((m_cost[index]<cost_threshold_plus) && (m_state_plus[m_TI[index]]>0))
               m_state_plus[m_TI[index]]--;




            printf("before eviction highestBelowThresholdCostPlus is %d, lowestAboveThresholdCostPlus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostPlus, lowestAboveThresholdCostPlus, m_cost[index]);
            //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
            if(m_cost[index]< cost_threshold_plus)   //find highestBelowThresholdCost
            {
               if(m_cost[index]>highestBelowThresholdCostPlus)
                  highestBelowThresholdCostPlus = m_cost[index];
            }
            else if (m_cost[index]> cost_threshold_plus) //find lowestAboveThresholdCost
            {
               if(m_cost[index]<lowestAboveThresholdCostPlus)
                  lowestAboveThresholdCostPlus = m_cost[index];
            }
            printf("after eviction highestBelowThresholdCostPlus is %d, lowestAboveThresholdCostPlus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostPlus, lowestAboveThresholdCostPlus, m_cost[index]);

            m_TI[index]=eip_truncated;
            m_cost[index]=0;
            moveToMRU(index);
            m_set_info->incrementAttempt(attempt);
            return index;
         }
      }

   }

   else if ((set_index % sampler_fraction)==2)  //sampler set using cost_threshold_minus and m_state_minus
   {
      printf("threshold_plus_miss_counter is %d, threshold_minus_miss_counter is %d, threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter, threshold_miss_counter);
      threshold_minus_miss_counter++;
      printf("increment threshold_minus_miss_counter\n");
      printf("cost_threshold_minus is %d\n", cost_threshold_minus);

      //finding out invalid blocks
      if(m_state_minus[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {

         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               if((m_cost[i]>cost_threshold_minus) && (m_state_minus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_minus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_minus) && (m_state_minus[m_TI[i]]>0))
                  m_state_minus[m_TI[i]]--;



               printf("before eviction highestBelowThresholdCostMinus is %d, lowestAboveThresholdCostMinus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostMinus, lowestAboveThresholdCostMinus, m_cost[i]);
               //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
               if(m_cost[i]< cost_threshold_minus)   //find highestBelowThresholdCost
               {
                  if(m_cost[i]>highestBelowThresholdCostMinus)
                     highestBelowThresholdCostMinus = m_cost[i];
               }
               else if (m_cost[i]> cost_threshold_minus) //find lowestAboveThresholdCost
               {
                  if(m_cost[i]<lowestAboveThresholdCostMinus)
                     lowestAboveThresholdCostMinus = m_cost[i];
               }
               printf("after eviction highestBelowThresholdCostMinus is %d, lowestAboveThresholdCostMinus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostMinus, lowestAboveThresholdCostMinus, m_cost[i]);
               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
               if((m_cost[i]>cost_threshold_minus) && (m_state_minus[m_TI[i]]<state_max)) //state should be incremented
                  m_state_minus[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold_minus) && (m_state_minus[m_TI[i]]>0))
                  m_state_minus[m_TI[i]]--;



               printf("before eviction highestBelowThresholdCostMinus is %d, lowestAboveThresholdCostMinus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostMinus, lowestAboveThresholdCostMinus, m_cost[i]);
               //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
               if(m_cost[i]< cost_threshold_minus)   //find highestBelowThresholdCost
               {
                  if(m_cost[i]>highestBelowThresholdCostMinus)
                     highestBelowThresholdCostMinus = m_cost[i];
               }
               else if (m_cost[i]> cost_threshold_minus) //find lowestAboveThresholdCost
               {
                  if(m_cost[i]<lowestAboveThresholdCostMinus)
                     lowestAboveThresholdCostMinus = m_cost[i];
               }
               printf("after eviction highestBelowThresholdCostMinus is %d, lowestAboveThresholdCostMinus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostMinus, lowestAboveThresholdCostMinus, m_cost[i]);
               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
            if((m_cost[i]>cost_threshold_minus) && (m_state_minus[m_TI[i]]<state_max)) //state should be incremented
               m_state_minus[m_TI[i]]++;
            else if((m_cost[i]<cost_threshold_minus) && (m_state_minus[m_TI[i]]>0))
               m_state_minus[m_TI[i]]--;



            printf("before eviction highestBelowThresholdCostMinus is %d, lowestAboveThresholdCostMinus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostMinus, lowestAboveThresholdCostMinus, m_cost[i]);
            //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
            if(m_cost[i]< cost_threshold_minus)   //find highestBelowThresholdCost
            {
               if(m_cost[i]>highestBelowThresholdCostMinus)
                  highestBelowThresholdCostMinus = m_cost[i];
            }
            else if (m_cost[i]> cost_threshold_minus) //find lowestAboveThresholdCost
            {
               if(m_cost[i]<lowestAboveThresholdCostMinus)
                  lowestAboveThresholdCostMinus = m_cost[i];
            }
            printf("after eviction highestBelowThresholdCostMinus is %d, lowestAboveThresholdCostMinus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostMinus, lowestAboveThresholdCostMinus, m_cost[i]);
            m_TI[i]=eip_truncated;
            m_cost[i]=0;
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
            if((m_cost[index]>cost_threshold_minus) && (m_state_minus[m_TI[index]]<state_max)) //state should be incremented
               m_state_minus[m_TI[index]]++;
            else if((m_cost[index]<cost_threshold_minus) && (m_state_minus[m_TI[index]]>0))
               m_state_minus[m_TI[index]]--;




            printf("before eviction highestBelowThresholdCostMinus is %d, lowestAboveThresholdCostMinus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostMinus, lowestAboveThresholdCostMinus, m_cost[index]);
            //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
            if(m_cost[index]< cost_threshold_minus)   //find highestBelowThresholdCost
            {
               if(m_cost[index]>highestBelowThresholdCostMinus)
                  highestBelowThresholdCostMinus = m_cost[index];
            }
            else if (m_cost[index]> cost_threshold_minus) //find lowestAboveThresholdCost
            {
               if(m_cost[index]<lowestAboveThresholdCostMinus)
                  lowestAboveThresholdCostMinus = m_cost[index];
            }
            printf("after eviction highestBelowThresholdCostMinus is %d, lowestAboveThresholdCostMinus is %d, cost of eviction candidate is %d\n", highestBelowThresholdCostMinus, lowestAboveThresholdCostMinus, m_cost[index]);
            m_TI[index]=eip_truncated;
            m_cost[index]=0;
            moveToMRU(index);
            m_set_info->incrementAttempt(attempt);
            return index;
         }
      }

   }

   else //non-sampler set, uses phc
   {
      if((set_index % sampler_fraction)==3)  //sampler set for calculating misses on current threshold
      {
         printf("threshold_plus_miss_counter is %d, threshold_minus_miss_counter is %d, threshold_miss_counter is %d\n", threshold_plus_miss_counter, threshold_minus_miss_counter, threshold_miss_counter);
         threshold_miss_counter++;
         printf("increment threshold_miss_counter\n");
      }
      printf("cost_threshold is %d\n", cost_threshold);
      //printf("\nnon-smplr set \n");   //ns
      //finding out invalid blocks
      if(m_state[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
      {

         for (UInt32 i = SRAM_ways; i < m_associativity; i++)
         {
            if (!m_cache_block_info_array[i]->isValid())
            {
               if((m_cost[i]>cost_threshold) && (m_state[m_TI[i]]<state_max)) //state should be incremented
                  m_state[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold) && (m_state[m_TI[i]]>0))
                  m_state[m_TI[i]]--;



               printf("before eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[i]);
               //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
               if(m_cost[i]< cost_threshold)   //find highestBelowThresholdCost
               {
                  if(m_cost[i]>highestBelowThresholdCost)
                     highestBelowThresholdCost = m_cost[i];
               }
               else if (m_cost[i]> cost_threshold) //find lowestAboveThresholdCost
               {
                  if(m_cost[i]<lowestAboveThresholdCost)
                     lowestAboveThresholdCost = m_cost[i];
               }
               printf("after eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[i]);
               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
               if((m_cost[i]>cost_threshold) && (m_state[m_TI[i]]<state_max)) //state should be incremented
                  m_state[m_TI[i]]++;
               else if((m_cost[i]<cost_threshold) && (m_state[m_TI[i]]>0))
                  m_state[m_TI[i]]--;



               printf("before eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[i]);
               //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
               if(m_cost[i]< cost_threshold)   //find highestBelowThresholdCost
               {
                  if(m_cost[i]>highestBelowThresholdCost)
                     highestBelowThresholdCost = m_cost[i];
               }
               else if (m_cost[i]> cost_threshold) //find lowestAboveThresholdCost
               {
                  if(m_cost[i]<lowestAboveThresholdCost)
                     lowestAboveThresholdCost = m_cost[i];
               }
               printf("after eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[i]);
               m_TI[i]=eip_truncated;
               m_cost[i]=0;
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
            if((m_cost[i]>cost_threshold) && (m_state[m_TI[i]]<state_max)) //state should be incremented
               m_state[m_TI[i]]++;
            else if((m_cost[i]<cost_threshold) && (m_state[m_TI[i]]>0))
               m_state[m_TI[i]]--;


            printf("before eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[i]);
            //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
            if(m_cost[i]< cost_threshold)   //find highestBelowThresholdCost
            {
               if(m_cost[i]>highestBelowThresholdCost)
                  highestBelowThresholdCost = m_cost[i];
            }
            else if (m_cost[i]> cost_threshold) //find lowestAboveThresholdCost
            {
               if(m_cost[i]<lowestAboveThresholdCost)
                  lowestAboveThresholdCost = m_cost[i];
            }
            printf("after eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[i]);
            m_TI[i]=eip_truncated;
            m_cost[i]=0;
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

            if((m_cost[index]>cost_threshold) && (m_state[m_TI[index]]<state_max)) //state should be incremented
               m_state[m_TI[index]]++;
            else if((m_cost[index]<cost_threshold) && (m_state[m_TI[index]]>0))
               m_state[m_TI[index]]--;



            printf("before eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[index]);
            //before eviction we need to find the highestBelowThresholdCost or lowestAboveThresholdCost cost
            if(m_cost[index]< cost_threshold)   //find highestBelowThresholdCost
            {
               if(m_cost[index]>highestBelowThresholdCost)
                  highestBelowThresholdCost = m_cost[index];
            }
            else if (m_cost[index]> cost_threshold) //find lowestAboveThresholdCost
            {
               if(m_cost[index]<lowestAboveThresholdCost)
                  lowestAboveThresholdCost = m_cost[index];
            }
            printf("after eviction highestBelowThresholdCost is %d, lowestAboveThresholdCost is %d, cost of eviction candidate is %d\n", highestBelowThresholdCost, lowestAboveThresholdCost, m_cost[index]);
            m_TI[index]=eip_truncated;
            m_cost[index]=0;
            moveToMRU(index);
            m_set_info->incrementAttempt(attempt);
            return index;
         }
      }

   }

   LOG_PRINT_ERROR("Should not reach here");
}
*/


void
CacheSetPHC::updateReplacementIndex(UInt32 accessed_index, UInt8 write_flag, UInt32 set_index)
{
   //printf("\nupdateReplacementIndex called in l3. set index is %d and write flag is %d \n", set_index, write_flag);
   m_set_info->increment(m_lru_bits[accessed_index]);
   moveToMRU(accessed_index);

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

//////////////////////////////////////////////////////////////////////////////////////////////
//created by arindam to pass writeback information to policy files (required in phc)
void
CacheSetPHC::updateReplacementIndex2(UInt32 accessed_index, UInt32 set_index)
{
    //printf("\nsampler set, called from writeback, ");
    //printf("accessed index is %d", accessed_index);

    //printf("\nprevious cost is \n");
    for (UInt32 i=0; i< m_associativity; i++)
    {
       //printf("%d ", m_cost[i]);
    }
    //printf("\n");
    if ((accessed_index>=0) && (accessed_index<m_associativity))
    {
       m_cost[accessed_index]=m_cost[accessed_index]+Ew;  //cost modification
       //printf("\naccessed_index is %d", accessed_index);
    }
    else
       printf("\nERROR!! accessed_index is %d", accessed_index);
    //printf("new cost is \n");
    for (UInt32 i=0; i< m_associativity; i++)
    {
       //printf("%d ", m_cost[i]);
    }
    //printf("\n");
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
