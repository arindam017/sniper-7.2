#ifndef CACHE_SET_RANDOM_H
#define CACHE_SET_RANDOM_H

#include "cache_set.h"

class CacheSetRandom : public CacheSet
{
   public:
      CacheSetRandom(CacheBase::cache_t cache_type,
            UInt32 associativity, UInt32 blocksize);
      ~CacheSetRandom();

      UInt32 getReplacementIndex(CacheCntlr *cntlr, IntPtr eip, UInt32 set_index);
      void updateReplacementIndex(UInt32 accessed_index, UInt8 write_flag, UInt32 set_index);

      void updateReplacementIndex2(UInt32 accessed_index, UInt32 set_index, IntPtr eip);   //created by arindam to pass writeback information to policy files (required in phc)

   private:
      Random m_rand;
};

#endif /* CACHE_SET_RANDOM_H */
