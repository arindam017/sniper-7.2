#ifndef CACHE_SET_LRU_HYBRID_H
#define CACHE_SET_LRU_HYBRID_H

#include "cache_set.h"

class CacheSetLRU_Hybrid : public CacheSet
{
   public:
      CacheSetLRU_Hybrid(CacheBase::cache_t cache_type,UInt32 associativity, UInt32 blocksize, UInt32 setID);
      virtual ~CacheSetLRU_Hybrid();

      virtual UInt32 getReplacementIndex(CacheCntlr *cntlr,IntPtr eip);
      void updateReplacementIndex(UInt32 accessed_index, UInt8 write_flag);

   protected:
      UInt32 m_setID;
      UInt8* m_lru_bits;
      /* As these (writes, reads and inserts) numbers are logged in registerStatsMetric,
       * they have to UInt64 */ 
      UInt64* m_access_writes;
      UInt64* m_access_inserts;
      UInt64* m_access_stores;
      UInt64* m_access_loads;
      
      void moveToMRU(UInt32 accessed_index);
};

#endif /* CACHE_SET_LRU_HYBRID_H */
