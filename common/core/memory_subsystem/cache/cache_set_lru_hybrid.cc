
#include "cache_set_lru_hybrid.h"
#include "log.h"
#include "stats.h"
#include "cstdio"

// Implements LRU replacement, optionally augmented with Query-Based Selection [Jaleel et al., MICRO'10]

CacheSetLRU_Hybrid::CacheSetLRU_Hybrid(
      CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize, UInt32 setID)
   : CacheSet(cache_type, associativity, blocksize)
{
   m_lru_bits = new UInt8[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
      m_lru_bits[i] = i;

   /* Both the varibles m_access_writes, m_access_loads belong to a particular set */
   m_access_writes  = new UInt64[m_associativity];
   m_access_loads   = new UInt64[m_associativity];
   m_access_stores  = new UInt64[m_associativity];
   m_access_inserts = new UInt64[m_associativity];
   

   char buffIndex[20];
   char buffSetID[20];

   for(UInt32 i = 0; i < m_associativity; ++i)
   {
      m_access_writes[i] = 0;
      m_access_loads[i]  = 0;
      m_access_stores[i]  = 0;
      m_access_inserts[i]  = 0;

      /* To format wayID to 2 digits, with leading zeros if digits are less than 2 */  
      sprintf(buffIndex, "%02u", i);

      /* To format setID to 4 digits, with leading zeros if digits are less than 4*/  
      sprintf(buffSetID, "%04u", setID);

      /* Writes are inserts + stores */
      
      registerStatsMetric("hybridCache", 0, String("access_writes:")
                         + buffSetID + "-" + buffIndex, &m_access_writes[i]);

      registerStatsMetric("hybridCache", 0, String("access_inserts:")
                          + buffSetID + "-" + buffIndex, &m_access_inserts[i]);

      registerStatsMetric("hybridCache", 0, String("access_stores:")
                          + buffSetID + "-" + buffIndex, &m_access_stores[i]);

      registerStatsMetric("hybridCache", 0, String("access_loads:")
                          + buffSetID + "-" + buffIndex, &m_access_loads[i]);
   }

 

m_setID=setID;


}

CacheSetLRU_Hybrid::~CacheSetLRU_Hybrid()
{
   delete [] m_lru_bits;
   delete [] m_access_writes;
   delete [] m_access_loads;
   delete [] m_access_stores;
   delete [] m_access_inserts;
   
}

UInt32
CacheSetLRU_Hybrid::getReplacementIndex(CacheCntlr *cntlr,IntPtr eip)   //it is run in case of load and store miss
{
   // First try to find an invalid block
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (!m_cache_block_info_array[i]->isValid())
      {
         // Mark our newly-inserted line as most-recently used
         moveToMRU(i);
         m_access_writes[i]++;    //contains store(cache store hit)+insert(cache load/store miss) count
         m_access_inserts[i]++;  //load/store miss
         return i;
      }
   }

   // Make m_num_attemps attempts at evicting the block at LRU position
   for(UInt8 attempt = 0; attempt < m_associativity; ++attempt)
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
      moveToMRU(index);
      m_access_writes[index]++;
      m_access_inserts[index]++;   //load/store miss
      return index;
   }

   LOG_PRINT_ERROR("Should not reach here");
}

void
CacheSetLRU_Hybrid::updateReplacementIndex(UInt32 accessed_index, UInt8 write_flag)  ///it is run in case of load and store hit
{
   //printf("setID:%u\n", m_setID);

   /* When a cache block is getting updated, it means that the block is being read */
  

   if (write_flag == 0)
   {
      ///printf("No");
        m_access_loads[accessed_index]++;   //load hit  
   }
   else if (write_flag == 1)
   {
        m_access_stores[accessed_index]++;
        m_access_writes[accessed_index]++;
        //printf("Yes");
        //m_access_stores[accessed_index]++;  //store hit        
   }
   

   moveToMRU(accessed_index);

   

}

void
CacheSetLRU_Hybrid::moveToMRU(UInt32 accessed_index)
{
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (m_lru_bits[i] < m_lru_bits[accessed_index])
         m_lru_bits[i] ++;
   }
   m_lru_bits[accessed_index] = 0;
}
