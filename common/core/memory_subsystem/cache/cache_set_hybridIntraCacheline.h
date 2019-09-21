#ifndef CACHE_SET_HRBRID_INTRACACHELINE_H
#define CACHE_SET_HRBRID_INTRACACHELINE_H

#include "cache_set.h"
#include "cache_set_lru.h"

class CacheSetHybridIntraCacheLine : public CacheSet
{
   public:
      CacheSetHybridIntraCacheLine(CacheBase::cache_t cache_type, UInt32 associativity,
                                   UInt32 blocksize);
      ~CacheSetHybridIntraCacheLine();

      UInt32 getReplacementIndex(CacheCntlr *cntlr);
      void updateReplacementIndex(UInt32 accessed_index);

   private:
      UInt8* m_lru_bits;
      void moveToMRU(UInt32 accessed_index);
};

#endif /* CACHE_SET_HRBRID_INTRACACHELINE_H */
