#include "cache_set_hybridIntraCacheline.h"
#include "log.h"
#include "stats.h"

static UInt8 access_block0_write;
static UInt8 access_block1_write;
static UInt8 access_block2_write;
static UInt8 access_block3_write;

static UInt8 access_block0_read;
static UInt8 access_block1_read;
static UInt8 access_block2_read;
static UInt8 access_block3_read;

// Implements LRU replacement, optionally augmented with Query-Based Selection [Jaleel et al., MICRO'10]

CacheSetHybridIntraCacheLine::CacheSetHybridIntraCacheLine(
      CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize)
   : CacheSet(cache_type, associativity, blocksize)
{
   m_lru_bits = new UInt8[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
      m_lru_bits[i] = i;
}

CacheSetHybridIntraCacheLine::~CacheSetHybridIntraCacheLine()
{
   delete [] m_lru_bits;
}

UInt32
CacheSetHybridIntraCacheLine::getReplacementIndex(CacheCntlr *cntlr)
{
   // First try to find an invalid block
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (!m_cache_block_info_array[i]->isValid())
      {
         // Mark our newly-inserted line as most-recently used
         moveToMRU(i);
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
      return index;
   }

   LOG_PRINT_ERROR("Should not reach here");
}

extern UInt8 g_accessIsWrite;
extern UInt8 g_accessIsRead;

void
CacheSetHybridIntraCacheLine::updateReplacementIndex(UInt32 accessed_index)
{
   moveToMRU(accessed_index);
   
   LOG_ASSERT_ERROR(m_associativity == 16, "Associativity should be 16!");
   
   if (accessed_index < 4)
   {
        if (g_accessIsWrite)
        {
            access_block0_write++;
        }
        else if (g_accessIsRead)
        {
            access_block0_read++;
        }
   }
   else if (accessed_index < 8)
   {
        if (g_accessIsWrite)
        {
            access_block0_write++;
        }
        else if (g_accessIsRead)
        {
            access_block0_read++;
        }
   }
   else if (accessed_index < 12)
   {
        if (g_accessIsWrite)
        {
            access_block0_write++;
        }
        else if (g_accessIsRead)
        {
            access_block0_read++;
        }
   }
   else if (accessed_index < 16)
   {
        if (g_accessIsWrite)
        {
            access_block0_write++;
        }
        else if (g_accessIsRead)
        {
            access_block0_read++;
        }
   }
   else
   {
        LOG_ASSERT_ERROR(accessed_index >= 16, "Wrong Access Index!");
   }
}

void
CacheSetHybridIntraCacheLine::moveToMRU(UInt32 accessed_index)
{
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (m_lru_bits[i] < m_lru_bits[accessed_index])
         m_lru_bits[i] ++;
   }
   
   m_lru_bits[accessed_index] = 0;
}
