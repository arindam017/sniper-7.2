#include "cache_set_round_robin.h"

CacheSetRoundRobin::CacheSetRoundRobin(
      CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize) :
   CacheSet(cache_type, associativity, blocksize)
{
   m_replacement_index = m_associativity - 1;
}

CacheSetRoundRobin::~CacheSetRoundRobin()
{}

UInt32
CacheSetRoundRobin::getReplacementIndex(CacheCntlr *cntlr, UInt8 l3_hit_flag, IntPtr eip, UInt32 set_index)
{
   UInt32 curr_replacement_index = m_replacement_index;
   m_replacement_index = (m_replacement_index == 0) ? (m_associativity-1) : (m_replacement_index-1);

   if (!isValidReplacement(m_replacement_index))
      return getReplacementIndex(cntlr, 100, 0, set_index);
   else
      return curr_replacement_index;
}

void
CacheSetRoundRobin::updateReplacementIndex(UInt32 accessed_index, UInt8 write_flag, UInt32 set_index)
{
   return;
}

////////////created by Arindam//////////////////sn
void
CacheSetRoundRobin::updateLoopBitPolicy(UInt32 index, UInt8 loopbit)
{
  
}
//////////////////////////////////////////////////
