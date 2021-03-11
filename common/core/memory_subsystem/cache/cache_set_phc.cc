#include "cache_set_phc.h"
#include "log.h"
#include "stats.h"
#include "simulator.h"
#include "config.hpp"
#include "cache.h"
#include "cache_set.h"


///////// required for dynamic cost change/////////////
UInt8 CplusK = 255;                 //C+(K0)
UInt8 CminusK = 0;                  //C-(K0)
UInt8 CplusKplus = 255;             //C+(K+)
UInt8 CplusKminus = 255;            //C+(K-)
UInt8 CminusKplus = 0;              //C-(K+)
UInt8 CminusKminus = 0;             //C-(K-)

UInt8 Cmax = 128;
UInt8 Cmin = 128;
UInt8 Cplusmax = 128;
UInt8 Cplusmin = 128;
UInt8 Cminusmax = 128;
UInt8 Cminusmin = 128;

///////// required for dynamic write burst prediction/////////////
UInt8 WplusK = 255;                 //W+(K0)
UInt8 WminusK = 0;                  //W-(K0)
UInt8 WplusKplus = 255;             //W+(K+)
UInt8 WplusKminus = 255;            //W+(K-)
UInt8 WminusKplus = 0;              //W-(K+)
UInt8 WminusKminus = 0;             //W-(K-)

UInt8 Wmax = 128;
UInt8 Wmin = 128;
UInt8 Wplusmax = 128;
UInt8 Wplusmin = 128;
UInt8 Wminusmax = 128;
UInt8 Wminusmin = 128;


static UInt16 Mplus = 0;               //counters. Same as M0, M+ and M- in paper
static UInt16 Mminus = 0;

static UInt16 M0 = 0;
static UInt16 MSamplerSetDcntMin = 0;
static UInt16 MSamplerSetDcntPlu = 0;


static UInt16 Wcntplus = 0;
static UInt16 Wcntminus = 0;
static UInt16 Wcnt0 = 0;

//static UInt8 C[4096] = {0};            //evicted Cost set for K0
//static UInt8 Cplus[4096] = {0};        //evicted Cost set for K+
//static UInt8 Cminus[4096] = {0};       //evicted Cost set for K-

UInt8 K = 148;                         //cost thresholds. Same as k, K+ and K- in paper
UInt8 Kplus = 149;
UInt8 Kminus = 147;

UInt8 W = 148;                         //write burst thresholds. Same as k, K+ and K- in paper
UInt8 Wplus = 149;
UInt8 Wminus = 147;


///////// required for static cost change////////////////
UInt8 cost_threshold = 148;
UInt8 cost_threshold_plus = 149;
UInt8 cost_threshold_minus = 147;
//in paper cost varies from -127 to 128. I am varying it from 0 to 255. 128 is 0 for me. So threshold of 20 translates to 148.

/////////////////////////////////////////////////////////

UInt8 Ew=24;
UInt8 Er=1;
UInt16 predictor_table_length=256;
UInt8 state_max=7;
UInt8 state_threshold = (state_max+1)/2;
UInt8 SRAM_ways=4;
UInt32 number_of_sets = 8192;
UInt32 sampler_fraction = 32;

extern UInt64 globalWritebacksToL3counter;   //this is a global counter. this counter will be reset when updateReplacementindex for phc in LLC is called 
UInt8 migrate_flag = 0;

static UInt8 m_state[256] = {0};             //state table used by sampler set 3-63
static UInt8 m_state_plus[256] = {0};        //state table used by sampler set 1
static UInt8 m_state_minus[256] = {0};       //state table used by sampler set 2

static UInt8 m_dcnt[256] = {0};              //deadblock predictor table
static UInt8 dcnt_initialization = 0;        //this variable is used to make sure dcnt is initialized only once

//////////////////dynamis deadblock threshold////////////////////////////////////////
static UInt8 dcnt_threshold = 192;                //deadblock predictor table threshold 
static UInt8 dcnt_threshold_plu = 194;           //deadblock predictor table threshold 
static UInt8 dcnt_threshold_min = 190;           //deadblock predictor table threshold 
//////////////////////////////////////////////////////////////////////////////////////

static bool changeToHighHistoryFlag = true;

static UInt8 m_wcnt[256] = {0};              //Write burst predictor

static UInt8 asl2_flag = 0;


static UInt16 lru_miss_counter = 0;

static UInt64 totalCacheMissCounter = 0;
static UInt64 totalCacheMissCounter_saturation = 4096;


static UInt64 readToWriteTransitionsAtInterval = 0;
static UInt64 writeToReadTransitionsAtInterval = 0;
static UInt64 readToWriteTransitionsAtEviction = 0;
static UInt64 writeToReadTransitionsAtEviction = 0;

static UInt64 validBlockEvicted = 0;





static UInt64 readIntenseBlocksM1                  = 0;
static UInt64 writeIntenseBlocksM1                 = 0;
static UInt64 readIntenseBlocksMDuringWrite        = 0;
static UInt64 writeIntenseBlocksMDuringWrite       = 0;

static UInt64 blocksReadBeforeM1020                = 0;
static UInt64 blocksReadBeforeM12040               = 0;
static UInt64 blocksReadBeforeM14060               = 0;
static UInt64 blocksReadBeforeM16080               = 0;
static UInt64 blocksReadBeforeM180100              = 0;
static UInt64 blocksReadAfterM1020                = 0;
static UInt64 blocksReadAfterM12040               = 0;
static UInt64 blocksReadAfterM14060               = 0;
static UInt64 blocksReadAfterM16080               = 0;
static UInt64 blocksReadAfterM180100              = 0;

static UInt64 blocksReadBeforeMDuringWrite020      = 0;
static UInt64 blocksReadBeforeMDuringWrite2040     = 0;
static UInt64 blocksReadBeforeMDuringWrite4060     = 0;
static UInt64 blocksReadBeforeMDuringWrite6080     = 0;
static UInt64 blocksReadBeforeMDuringWrite80100    = 0;
static UInt64 blocksReadAfterMDuringWrite020      = 0;
static UInt64 blocksReadAfterMDuringWrite2040     = 0;
static UInt64 blocksReadAfterMDuringWrite4060     = 0;
static UInt64 blocksReadAfterMDuringWrite6080     = 0;
static UInt64 blocksReadAfterMDuringWrite80100    = 0;

static UInt64 blocksWriteBeforeM1020               = 0;
static UInt64 blocksWriteBeforeM12040              = 0;
static UInt64 blocksWriteBeforeM14060              = 0;
static UInt64 blocksWriteBeforeM16080              = 0;
static UInt64 blocksWriteBeforeM180100             = 0;
static UInt64 blocksWriteAfterM1020               = 0;
static UInt64 blocksWriteAfterM12040              = 0;
static UInt64 blocksWriteAfterM14060              = 0;
static UInt64 blocksWriteAfterM16080              = 0;
static UInt64 blocksWriteAfterM180100             = 0;

static UInt64 blocksWriteBeforeMDuringWrite020     = 0;
static UInt64 blocksWriteBeforeMDuringWrite2040    = 0;
static UInt64 blocksWriteBeforeMDuringWrite4060    = 0;
static UInt64 blocksWriteBeforeMDuringWrite6080    = 0;
static UInt64 blocksWriteBeforeMDuringWrite80100   = 0;
static UInt64 blocksWriteAfterMDuringWrite020     = 0;
static UInt64 blocksWriteAfterMDuringWrite2040    = 0;
static UInt64 blocksWriteAfterMDuringWrite4060    = 0;
static UInt64 blocksWriteAfterMDuringWrite6080    = 0;
static UInt64 blocksWriteAfterMDuringWrite80100   = 0;

static UInt64 m1Count                              = 0;
static UInt64 mDuringWriteCount                    = 0;

static UInt64 SRAMReadBeforeMigration              = 0;
static UInt64 SRAMReadAfterMigration               = 0;
static UInt64 SRAMWriteBeforeMigration             = 0;
static UInt64 SRAMWriteAfterMigration              = 0;
static UInt64 STTRAMReadBeforeMigration            = 0;
static UInt64 STTRAMReadAfterMigration             = 0;
static UInt64 STTRAMWriteBeforeMigration           = 0;
static UInt64 STTRAMWriteAfterMigration            = 0;

static UInt64 livingSRAMBlocksEvicted              = 0;
static UInt64 deadSRAMBlocksEvicted                = 0;
      
static UInt64 singleMigrationCount                = 0;
static UInt64 doubleMigrationCount                = 0;
static UInt64 noMigrationCount                    = 0;

static UInt64 a0                    = 0;
static UInt64 b0                    = 0;
static UInt64 c0                    = 0;
static UInt64 d0                    = 0;
static UInt64 e0                    = 0;
static UInt64 f0                    = 0;
static UInt64 g0                    = 0;

static UInt64 a1                    = 0;
static UInt64 b1                    = 0;
static UInt64 c1                    = 0;
static UInt64 d1                    = 0;
static UInt64 e1                    = 0;
static UInt64 f1                    = 0;
static UInt64 g1                    = 0;

static UInt64 validSRAMEvictions                    = 0;
static UInt64 validSTTREvictions                    = 0;

static UInt64 forceMigrationSramToSTTram     = 0;
static UInt64 forceMigrationSTTramToSram     = 0;

static UInt64 SRAM_reads_hits    = 0;
static UInt64 SRAM_write_hits    = 0;
static UInt64 STTR_reads_hits    = 0;
static UInt64 STTR_write_hits    = 0;




static UInt16 histogram[1000] = {0};
static UInt8 sf = 1;

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

   //for (UInt32 i = 0; i < m_associativity; i++)
   //   m_lru_bits[i] = i;

   //Implementing 2 separate LRUs for SRAM ans STTRAM
   for (UInt32 i=0; i<SRAM_ways; i++)
      m_lru_bits[i]=i;
   for (UInt32 i=SRAM_ways; i<m_associativity; i++)
      m_lru_bits[i] = (i-SRAM_ways);

   m_lru_bits_unified = new UInt8[m_associativity];   //This is a unified LRU stack
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      m_lru_bits_unified[i] = i; //unified LRU stack
   }




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

   if(dcnt_initialization==0)    //Used to initiaize dcnt array to 128 only once. This is a global array, not for a particular set. Also m_state is initialised once
   {
      for(UInt32 i = 0; i<256; i++)
      {
         m_dcnt[i] = 128;
         m_state[i] = state_max/2;
         m_state_plus[i] = state_max/2;
         m_state_minus[i] = state_max/2;

         m_wcnt[i] = 128;
      }

      dcnt_initialization = 1;
   }

   m_deadblock = new UInt8[m_associativity];    //deadblock counter using Newton's method
   for (UInt32 i = 0; i < m_associativity; i++)
      m_deadblock[i] = 0; 



   m_read_before_m1              = new UInt64[m_associativity];
   m_write_before_m1             = new UInt64[m_associativity];
   m_read_after_m1               = new UInt64[m_associativity];
   m_write_after_m1              = new UInt64[m_associativity];
   m1_flag                       = new UInt8[m_associativity];
   m_read_before_mduringwrite    = new UInt64[m_associativity];
   m_write_before_mduringwrite   = new UInt64[m_associativity];
   m_read_after_mduringwrite     = new UInt64[m_associativity];
   m_write_after_mduringwrite    = new UInt64[m_associativity];
   mduringwrite_flag             = new UInt8[m_associativity];

   for (UInt32 i = 0; i < m_associativity; i++)
   {
      m_read_before_m1[i]  = 0;
      m_write_before_m1[i] = 0;  
      m_read_after_m1[i]   = 0; 
      m_write_after_m1[i]  = 0; 
      m1_flag[i]           = 0;  //If a block has been migrated this is set to 1

      m_read_before_mduringwrite[i]  = 0;
      m_write_before_mduringwrite[i] = 0;  
      m_read_after_mduringwrite[i]   = 0; 
      m_write_after_mduringwrite[i]  = 0; 
      mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
   }

   

   ///////////////////////////////////////////////////////
   if (0 == g_iteration_count)   //copied from Udal. This loop ensures that register stat metric is called once instead of for all the sets
   {
      //printf("\n\nInterval length for phase change is %d and interval length for wrti is %d\n", totalCacheMissCounter_saturation, N_transition);
      printf("Associativity is %d, SRAM ways are %d\n\n\n", m_associativity, SRAM_ways);
      g_iteration_count++;
      registerStatsMetric("interval_timer", 0 , "Write_To_Read_Transitions_At_Eviction",   &writeToReadTransitionsAtEviction);
      registerStatsMetric("interval_timer", 0 , "Read_To_Write_Transitions_At_Eviction",   &readToWriteTransitionsAtEviction);
      registerStatsMetric("interval_timer", 0 , "Read_Intense_Block_Counter",              &read_intense_block_counter);
      registerStatsMetric("interval_timer", 0 , "Write_Intense_Block_Counter",             &write_intense_block_counter);
      registerStatsMetric("interval_timer", 0 , "Deadblock_Counter",                       &deadblock_counter);
      registerStatsMetric("interval_timer", 0 , "Valid_Block_Evicted",                     &validBlockEvicted);


      registerStatsMetric("interval_timer", 0 , "Read_Intense_Blocks_M",                  &readIntenseBlocksM1               );
      registerStatsMetric("interval_timer", 0 , "Write_Intense_Blocks_M",                 &writeIntenseBlocksM1              );
      registerStatsMetric("interval_timer", 0 , "Read_Intense_Blocks_MDuringMigrate",      &readIntenseBlocksMDuringWrite     );
      registerStatsMetric("interval_timer", 0 , "Write_Intense_Blocks_MDuringMigrate",     &writeIntenseBlocksMDuringWrite    );

      registerStatsMetric("interval_timer", 0 , "M_Count",                               &m1Count                          );
      registerStatsMetric("interval_timer", 0 , "Migrate_During_Write_Count",             &mDuringWriteCount                );

      registerStatsMetric("interval_timer", 0 , "SRAM_Read_Before_Migration",                  &SRAMReadBeforeMigration    );
      registerStatsMetric("interval_timer", 0 , "SRAM_Read_After_Migration",                   &SRAMReadAfterMigration     );
      registerStatsMetric("interval_timer", 0 , "SRAM_Write_Before_Migration",                 &SRAMWriteBeforeMigration   );
      registerStatsMetric("interval_timer", 0 , "SRAM_Write_After_Migration",                  &SRAMWriteAfterMigration    );
      registerStatsMetric("interval_timer", 0 , "STTRAM_Read_Before_Migration",                &STTRAMReadBeforeMigration  );
      registerStatsMetric("interval_timer", 0 , "STTRAM_Read_After_Migration",                 &STTRAMReadAfterMigration   );
      registerStatsMetric("interval_timer", 0 , "STTRAM_Write_Before_Migration",               &STTRAMWriteBeforeMigration );
      registerStatsMetric("interval_timer", 0 , "STTRAM_Write_After_Migration",                &STTRAMWriteAfterMigration  );

      registerStatsMetric("interval_timer", 0 , "living_SRAM_Blocks_Evicted",                &livingSRAMBlocksEvicted  );
      registerStatsMetric("interval_timer", 0 , "dead_SRAM_Blocks_Evicted",                &deadSRAMBlocksEvicted  ); 

      registerStatsMetric("interval_timer", 0 , "single_Migration_Count",                 &singleMigrationCount  ); 
      registerStatsMetric("interval_timer", 0 , "double_Migration_Count",                 &doubleMigrationCount  ); 
      registerStatsMetric("interval_timer", 0 , "no_Migration_Count",                     &noMigrationCount  );   
        
      registerStatsMetric("interval_timer", 0 , "a_SRAM",        &a0  );
      registerStatsMetric("interval_timer", 0 , "b_SRAM",        &b0  );
      registerStatsMetric("interval_timer", 0 , "c_SRAM",        &c0  );
      registerStatsMetric("interval_timer", 0 , "d_SRAM",        &d0  );
      registerStatsMetric("interval_timer", 0 , "e_SRAM",        &e0  );
      registerStatsMetric("interval_timer", 0 , "f_SRAM",        &f0  );
      registerStatsMetric("interval_timer", 0 , "g_SRAM",        &g0  );  
      registerStatsMetric("interval_timer", 0 , "a_STTR",        &a1  );
      registerStatsMetric("interval_timer", 0 , "b_STTR",        &b1  );
      registerStatsMetric("interval_timer", 0 , "c_STTR",        &c1  );
      registerStatsMetric("interval_timer", 0 , "d_STTR",        &d1  );
      registerStatsMetric("interval_timer", 0 , "e_STTR",        &e1  );
      registerStatsMetric("interval_timer", 0 , "f_STTR",        &f1  );
      registerStatsMetric("interval_timer", 0 , "g_STTR",        &g1  ); 

      registerStatsMetric("interval_timer", 0 , "force_Migration_SramToSTTram",        &forceMigrationSramToSTTram  ); 
      registerStatsMetric("interval_timer", 0 , "force_Migration_STTramToSram",        &forceMigrationSTTramToSram  ); 

      registerStatsMetric("interval_timer", 0 , "SRAM_reads_hits",        &SRAM_reads_hits  ); 
      registerStatsMetric("interval_timer", 0 , "SRAM_write_hits",        &SRAM_write_hits  ); 
      registerStatsMetric("interval_timer", 0 , "STTR_reads_hits",        &STTR_reads_hits  ); 
      registerStatsMetric("interval_timer", 0 , "STTR_write_hits",        &STTR_write_hits  ); 
      

   }
}



CacheSetPHC::~CacheSetPHC()
{
   delete [] m_lru_bits;
   delete [] m_lru_bits_unified;
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

   //printf("K is %d\n", K);

   if (totalCacheMissCounter == totalCacheMissCounter_saturation) //phase change
   {


      //////////////////////cost/////////////////////////////////

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

      /////////////////////write burst//////////////////////////////////

      //finding out WminusK
      if(Wmin<W)  
         WminusK = Wmax;
      else 
         WminusK = W;

      //finding out WplusK
      if(Wmax>W)  
         WplusK = Wmin;
      else 
         WplusK = W;

      ///////////////////////////////////////////////////////

      //finding out WminusKminus
      if(Wminusmin<Wminus)
         WminusKminus = Wminusmax;
      else 
         WminusKminus = Wminus;

      //finding out WplusKminus
      if(Wminusmax>Wminus)
         WplusKminus = Wminusmin;
      else 
         WplusKminus = Wminus;

      ///////////////////////////////////////////////////////

      //finding out WminusKplus
      if(Wplusmin<Wplus)
         WminusKplus = Wplusmax;
      else
         WminusKplus = Wplus;

      //finding out WplusKplus
      if(Wplusmax>Wplus)
         WplusKplus = Wplusmin;
      else 
         WplusKplus = Wplus;


      /////////////////////////////////////////////////////////////////////////////////


      if((Wcntminus<Wcnt0) && (Wcntminus<Wcntplus))         //decrement threshold; K should be replaced by Kminus
      {
         W = Wminus;
         Wplus = WplusKminus;
         Wminus = WminusKminus;         
      }
      else if((Wcntplus<Wcnt0) && (Wcntplus<Wcntminus))     //increment threshold; K should be replaced by Kplus
      {
         W = Wplus;
         Wplus = WplusKplus;
         Wminus = WminusKplus;
      }
      else                                      //only change K+ and K-                       
      {
         Wminus = WminusK;
         Wplus = WplusK;
      }


      //////////////////// deadblock threshold adjustment//////////////////////////

      //printf("MSamplerSetDcntMin is %d, MSamplerSetDcntPlu is %d and M0 is %d\n", MSamplerSetDcntMin, MSamplerSetDcntPlu, M0);

      if (MSamplerSetDcntMin < MSamplerSetDcntPlu)
      {
         if(MSamplerSetDcntMin <= M0)
         {
            if (dcnt_threshold_min > 1)   //Decrease T0
            {
               dcnt_threshold = dcnt_threshold_min;
               dcnt_threshold_min = dcnt_threshold - 2;
               dcnt_threshold_plu = dcnt_threshold + 2;

               changeToHighHistoryFlag = false;
            } 
         }
      }
      else if (MSamplerSetDcntMin > MSamplerSetDcntPlu)
      {
         if (MSamplerSetDcntPlu <= M0)
         {
            if (dcnt_threshold_plu < 254) //Increase T0
            {
               dcnt_threshold = dcnt_threshold_plu;
               dcnt_threshold_min = dcnt_threshold - 2;
               dcnt_threshold_plu = dcnt_threshold + 2;

               changeToHighHistoryFlag = true;
            }
         }
      }
      else
      {
         if(MSamplerSetDcntMin <= M0)  //ChangeToHigh/ChangeTOLow: Coin Toss
         {
            if(changeToHighHistoryFlag)
            {
               if (dcnt_threshold_plu < 254)
               {
                  dcnt_threshold = dcnt_threshold_plu;
                  dcnt_threshold_min = dcnt_threshold - 2;
                  dcnt_threshold_plu = dcnt_threshold + 2;

                  changeToHighHistoryFlag = true;
               }

            }
            else
            {
               if (dcnt_threshold_min > 1)   //Decrease T0
               {
                  dcnt_threshold = dcnt_threshold_min;
                  dcnt_threshold_min = dcnt_threshold - 2;
                  dcnt_threshold_plu = dcnt_threshold + 2;
   
                  changeToHighHistoryFlag = false;
               }

            }
            
         }
      }


      /////////////////////////////////////////////////////////////////////////////
      

      //reinitialization
      lru_miss_counter = 0;

      Mplus = 0;
      Mminus = 0;

      M0 = 0;
      MSamplerSetDcntMin = 0;
      MSamplerSetDcntPlu = 0;


      Wcntplus = 0;
      Wcntminus = 0;
      Wcnt0 = 0;

      totalCacheMissCounter = 0;

      Cmax = 128;
      Cmin = 128;
      Cplusmax = 128;
      Cplusmin = 128;
      Cminusmax = 128;
      Cminusmin = 128;

      Wmax = 128;
      Wmin = 128;
      Wplusmax = 128;
      Wplusmin = 128;
      Wminusmax = 128;
      Wminusmin = 128;
      
   
      read_intense_block_counter = read_intense_block_counter/2;
      write_intense_block_counter = write_intense_block_counter/2;
      deadblock_counter = deadblock_counter/2;

   }
   
   
   //UInt16 eip_truncated=eip%256;  //truncating eip to last 12bits
   UInt16 eip_truncated = truncatedEipCalculation(eip);  //this is done for generating hashed PC. Last 5 bytes (LSB) are XOR-ed


   if ((set_index % sampler_fraction)==1)  //sampler set using Kplus and m_state_plus
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

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1 

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

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1

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

            m_read_before_m1[i]  = 0;
            m_write_before_m1[i] = 0;  
            m_read_after_m1[i]   = 0; 
            m_write_after_m1[i]  = 0; 
            m1_flag[i]           = 0; 

            m_read_before_mduringwrite[i]  = 0;
            m_write_before_mduringwrite[i] = 0;  
            m_read_after_mduringwrite[i]   = 0; 
            m_write_after_mduringwrite[i]  = 0; 
            mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

         //Calculating Cplusmax and Cplusmin
         if(m_cost[index]<Cplusmin)
            Cplusmin = m_cost[index];
         if(m_cost[index]>Cplusmax)
            Cplusmax = m_cost[index];

         //Calculating Wmax and Wmin
         if(m_wcnt[m_TI[index]]<Wmin)
            Wmin = m_wcnt[m_TI[index]];
         if(m_wcnt[m_TI[index]]>Wmax)
            Wmax = m_wcnt[m_TI[index]];


         if(m_dcnt[m_TI[index]] != 255)
            m_dcnt[m_TI[index]]++;  //increment deadblock counter on eviction

         if(m_wcnt[m_TI[index]] != 0)
            m_wcnt[m_TI[index]]--;  //decrement write burst counter on eviction


         if((index<SRAM_ways) && (index>=0))
         {
            if(m_dcnt[m_TI[index]]<dcnt_threshold)
               livingSRAMBlocksEvicted++;
            else
               deadSRAMBlocksEvicted++;

         }

         if((m1_flag[index]==1) && (mduringwrite_flag[index]==1))
            doubleMigrationCount++;
         else if ((m1_flag[index]==0) && (mduringwrite_flag[index]==0))
            noMigrationCount++;
         else
            singleMigrationCount++;
         

         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         UInt64 percentageReadBeforeM1             =  0;
         UInt64 percentageWriteBeforeM1            =  0;

         UInt64 percentageReadBeforeMDuringWrite   =  0;
         UInt64 percentageWriteBeforeMDuringWrite  =  0;

         
         if (m_read_before_m1[index]!=0)
            percentageReadBeforeM1             =   ((100 * m_read_before_m1[index])/(m_read_before_m1[index] + m_read_after_m1[index]));
         
         if (m_write_before_m1[index]!=0)
            percentageWriteBeforeM1            =   ((100 * m_write_before_m1[index])/(m_write_before_m1[index] + m_write_after_m1[index]));

         if (m_read_before_mduringwrite[index]!=0)
            percentageReadBeforeMDuringWrite   =   ((100 * m_read_before_mduringwrite[index])/(m_read_before_mduringwrite[index] + m_read_after_mduringwrite[index])); 
         
         if (m_write_before_mduringwrite[index]!=0)
            percentageWriteBeforeMDuringWrite  =   ((100 * m_write_before_mduringwrite[index])/(m_write_before_mduringwrite[index] + m_write_after_mduringwrite[index]));

         
         ///////////////////[READ-M1]////////////////////////
         if ((percentageReadBeforeM1>=0)&&(percentageReadBeforeM1<20))
         {
            blocksReadBeforeM1020++;
            blocksReadAfterM180100++;
         }

         else if ((percentageReadBeforeM1>=20)&&(percentageReadBeforeM1<40))
         {
            blocksReadBeforeM12040++;
            blocksReadAfterM16080++;
         }

         else if ((percentageReadBeforeM1>=40)&&(percentageReadBeforeM1<60))
         {
            blocksReadBeforeM14060++;
            blocksReadAfterM14060++;
         }

         else if ((percentageReadBeforeM1>=60)&&(percentageReadBeforeM1<80))
         {
            blocksReadBeforeM16080++;
            blocksReadAfterM12040++;
         }

         else if ((percentageReadBeforeM1>=80)&&(percentageReadBeforeM1<=100))
         {
            blocksReadBeforeM180100++;
            blocksReadAfterM1020++;
         }

         else
         {

         }


         ///////////////////[READ-MDURINGWRITE]////////////////////////
         if ((percentageReadBeforeMDuringWrite>=0)&&(percentageReadBeforeMDuringWrite<20))
         {
            blocksReadBeforeMDuringWrite020++;
            blocksReadAfterMDuringWrite80100++;
         }

         else if ((percentageReadBeforeMDuringWrite>=20)&&(percentageReadBeforeMDuringWrite<40))
         {
            blocksReadBeforeMDuringWrite2040++;
            blocksReadAfterMDuringWrite6080++;
         }

         else if ((percentageReadBeforeMDuringWrite>=40)&&(percentageReadBeforeMDuringWrite<60))
         {
            blocksReadBeforeMDuringWrite4060++;
            blocksReadAfterMDuringWrite4060++;
         }

         else if ((percentageReadBeforeMDuringWrite>=60)&&(percentageReadBeforeMDuringWrite<80))
         {
            blocksReadBeforeMDuringWrite6080++;
            blocksReadAfterMDuringWrite2040++;
         }

         else if ((percentageReadBeforeMDuringWrite>=80)&&(percentageReadBeforeMDuringWrite<=100))
         {
            blocksReadBeforeMDuringWrite80100++;
            blocksReadAfterMDuringWrite020++;
         }

         else
         {
            
         }


         ///////////////////[WRITE-M1]////////////////////////

         if ((percentageWriteBeforeM1>=0)&&(percentageWriteBeforeM1<20))
         {
            blocksWriteBeforeM1020++;
            blocksWriteAfterM180100++;
         }

         else if ((percentageWriteBeforeM1>=20)&&(percentageWriteBeforeM1<40))
         {
            blocksWriteBeforeM12040++;
            blocksWriteAfterM16080++;
         }

         else if ((percentageWriteBeforeM1>=40)&&(percentageWriteBeforeM1<60))
         {
            blocksWriteBeforeM14060++;
            blocksWriteAfterM14060++;
         }

         else if ((percentageWriteBeforeM1>=60)&&(percentageWriteBeforeM1<80))
         {
            blocksWriteBeforeM16080++;
            blocksWriteAfterM12040++;
         }

         else if ((percentageWriteBeforeM1>=80)&&(percentageWriteBeforeM1<=100))
         {
            blocksWriteBeforeM180100++;
            blocksWriteAfterM1020++;
         }

         else
         {

         }


         ///////////////////[WRITE-MDURINGWRITE]////////////////////////
         if ((percentageWriteBeforeMDuringWrite>=0)&&(percentageWriteBeforeMDuringWrite<20))
         {
            blocksWriteBeforeMDuringWrite020++;
            blocksWriteAfterMDuringWrite80100++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=20)&&(percentageWriteBeforeMDuringWrite<40))
         {
            blocksWriteBeforeMDuringWrite2040++;
            blocksWriteAfterMDuringWrite6080++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=40)&&(percentageWriteBeforeMDuringWrite<60))
         {
            blocksWriteBeforeMDuringWrite4060++;
            blocksWriteAfterMDuringWrite4060++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=60)&&(percentageWriteBeforeMDuringWrite<80))
         {
            blocksWriteBeforeMDuringWrite6080++;
            blocksWriteAfterMDuringWrite2040++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=80)&&(percentageWriteBeforeMDuringWrite<=100))
         {
            blocksWriteBeforeMDuringWrite80100++;
            blocksWriteAfterMDuringWrite020++;
         }

         else
         {
            
         }

         if (m1_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-M1]//////////////////////////
            if ((m_read_before_m1[index]+m_read_after_m1[index])>(sf*(m_write_before_m1[index]+m_write_after_m1[index])))
               readIntenseBlocksM1++;

            ///////////////////[WRITE-INTENSE-BLOCKS-M1]//////////////////////////
            else
               writeIntenseBlocksM1++;
         }
         
         if (mduringwrite_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            if ((m_read_before_mduringwrite[index]+m_read_after_mduringwrite[index])>(sf*(m_write_before_mduringwrite[index]+m_write_after_mduringwrite[index])))
               readIntenseBlocksMDuringWrite++;

            ///////////////////[WRITE-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            else
               writeIntenseBlocksMDuringWrite++;
         }
         
         

         




         ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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

         m_read_before_m1[index]  = 0;
         m_write_before_m1[index] = 0;  
         m_read_after_m1[index]   = 0; 
         m_write_after_m1[index]  = 0; 
         m1_flag[index]           = 0; 

         m_read_before_mduringwrite[index]  = 0;
         m_write_before_mduringwrite[index] = 0;  
         m_read_after_mduringwrite[index]   = 0; 
         m_write_after_mduringwrite[index]  = 0; 
         mduringwrite_flag[index]           = 0;  //If a block has been migrated this is set to 1
         
         validBlockEvicted++;

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

               
               write_array[i] = 0;  //reset the counters on eviction
               read_array[i] = 0;
               access_counter[i] = 0;
               m_deadblock[i] = 0;

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

            m_read_before_m1[i]  = 0;
            m_write_before_m1[i] = 0;  
            m_read_after_m1[i]   = 0; 
            m_write_after_m1[i]  = 0; 
            m1_flag[i]           = 0; 

            m_read_before_mduringwrite[i]  = 0;
            m_write_before_mduringwrite[i] = 0;  
            m_read_after_mduringwrite[i]   = 0; 
            m_write_after_mduringwrite[i]  = 0; 
            mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

         //Calculating Cminusmax and Cminusmin
         if(m_cost[index]<Cminusmin)
            Cminusmin = m_cost[index];
         if(m_cost[index]>Cminusmax)
            Cminusmax = m_cost[index];

         //Calculating Wmax and Wmin
         if(m_wcnt[m_TI[index]]<Wmin)
            Wmin = m_wcnt[m_TI[index]];
         if(m_wcnt[m_TI[index]]>Wmax)
            Wmax = m_wcnt[m_TI[index]];



         if(m_dcnt[m_TI[index]] != 255)
            m_dcnt[m_TI[index]]++;  //increment deadblock counter on eviction

         if(m_wcnt[m_TI[index]] != 0)
            m_wcnt[m_TI[index]]--;  //decrement write burst counter on eviction


         if((index<SRAM_ways) && (index>=0))
         {
            if(m_dcnt[m_TI[index]]<dcnt_threshold)
               livingSRAMBlocksEvicted++;
            else
               deadSRAMBlocksEvicted++;

         }

         if((m1_flag[index]==1) && (mduringwrite_flag[index]==1))
            doubleMigrationCount++;
         else if ((m1_flag[index]==0) && (mduringwrite_flag[index]==0))
            noMigrationCount++;
         else
            singleMigrationCount++;

         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         UInt64 percentageReadBeforeM1             =  0;
         UInt64 percentageWriteBeforeM1            =  0;

         UInt64 percentageReadBeforeMDuringWrite   =  0;
         UInt64 percentageWriteBeforeMDuringWrite  =  0;

         
         if (m_read_before_m1[index]!=0)
            percentageReadBeforeM1             =   ((100 * m_read_before_m1[index])/(m_read_before_m1[index] + m_read_after_m1[index]));
         if (m_write_before_m1[index]!=0)
            percentageWriteBeforeM1            =   ((100 * m_write_before_m1[index])/(m_write_before_m1[index] + m_write_after_m1[index]));

         if (m_read_before_mduringwrite[index]!=0)
            percentageReadBeforeMDuringWrite   =   ((100 * m_read_before_mduringwrite[index])/(m_read_before_mduringwrite[index] + m_read_after_mduringwrite[index])); 
         if (m_write_before_mduringwrite[index]!=0)
            percentageWriteBeforeMDuringWrite  =   ((100 * m_write_before_mduringwrite[index])/(m_write_before_mduringwrite[index] + m_write_after_mduringwrite[index]));

         
         ///////////////////[READ-M1]////////////////////////
         if ((percentageReadBeforeM1>=0)&&(percentageReadBeforeM1<20))
         {
            blocksReadBeforeM1020++;
            blocksReadAfterM180100++;
         }

         else if ((percentageReadBeforeM1>=20)&&(percentageReadBeforeM1<40))
         {
            blocksReadBeforeM12040++;
            blocksReadAfterM16080++;
         }

         else if ((percentageReadBeforeM1>=40)&&(percentageReadBeforeM1<60))
         {
            blocksReadBeforeM14060++;
            blocksReadAfterM14060++;
         }

         else if ((percentageReadBeforeM1>=60)&&(percentageReadBeforeM1<80))
         {
            blocksReadBeforeM16080++;
            blocksReadAfterM12040++;
         }

         else if ((percentageReadBeforeM1>=80)&&(percentageReadBeforeM1<=100))
         {
            blocksReadBeforeM180100++;
            blocksReadAfterM1020++;
         }

         else
         {

         }


         ///////////////////[READ-MDURINGWRITE]////////////////////////
         if ((percentageReadBeforeMDuringWrite>=0)&&(percentageReadBeforeMDuringWrite<20))
         {
            blocksReadBeforeMDuringWrite020++;
            blocksReadAfterMDuringWrite80100++;
         }

         else if ((percentageReadBeforeMDuringWrite>=20)&&(percentageReadBeforeMDuringWrite<40))
         {
            blocksReadBeforeMDuringWrite2040++;
            blocksReadAfterMDuringWrite6080++;
         }

         else if ((percentageReadBeforeMDuringWrite>=40)&&(percentageReadBeforeMDuringWrite<60))
         {
            blocksReadBeforeMDuringWrite4060++;
            blocksReadAfterMDuringWrite4060++;
         }

         else if ((percentageReadBeforeMDuringWrite>=60)&&(percentageReadBeforeMDuringWrite<80))
         {
            blocksReadBeforeMDuringWrite6080++;
            blocksReadAfterMDuringWrite2040++;
         }

         else if ((percentageReadBeforeMDuringWrite>=80)&&(percentageReadBeforeMDuringWrite<=100))
         {
            blocksReadBeforeMDuringWrite80100++;
            blocksReadAfterMDuringWrite020++;
         }

         else
         {
            
         }


         ///////////////////[WRITE-M1]////////////////////////

         if ((percentageWriteBeforeM1>=0)&&(percentageWriteBeforeM1<20))
         {
            blocksWriteBeforeM1020++;
            blocksWriteAfterM180100++;
         }

         else if ((percentageWriteBeforeM1>=20)&&(percentageWriteBeforeM1<40))
         {
            blocksWriteBeforeM12040++;
            blocksWriteAfterM16080++;
         }

         else if ((percentageWriteBeforeM1>=40)&&(percentageWriteBeforeM1<60))
         {
            blocksWriteBeforeM14060++;
            blocksWriteAfterM14060++;
         }

         else if ((percentageWriteBeforeM1>=60)&&(percentageWriteBeforeM1<80))
         {
            blocksWriteBeforeM16080++;
            blocksWriteAfterM12040++;
         }

         else if ((percentageWriteBeforeM1>=80)&&(percentageWriteBeforeM1<=100))
         {
            blocksWriteBeforeM180100++;
            blocksWriteAfterM1020++;
         }

         else
         {

         }


         ///////////////////[WRITE-MDURINGWRITE]////////////////////////
         if ((percentageWriteBeforeMDuringWrite>=0)&&(percentageWriteBeforeMDuringWrite<20))
         {
            blocksWriteBeforeMDuringWrite020++;
            blocksWriteAfterMDuringWrite80100++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=20)&&(percentageWriteBeforeMDuringWrite<40))
         {
            blocksWriteBeforeMDuringWrite2040++;
            blocksWriteAfterMDuringWrite6080++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=40)&&(percentageWriteBeforeMDuringWrite<60))
         {
            blocksWriteBeforeMDuringWrite4060++;
            blocksWriteAfterMDuringWrite4060++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=60)&&(percentageWriteBeforeMDuringWrite<80))
         {
            blocksWriteBeforeMDuringWrite6080++;
            blocksWriteAfterMDuringWrite2040++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=80)&&(percentageWriteBeforeMDuringWrite<=100))
         {
            blocksWriteBeforeMDuringWrite80100++;
            blocksWriteAfterMDuringWrite020++;
         }

         else
         {
            
         }

         if (m1_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-M1]//////////////////////////
            if ((m_read_before_m1[index]+m_read_after_m1[index])>(sf*(m_write_before_m1[index]+m_write_after_m1[index])))
               readIntenseBlocksM1++;

            ///////////////////[WRITE-INTENSE-BLOCKS-M1]//////////////////////////
            else
               writeIntenseBlocksM1++;
         }
         
         if (mduringwrite_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            if ((m_read_before_mduringwrite[index]+m_read_after_mduringwrite[index])>(sf*(m_write_before_mduringwrite[index]+m_write_after_mduringwrite[index])))
               readIntenseBlocksMDuringWrite++;

            ///////////////////[WRITE-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            else
               writeIntenseBlocksMDuringWrite++;
         }
         
         

         




         ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         

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

         m_read_before_m1[index]  = 0;
         m_write_before_m1[index] = 0;  
         m_read_after_m1[index]   = 0; 
         m_write_after_m1[index]  = 0; 
         m1_flag[index]           = 0; 

         m_read_before_mduringwrite[index]  = 0;
         m_write_before_mduringwrite[index] = 0;  
         m_read_after_mduringwrite[index]   = 0; 
         m_write_after_mduringwrite[index]  = 0; 
         mduringwrite_flag[index]           = 0;  //If a block has been migrated this is set to 1
         
         validBlockEvicted++;

         moveToMRU(index);
         m_set_info->incrementAttempt(attempt);


         return index;
         
      } 
        
   }


   else if ((set_index % sampler_fraction)==3)  //MSamplerSetDcntMin++;
   {
      MSamplerSetDcntMin++;
      //printf("MSamplerSetDcntMin is %d\n", MSamplerSetDcntMin);

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

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

            m_read_before_m1[i]  = 0;
            m_write_before_m1[i] = 0;  
            m_read_after_m1[i]   = 0; 
            m_write_after_m1[i]  = 0; 
            m1_flag[i]           = 0; 

            m_read_before_mduringwrite[i]  = 0;
            m_write_before_mduringwrite[i] = 0;  
            m_read_after_mduringwrite[i]   = 0; 
            m_write_after_mduringwrite[i]  = 0; 
            mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
            moveToMRU(i);
            //printf("invalid block found, but not in proper partition\n");
            return i;
         }
      }
      


      //INVALID BLOCK NOT FOUND
      validBlockEvicted++;
      // Make m_num_attemps attempts at evicting the block at LRU position
      for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt) //returns index
      {


         UInt32 index = 0;
         UInt8 max_bits = 0;

         UInt32 forceMigrationIndex;

         if(m_state[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
         {
            
            bool checkFlag2 = false;
            UInt8 max_dcnt2 = 0;

            index = SRAM_ways; 

            for (UInt32 i = SRAM_ways; i < m_associativity; i++)  //checking for  deadblock in STTRAM
            {
               if ((m_dcnt[m_TI[i]] > max_dcnt2) && (isDeadBlock(i, set_index)) && (isValidReplacement(i)))
               {
                  index = i;
                  max_dcnt2 = m_dcnt[m_TI[i]];
                  checkFlag2 = true;
               }
            }

            if(!checkFlag2)   //No valid deadblock found in STTRAM. Select LRU block from STTRAM as victim
            {
            
               ///////////////////////////////
               index = SRAM_ways;
               for (UInt32 i = SRAM_ways; i < m_associativity; i++)
               {
                  if (m_lru_bits[i] > max_bits && isValidReplacement(i))
                  {
                     index = i;
                     max_bits = m_lru_bits[i];
                  }
               }               
               ///////////////////////////////

            }
            
         }
   
         else  //TI is hot. select victim from SRAM
         {   
            
            ///////////////////////////////////////////////////////////////////////////////////////////

            bool checkFlag=false;
            UInt8 max_dcnt = 0;

            index = 0;

            for (UInt32 i = 0; i < SRAM_ways; i++) //checking for deadblock in  SRAM partition
            {
               if ((m_dcnt[m_TI[i]] > max_dcnt) && (isDeadBlock(i, set_index)) && (isValidReplacement(i)))
               {
                  index = i;
                  max_dcnt = m_dcnt[m_TI[i]];
                  checkFlag = true;
               }
            }

            if(!checkFlag) //valid deadblock not found in SRAM. Select LRU block from SRAM as victim
            {

               index = 0;
               for (UInt32 i = 0; i < SRAM_ways; i++)
               {
                  if (m_lru_bits[i] > max_bits && isValidReplacement(i))
                  {
                     index = i;
                     max_bits = m_lru_bits[i];
                  }
               }               
            }

            /////////////////////////////////////////////////////////////////////////////////////////

         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");


         
         //////////////////////////////////////////////////////////////////

         bool deadblockInSRAMFlag=false;
         bool deadblockInSTTRFlag=false;

         bool deadBlockInBothPartition=false;
         bool deadBlockInNeitherPartition=false;
         bool deadBlockInSRAMOnly=false;
         bool deadBlockInSTTROnly=false;



         //checking deadblock///
         for(UInt32 iter=0; iter<SRAM_ways; iter++)
         {
            if(isDeadBlock(iter, set_index))
               deadblockInSRAMFlag=true;

         }
         for(UInt32 iter=SRAM_ways; iter<m_associativity; iter++)
         {
            if(isDeadBlock(iter, set_index))
               deadblockInSTTRFlag=true;
            
         }
 
         if ((deadblockInSRAMFlag) && (deadblockInSTTRFlag))
            deadBlockInBothPartition=true;
         else if ((!deadblockInSRAMFlag) && (!deadblockInSTTRFlag))
            deadBlockInNeitherPartition=true;
         else if ((deadblockInSRAMFlag) && (!deadblockInSTTRFlag))
            deadBlockInSRAMOnly=true;
         else if ((!deadblockInSRAMFlag) && (deadblockInSTTRFlag))
            deadBlockInSTTROnly=true;
         

         if ((index>=0) && (index<SRAM_ways))
         {
            validSRAMEvictions++;

            if(deadBlockInBothPartition)
            {

               if(isDeadBlock(index, set_index))
                  d0++;
               else
                  e0++;
                           
            }
            else if(deadBlockInNeitherPartition)
            {
               UInt32 indexLocal = 0;
               UInt8 max_bits_local = 0;

               for (UInt32 iter2 = 0; iter2 < m_associativity; iter2++)
               {
                  //if (m_lru_bits_unified[iter2] > max_bits_local) && (isValidReplacement(iter2))
                  if ((m_lru_bits_unified[iter2] > max_bits_local) && (isValidReplacement(iter2)))
                  {
                     indexLocal = iter2;
                     max_bits_local = m_lru_bits_unified[iter2];
                  }
               }

               if((indexLocal>=SRAM_ways) && (indexLocal<m_associativity))
                  g0++;
               else 
                  f0++;

            }
            else if(deadBlockInSRAMOnly)
            {
               if(deadBlockInBothPartition)
               {
                  if(isDeadBlock(index, set_index))
                     b0++;
                  else
                     c0++;
               }

            }
            else if(deadBlockInSTTROnly)
            {
               a0++;
            }

         }



         else if((index>=SRAM_ways) && (index<m_associativity))
         {
            validSTTREvictions++;

            if(deadBlockInBothPartition)
            {
               if(isDeadBlock(index, set_index))
                  d1++;
               else
                  e1++;
            }
            else if(deadBlockInNeitherPartition)
            {

               UInt32 indexLocal = 0;
               UInt8 max_bits_local = 0;

               for (UInt32 iter2 = 0; iter2 < m_associativity; iter2++)
               {
                  //if (m_lru_bits_unified[iter2] > max_bits_local && isValidReplacement(iter2))
                  if ((m_lru_bits_unified[iter2] > max_bits_local) && (isValidReplacement(iter2)))
                  {
                     indexLocal = iter2;
                     max_bits_local = m_lru_bits_unified[iter2];
                  }
               }

               if((indexLocal>=0) && (indexLocal<SRAM_ways))
                  g1++;
               else 
                  f1++;
            }
            else if(deadBlockInSRAMOnly)
            {
               a1++;
            }
            else if(deadBlockInSTTROnly)
            {
               if(deadBlockInBothPartition)
               {
                  if(isDeadBlock(index, set_index))
                     b1++;
                  else
                     c1++;
               }
            }
         }

         //////////////////////////////////////////////////////////////////////////////////

         

   
            
         if((m_cost[index]>K) && (m_state[m_TI[index]]<state_max)) //state should be incremented
            m_state[m_TI[index]]++;
         else if((m_cost[index]<K) && (m_state[m_TI[index]]>0))
            m_state[m_TI[index]]--;         
         
         //check if the victim block is read intense, or write intense or deadblock

         if(m_dcnt[m_TI[index]]>=dcnt_threshold)   //deadblock
            deadblock_counter++;
         else if((m_dcnt[m_TI[index]]<dcnt_threshold)&&(m_cost[index]<K))  //read_intense
            read_intense_block_counter++;
         else if((m_dcnt[m_TI[index]]<dcnt_threshold)&&(m_cost[index]>=K))  //write_intense
            write_intense_block_counter++;
         else
            printf("ERROR!!!!!!\n");


         //Calculating Cmax and Cmin
         if(m_cost[index]<Cmin)
            Cmin = m_cost[index];
         if(m_cost[index]>Cmax)
            Cmax = m_cost[index];



         //Calculating Wmax and Wmin
         if(m_wcnt[m_TI[index]]<Wmin)
            Wmin = m_wcnt[m_TI[index]];
         if(m_wcnt[m_TI[index]]>Wmax)
            Wmax = m_wcnt[m_TI[index]];



         if(m_dcnt[m_TI[index]] != 255)
            m_dcnt[m_TI[index]]++;  //increment deadblock counter on eviction

         if(m_wcnt[m_TI[index]] != 0)
            m_wcnt[m_TI[index]]--;  //decrement write burst counter on eviction

         if((index<SRAM_ways) && (index>=0))
         {
            if(m_dcnt[m_TI[index]]<dcnt_threshold)
               livingSRAMBlocksEvicted++;
            else
               deadSRAMBlocksEvicted++;

         }

         if((m1_flag[index]==1) && (mduringwrite_flag[index]==1))
            doubleMigrationCount++;
         else if ((m1_flag[index]==0) && (mduringwrite_flag[index]==0))
            noMigrationCount++;
         else
            singleMigrationCount++;

         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         
         UInt64 percentageReadBeforeM1             =  0;
         UInt64 percentageWriteBeforeM1            =  0;

         UInt64 percentageReadBeforeMDuringWrite   =  0;
         UInt64 percentageWriteBeforeMDuringWrite  =  0;

         
         if (m_read_before_m1[index]!=0)
            percentageReadBeforeM1             =   ((100 * m_read_before_m1[index])/(m_read_before_m1[index] + m_read_after_m1[index]));
         if (m_write_before_m1[index]!=0)
            percentageWriteBeforeM1            =   ((100 * m_write_before_m1[index])/(m_write_before_m1[index] + m_write_after_m1[index]));

         if (m_read_before_mduringwrite[index]!=0)
            percentageReadBeforeMDuringWrite   =   ((100 * m_read_before_mduringwrite[index])/(m_read_before_mduringwrite[index] + m_read_after_mduringwrite[index])); 
         if (m_write_before_mduringwrite[index]!=0)
            percentageWriteBeforeMDuringWrite  =   ((100 * m_write_before_mduringwrite[index])/(m_write_before_mduringwrite[index] + m_write_after_mduringwrite[index]));
         

         ///////////////////[READ-M1]////////////////////////
         if ((percentageReadBeforeM1>=0)&&(percentageReadBeforeM1<20))
         {
            blocksReadBeforeM1020++;
            blocksReadAfterM180100++;
         }

         else if ((percentageReadBeforeM1>=20)&&(percentageReadBeforeM1<40))
         {
            blocksReadBeforeM12040++;
            blocksReadAfterM16080++;
         }

         else if ((percentageReadBeforeM1>=40)&&(percentageReadBeforeM1<60))
         {
            blocksReadBeforeM14060++;
            blocksReadAfterM14060++;
         }

         else if ((percentageReadBeforeM1>=60)&&(percentageReadBeforeM1<80))
         {
            blocksReadBeforeM16080++;
            blocksReadAfterM12040++;
         }

         else if ((percentageReadBeforeM1>=80)&&(percentageReadBeforeM1<=100))
         {

            blocksReadBeforeM180100++;
            blocksReadAfterM1020++;
         }

         else
         {

         }


         ///////////////////[READ-MDURINGWRITE]////////////////////////
         if ((percentageReadBeforeMDuringWrite>=0)&&(percentageReadBeforeMDuringWrite<20))
         {
            blocksReadBeforeMDuringWrite020++;
            blocksReadAfterMDuringWrite80100++;
         }

         else if ((percentageReadBeforeMDuringWrite>=20)&&(percentageReadBeforeMDuringWrite<40))
         {
            blocksReadBeforeMDuringWrite2040++;
            blocksReadAfterMDuringWrite6080++;
         }

         else if ((percentageReadBeforeMDuringWrite>=40)&&(percentageReadBeforeMDuringWrite<60))
         {
            blocksReadBeforeMDuringWrite4060++;
            blocksReadAfterMDuringWrite4060++;
         }

         else if ((percentageReadBeforeMDuringWrite>=60)&&(percentageReadBeforeMDuringWrite<80))
         {
            blocksReadBeforeMDuringWrite6080++;
            blocksReadAfterMDuringWrite2040++;
         }

         else if ((percentageReadBeforeMDuringWrite>=80)&&(percentageReadBeforeMDuringWrite<=100))
         {
            blocksReadBeforeMDuringWrite80100++;
            blocksReadAfterMDuringWrite020++;
         }

         else
         {
            
         }


         ///////////////////[WRITE-M1]////////////////////////

         if ((percentageWriteBeforeM1>=0)&&(percentageWriteBeforeM1<20))
         {
            blocksWriteBeforeM1020++;
            blocksWriteAfterM180100++;
         }

         else if ((percentageWriteBeforeM1>=20)&&(percentageWriteBeforeM1<40))
         {
            blocksWriteBeforeM12040++;
            blocksWriteAfterM16080++;
         }

         else if ((percentageWriteBeforeM1>=40)&&(percentageWriteBeforeM1<60))
         {
            blocksWriteBeforeM14060++;
            blocksWriteAfterM14060++;
         }

         else if ((percentageWriteBeforeM1>=60)&&(percentageWriteBeforeM1<80))
         {
            blocksWriteBeforeM16080++;
            blocksWriteAfterM12040++;
         }

         else if ((percentageWriteBeforeM1>=80)&&(percentageWriteBeforeM1<=100))
         {
            blocksWriteBeforeM180100++;
            blocksWriteAfterM1020++;
         }

         else
         {

         }


         ///////////////////[WRITE-MDURINGWRITE]////////////////////////
         if ((percentageWriteBeforeMDuringWrite>=0)&&(percentageWriteBeforeMDuringWrite<20))
         {
            blocksWriteBeforeMDuringWrite020++;
            blocksWriteAfterMDuringWrite80100++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=20)&&(percentageWriteBeforeMDuringWrite<40))
         {
            blocksWriteBeforeMDuringWrite2040++;
            blocksWriteAfterMDuringWrite6080++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=40)&&(percentageWriteBeforeMDuringWrite<60))
         {
            blocksWriteBeforeMDuringWrite4060++;
            blocksWriteAfterMDuringWrite4060++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=60)&&(percentageWriteBeforeMDuringWrite<80))
         {
            blocksWriteBeforeMDuringWrite6080++;
            blocksWriteAfterMDuringWrite2040++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=80)&&(percentageWriteBeforeMDuringWrite<=100))
         {
            blocksWriteBeforeMDuringWrite80100++;
            blocksWriteAfterMDuringWrite020++;
         }

         else
         {
            
         }

         if (m1_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-M1]//////////////////////////
            if ((m_read_before_m1[index]+m_read_after_m1[index])>(sf*(m_write_before_m1[index]+m_write_after_m1[index])))
               readIntenseBlocksM1++;

            ///////////////////[WRITE-INTENSE-BLOCKS-M1]//////////////////////////
            else
               writeIntenseBlocksM1++;
         }
         
         if (mduringwrite_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            if ((m_read_before_mduringwrite[index]+m_read_after_mduringwrite[index])>(sf*(m_write_before_mduringwrite[index]+m_write_after_mduringwrite[index])))
               readIntenseBlocksMDuringWrite++;

            ///////////////////[WRITE-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            else
               writeIntenseBlocksMDuringWrite++;
         }
         
         
         ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         
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

         m_read_before_m1[index]  = 0;
         m_write_before_m1[index] = 0;  
         m_read_after_m1[index]   = 0; 
         m_write_after_m1[index]  = 0; 
         m1_flag[index]           = 0; 

         m_read_before_mduringwrite[index]  = 0;
         m_write_before_mduringwrite[index] = 0;  
         m_read_after_mduringwrite[index]   = 0; 
         m_write_after_mduringwrite[index]  = 0; 
         mduringwrite_flag[index]           = 0;  //If a block has been migrated this is set to 1     

         validBlockEvicted++;

         // Mark our newly-inserted line as most-recently used
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt); 
       

         return index;
         
      } 
   }


   else if ((set_index % sampler_fraction)==4)  //MSamplerSetDcntPlu++;
   {
      MSamplerSetDcntPlu++;
      //printf("MSamplerSetDcntPlu is %d\n", MSamplerSetDcntPlu);

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

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

            m_read_before_m1[i]  = 0;
            m_write_before_m1[i] = 0;  
            m_read_after_m1[i]   = 0; 
            m_write_after_m1[i]  = 0; 
            m1_flag[i]           = 0; 

            m_read_before_mduringwrite[i]  = 0;
            m_write_before_mduringwrite[i] = 0;  
            m_read_after_mduringwrite[i]   = 0; 
            m_write_after_mduringwrite[i]  = 0; 
            mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
            moveToMRU(i);
            //printf("invalid block found, but not in proper partition\n");
            return i;
         }
      }
      


      //INVALID BLOCK NOT FOUND
      validBlockEvicted++;
      // Make m_num_attemps attempts at evicting the block at LRU position
      for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt) //returns index
      {


         UInt32 index = 0;
         UInt8 max_bits = 0;

         UInt32 forceMigrationIndex;

         if(m_state[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
         {
            
            bool checkFlag2 = false;
            UInt8 max_dcnt2 = 0;

            index = SRAM_ways; 

            for (UInt32 i = SRAM_ways; i < m_associativity; i++)  //checking for  deadblock in STTRAM
            {
               if ((m_dcnt[m_TI[i]] > max_dcnt2) && (isDeadBlock(i, set_index)) && (isValidReplacement(i)))
               {
                  index = i;
                  max_dcnt2 = m_dcnt[m_TI[i]];
                  checkFlag2 = true;
               }
            }

            if(!checkFlag2)   //No valid deadblock found in STTRAM. Select LRU block from STTRAM as victim
            {
               ///////////////////////////////
               index = SRAM_ways;
               for (UInt32 i = SRAM_ways; i < m_associativity; i++)
               {
                  if (m_lru_bits[i] > max_bits && isValidReplacement(i))
                  {
                     index = i;
                     max_bits = m_lru_bits[i];
                  }
               }               
               ///////////////////////////////
            }
            
         }
   
         else  //TI is hot. select victim from SRAM
         {   
            
            ///////////////////////////////////////////////////////////////////////////////////////////

            bool checkFlag=false;
            UInt8 max_dcnt = 0;

            index = 0;

            for (UInt32 i = 0; i < SRAM_ways; i++) //checking for deadblock in  SRAM partition
            {
               if ((m_dcnt[m_TI[i]] > max_dcnt) && (isDeadBlock(i, set_index)) && (isValidReplacement(i)))
               {
                  index = i;
                  max_dcnt = m_dcnt[m_TI[i]];
                  checkFlag = true;
               }
            }

            if(!checkFlag) //valid deadblock not found in SRAM. Se;ect LRU from SRAM as victim
            {

               index = 0;
               for (UInt32 i = 0; i < SRAM_ways; i++)
               {
                  if (m_lru_bits[i] > max_bits && isValidReplacement(i))
                  {
                     index = i;
                     max_bits = m_lru_bits[i];
                  }
               }               
            }

            /////////////////////////////////////////////////////////////////////////////////////////

         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");


         
         //////////////////////////////////////////////////////////////////

         bool deadblockInSRAMFlag=false;
         bool deadblockInSTTRFlag=false;

         bool deadBlockInBothPartition=false;
         bool deadBlockInNeitherPartition=false;
         bool deadBlockInSRAMOnly=false;
         bool deadBlockInSTTROnly=false;



         //checking deadblock///
         for(UInt32 iter=0; iter<SRAM_ways; iter++)
         {
            if(isDeadBlock(iter, set_index))
               deadblockInSRAMFlag=true;

         }
         for(UInt32 iter=SRAM_ways; iter<m_associativity; iter++)
         {
            if(isDeadBlock(iter, set_index))
               deadblockInSTTRFlag=true;
            
         }
 
         if ((deadblockInSRAMFlag) && (deadblockInSTTRFlag))
            deadBlockInBothPartition=true;
         else if ((!deadblockInSRAMFlag) && (!deadblockInSTTRFlag))
            deadBlockInNeitherPartition=true;
         else if ((deadblockInSRAMFlag) && (!deadblockInSTTRFlag))
            deadBlockInSRAMOnly=true;
         else if ((!deadblockInSRAMFlag) && (deadblockInSTTRFlag))
            deadBlockInSTTROnly=true;
         

         if ((index>=0) && (index<SRAM_ways))
         {
            validSRAMEvictions++;

            if(deadBlockInBothPartition)
            {

               if(isDeadBlock(index, set_index))
                  d0++;
               else
                  e0++;
                           
            }
            else if(deadBlockInNeitherPartition)
            {
               UInt32 indexLocal = 0;
               UInt8 max_bits_local = 0;

               for (UInt32 iter2 = 0; iter2 < m_associativity; iter2++)
               {
                  //if (m_lru_bits_unified[iter2] > max_bits_local) && (isValidReplacement(iter2))
                  if ((m_lru_bits_unified[iter2] > max_bits_local) && (isValidReplacement(iter2)))
                  {
                     indexLocal = iter2;
                     max_bits_local = m_lru_bits_unified[iter2];
                  }
               }

               if((indexLocal>=SRAM_ways) && (indexLocal<m_associativity))
                  g0++;
               else 
                  f0++;

            }
            else if(deadBlockInSRAMOnly)
            {
               if(deadBlockInBothPartition)
               {
                  if(isDeadBlock(index, set_index))
                     b0++;
                  else
                     c0++;
               }

            }
            else if(deadBlockInSTTROnly)
            {
               a0++;
            }

         }



         else if((index>=SRAM_ways) && (index<m_associativity))
         {
            validSTTREvictions++;

            if(deadBlockInBothPartition)
            {
               if(isDeadBlock(index, set_index))
                  d1++;
               else
                  e1++;
            }
            else if(deadBlockInNeitherPartition)
            {

               UInt32 indexLocal = 0;
               UInt8 max_bits_local = 0;

               for (UInt32 iter2 = 0; iter2 < m_associativity; iter2++)
               {
                  //if (m_lru_bits_unified[iter2] > max_bits_local && isValidReplacement(iter2))
                  if ((m_lru_bits_unified[iter2] > max_bits_local) && (isValidReplacement(iter2)))
                  {
                     indexLocal = iter2;
                     max_bits_local = m_lru_bits_unified[iter2];
                  }
               }

               if((indexLocal>=0) && (indexLocal<SRAM_ways))
                  g1++;
               else 
                  f1++;
            }
            else if(deadBlockInSRAMOnly)
            {
               a1++;
            }
            else if(deadBlockInSTTROnly)
            {
               if(deadBlockInBothPartition)
               {
                  if(isDeadBlock(index, set_index))
                     b1++;
                  else
                     c1++;
               }
            }
         }

         //////////////////////////////////////////////////////////////////////////////////

         

   
            
         if((m_cost[index]>K) && (m_state[m_TI[index]]<state_max)) //state should be incremented
            m_state[m_TI[index]]++;
         else if((m_cost[index]<K) && (m_state[m_TI[index]]>0))
            m_state[m_TI[index]]--;         
         
         //check if the victim block is read intense, or write intense or deadblock

         if(m_dcnt[m_TI[index]]>=dcnt_threshold)   //deadblock
            deadblock_counter++;
         else if((m_dcnt[m_TI[index]]<dcnt_threshold)&&(m_cost[index]<K))  //read_intense
            read_intense_block_counter++;
         else if((m_dcnt[m_TI[index]]<dcnt_threshold)&&(m_cost[index]>=K))  //write_intense
            write_intense_block_counter++;
         else
            printf("ERROR!!!!!!\n");


         //Calculating Cmax and Cmin
         if(m_cost[index]<Cmin)
            Cmin = m_cost[index];
         if(m_cost[index]>Cmax)
            Cmax = m_cost[index];



         //Calculating Wmax and Wmin
         if(m_wcnt[m_TI[index]]<Wmin)
            Wmin = m_wcnt[m_TI[index]];
         if(m_wcnt[m_TI[index]]>Wmax)
            Wmax = m_wcnt[m_TI[index]];



         if(m_dcnt[m_TI[index]] != 255)
            m_dcnt[m_TI[index]]++;  //increment deadblock counter on eviction

         if(m_wcnt[m_TI[index]] != 0)
            m_wcnt[m_TI[index]]--;  //decrement write burst counter on eviction

         if((index<SRAM_ways) && (index>=0))
         {
            if(m_dcnt[m_TI[index]]<dcnt_threshold)
               livingSRAMBlocksEvicted++;
            else
               deadSRAMBlocksEvicted++;

         }

         if((m1_flag[index]==1) && (mduringwrite_flag[index]==1))
            doubleMigrationCount++;
         else if ((m1_flag[index]==0) && (mduringwrite_flag[index]==0))
            noMigrationCount++;
         else
            singleMigrationCount++;

         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         
         UInt64 percentageReadBeforeM1             =  0;
         UInt64 percentageWriteBeforeM1            =  0;

         UInt64 percentageReadBeforeMDuringWrite   =  0;
         UInt64 percentageWriteBeforeMDuringWrite  =  0;

         
         if (m_read_before_m1[index]!=0)
            percentageReadBeforeM1             =   ((100 * m_read_before_m1[index])/(m_read_before_m1[index] + m_read_after_m1[index]));
         if (m_write_before_m1[index]!=0)
            percentageWriteBeforeM1            =   ((100 * m_write_before_m1[index])/(m_write_before_m1[index] + m_write_after_m1[index]));

         if (m_read_before_mduringwrite[index]!=0)
            percentageReadBeforeMDuringWrite   =   ((100 * m_read_before_mduringwrite[index])/(m_read_before_mduringwrite[index] + m_read_after_mduringwrite[index])); 
         if (m_write_before_mduringwrite[index]!=0)
            percentageWriteBeforeMDuringWrite  =   ((100 * m_write_before_mduringwrite[index])/(m_write_before_mduringwrite[index] + m_write_after_mduringwrite[index]));
         

         ///////////////////[READ-M1]////////////////////////
         if ((percentageReadBeforeM1>=0)&&(percentageReadBeforeM1<20))
         {
            blocksReadBeforeM1020++;
            blocksReadAfterM180100++;
         }

         else if ((percentageReadBeforeM1>=20)&&(percentageReadBeforeM1<40))
         {
            blocksReadBeforeM12040++;
            blocksReadAfterM16080++;
         }

         else if ((percentageReadBeforeM1>=40)&&(percentageReadBeforeM1<60))
         {
            blocksReadBeforeM14060++;
            blocksReadAfterM14060++;
         }

         else if ((percentageReadBeforeM1>=60)&&(percentageReadBeforeM1<80))
         {
            blocksReadBeforeM16080++;
            blocksReadAfterM12040++;
         }

         else if ((percentageReadBeforeM1>=80)&&(percentageReadBeforeM1<=100))
         {

            blocksReadBeforeM180100++;
            blocksReadAfterM1020++;
         }

         else
         {

         }


         ///////////////////[READ-MDURINGWRITE]////////////////////////
         if ((percentageReadBeforeMDuringWrite>=0)&&(percentageReadBeforeMDuringWrite<20))
         {
            blocksReadBeforeMDuringWrite020++;
            blocksReadAfterMDuringWrite80100++;
         }

         else if ((percentageReadBeforeMDuringWrite>=20)&&(percentageReadBeforeMDuringWrite<40))
         {
            blocksReadBeforeMDuringWrite2040++;
            blocksReadAfterMDuringWrite6080++;
         }

         else if ((percentageReadBeforeMDuringWrite>=40)&&(percentageReadBeforeMDuringWrite<60))
         {
            blocksReadBeforeMDuringWrite4060++;
            blocksReadAfterMDuringWrite4060++;
         }

         else if ((percentageReadBeforeMDuringWrite>=60)&&(percentageReadBeforeMDuringWrite<80))
         {
            blocksReadBeforeMDuringWrite6080++;
            blocksReadAfterMDuringWrite2040++;
         }

         else if ((percentageReadBeforeMDuringWrite>=80)&&(percentageReadBeforeMDuringWrite<=100))
         {
            blocksReadBeforeMDuringWrite80100++;
            blocksReadAfterMDuringWrite020++;
         }

         else
         {
            
         }


         ///////////////////[WRITE-M1]////////////////////////

         if ((percentageWriteBeforeM1>=0)&&(percentageWriteBeforeM1<20))
         {
            blocksWriteBeforeM1020++;
            blocksWriteAfterM180100++;
         }

         else if ((percentageWriteBeforeM1>=20)&&(percentageWriteBeforeM1<40))
         {
            blocksWriteBeforeM12040++;
            blocksWriteAfterM16080++;
         }

         else if ((percentageWriteBeforeM1>=40)&&(percentageWriteBeforeM1<60))
         {
            blocksWriteBeforeM14060++;
            blocksWriteAfterM14060++;
         }

         else if ((percentageWriteBeforeM1>=60)&&(percentageWriteBeforeM1<80))
         {
            blocksWriteBeforeM16080++;
            blocksWriteAfterM12040++;
         }

         else if ((percentageWriteBeforeM1>=80)&&(percentageWriteBeforeM1<=100))
         {
            blocksWriteBeforeM180100++;
            blocksWriteAfterM1020++;
         }

         else
         {

         }


         ///////////////////[WRITE-MDURINGWRITE]////////////////////////
         if ((percentageWriteBeforeMDuringWrite>=0)&&(percentageWriteBeforeMDuringWrite<20))
         {
            blocksWriteBeforeMDuringWrite020++;
            blocksWriteAfterMDuringWrite80100++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=20)&&(percentageWriteBeforeMDuringWrite<40))
         {
            blocksWriteBeforeMDuringWrite2040++;
            blocksWriteAfterMDuringWrite6080++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=40)&&(percentageWriteBeforeMDuringWrite<60))
         {
            blocksWriteBeforeMDuringWrite4060++;
            blocksWriteAfterMDuringWrite4060++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=60)&&(percentageWriteBeforeMDuringWrite<80))
         {
            blocksWriteBeforeMDuringWrite6080++;
            blocksWriteAfterMDuringWrite2040++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=80)&&(percentageWriteBeforeMDuringWrite<=100))
         {
            blocksWriteBeforeMDuringWrite80100++;
            blocksWriteAfterMDuringWrite020++;
         }

         else
         {
            
         }

         if (m1_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-M1]//////////////////////////
            if ((m_read_before_m1[index]+m_read_after_m1[index])>(sf*(m_write_before_m1[index]+m_write_after_m1[index])))
               readIntenseBlocksM1++;

            ///////////////////[WRITE-INTENSE-BLOCKS-M1]//////////////////////////
            else
               writeIntenseBlocksM1++;
         }
         
         if (mduringwrite_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            if ((m_read_before_mduringwrite[index]+m_read_after_mduringwrite[index])>(sf*(m_write_before_mduringwrite[index]+m_write_after_mduringwrite[index])))
               readIntenseBlocksMDuringWrite++;

            ///////////////////[WRITE-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            else
               writeIntenseBlocksMDuringWrite++;
         }
         
         
         ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         
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

         m_read_before_m1[index]  = 0;
         m_write_before_m1[index] = 0;  
         m_read_after_m1[index]   = 0; 
         m_write_after_m1[index]  = 0; 
         m1_flag[index]           = 0; 

         m_read_before_mduringwrite[index]  = 0;
         m_write_before_mduringwrite[index] = 0;  
         m_read_after_mduringwrite[index]   = 0; 
         m_write_after_mduringwrite[index]  = 0; 
         mduringwrite_flag[index]           = 0;  //If a block has been migrated this is set to 1     

         validBlockEvicted++;

         // Mark our newly-inserted line as most-recently used
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt); 
       

         return index;
         
      } 
   }


   else //non-sampler set, uses phc
   {  
      //printf("non-sampler\n");
      if((set_index % sampler_fraction)==7)  //sampler set for calculating misses on current threshold
      {
         M0++;
         Wcnt0++;
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

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

               m_read_before_m1[i]  = 0;
               m_write_before_m1[i] = 0;  
               m_read_after_m1[i]   = 0; 
               m_write_after_m1[i]  = 0; 
               m1_flag[i]           = 0; 

               m_read_before_mduringwrite[i]  = 0;
               m_write_before_mduringwrite[i] = 0;  
               m_read_after_mduringwrite[i]   = 0; 
               m_write_after_mduringwrite[i]  = 0; 
               mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
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

            m_read_before_m1[i]  = 0;
            m_write_before_m1[i] = 0;  
            m_read_after_m1[i]   = 0; 
            m_write_after_m1[i]  = 0; 
            m1_flag[i]           = 0; 

            m_read_before_mduringwrite[i]  = 0;
            m_write_before_mduringwrite[i] = 0;  
            m_read_after_mduringwrite[i]   = 0; 
            m_write_after_mduringwrite[i]  = 0; 
            mduringwrite_flag[i]           = 0;  //If a block has been migrated this is set to 1
               
            moveToMRU(i);
            //printf("invalid block found, but not in proper partition\n");
            return i;
         }
      }
      


      //INVALID BLOCK NOT FOUND
      validBlockEvicted++;
      // Make m_num_attemps attempts at evicting the block at LRU position
      for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt) //returns index
      {


         UInt32 index = 0;
         UInt8 max_bits = 0;

         UInt32 forceMigrationIndex;

         if(m_state[eip_truncated]<state_threshold)   //TI is cold. select victim from STTRAM
         {
            
            bool checkFlag2 = false;
            UInt8 max_dcnt2 = 0;

            index = SRAM_ways; 

            for (UInt32 i = SRAM_ways; i < m_associativity; i++)  //checking for  deadblock in STTRAM
            {
               if ((m_dcnt[m_TI[i]] > max_dcnt2) && (isDeadBlock(i, set_index)) && (isValidReplacement(i)))
               {
                  index = i;
                  max_dcnt2 = m_dcnt[m_TI[i]];
                  checkFlag2 = true;
               }
            }
            
            if(!checkFlag2)   //No valid deadblock found in STTRAM. Select LRU block in STTRAM as victim
            {
               /*
               forceMigrationIndex = 0;
               bool checkFlag3 = false;

               for (UInt32 i = 0; i < SRAM_ways; i++)
               {
                  if ((m_dcnt[m_TI[i]] > max_dcnt2) && (isDeadBlock(i, set_index)) && (isValidReplacement(i)))
                  {
                     forceMigrationIndex = i;
                     max_dcnt2 = m_dcnt[m_TI[i]];
                     checkFlag3 = true;
                  }
               }
               
               if(checkFlag3) //valid deadblock found in SRAM. find out a victim from STTRAM. swap it with forceMigrationIndex from SRAM
               {

                  index =  SRAM_ways;
                  
                  //Most write intensive block in STTRAM
                  UInt8 maxState = 0;
                  for (UInt32 i = SRAM_ways; i < m_associativity; i++)
                  {
                     if ((m_state[m_TI[i]] > maxState) && (isValidReplacement(i)))
                     {
                        index = i;
                        maxState = m_state[m_TI[i]];
                     }
                  }
                  
                  //swap it with forceMigrationIndex from SRAM
                  swapTwoBlocks(index, forceMigrationIndex);
                  forceMigrationSTTramToSram++;
               }

               else  //valid deadblock not found in SRAM. Proceed with baseline PHC. Will return index
               {
               */   
                ///////////////////////////////
               index = SRAM_ways;
               for (UInt32 i = SRAM_ways; i < m_associativity; i++)
               {
                  if (m_lru_bits[i] > max_bits && isValidReplacement(i))
                  {
                     index = i;
                     max_bits = m_lru_bits[i];
                  }
               }               
                ///////////////////////////////

            }
            
         }
   
         else  //TI is hot. select victim from SRAM
         {   
            
            ///////////////////////////////////////////////////////////////////////////////////////////

            bool checkFlag=false;
            UInt8 max_dcnt = 0;

            index = 0;
            
            for (UInt32 i = 0; i < SRAM_ways; i++) //checking for deadblock in  SRAM partition
            {
               if ((m_dcnt[m_TI[i]] > max_dcnt) && (isDeadBlock(i, set_index)) && (isValidReplacement(i)))
               {
                  index = i;
                  max_dcnt = m_dcnt[m_TI[i]];
                  checkFlag = true;
               }
            }

            if(!checkFlag) //valid deadblock not found in SRAM. Select LRU block in SRAM as victim
            {
               /*
               forceMigrationIndex = SRAM_ways; 
               bool checkFlag1=false; 

               for (UInt32 i = SRAM_ways; i < m_associativity; i++) //checking for deadblock in  STTRAM partition
               {
                  if ((m_dcnt[m_TI[i]] > max_dcnt) && (isDeadBlock(i, set_index)) && (isValidReplacement(i)))
                  {  
                     forceMigrationIndex = i;
                     max_dcnt = m_dcnt[m_TI[i]];
                     checkFlag1 = true;
                  }
               }

               if(checkFlag1)   //valid deadblock found in STTRAM. find out a victim from SRAM. swap it with forceMigrationIndex block from STTRAM
               {

                  index = 0;

                  //Most Read Intense Block in SRAM
                  UInt8 minState = 255;
                  for (UInt32 i = 0; i < SRAM_ways; i++)
                  {
                     if ((m_state[m_TI[i]] < minState) && (isValidReplacement(i)))
                     {
                        index = i;
                        minState = m_state[m_TI[i]];
                     }
                  }
                  

                  //swap it with forceMigrationIndex block from STTRAM
                  swapTwoBlocks(index, forceMigrationIndex);
                  forceMigrationSramToSTTram++;
               }
               
               else  //valid deadblock not found in STTRAM. Proceed with baseline PHC. Will return index
               {
               */
               index = 0;
               for (UInt32 i = 0; i < SRAM_ways; i++)
               {
                  if (m_lru_bits[i] > max_bits && isValidReplacement(i))
                  {
                     index = i;
                     max_bits = m_lru_bits[i];
                  }
               }               

            }

               /////////////////////////////////////////////////////////////////////////////////////////

        
         }


         LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");


         
         //////////////////////////////////////////////////////////////////

         bool deadblockInSRAMFlag=false;
         bool deadblockInSTTRFlag=false;

         bool deadBlockInBothPartition=false;
         bool deadBlockInNeitherPartition=false;
         bool deadBlockInSRAMOnly=false;
         bool deadBlockInSTTROnly=false;



         //checking deadblock///
         for(UInt32 iter=0; iter<SRAM_ways; iter++)
         {
            if(isDeadBlock(iter, set_index))
               deadblockInSRAMFlag=true;

         }
         for(UInt32 iter=SRAM_ways; iter<m_associativity; iter++)
         {
            if(isDeadBlock(iter, set_index))
               deadblockInSTTRFlag=true;
            
         }
 
         if ((deadblockInSRAMFlag) && (deadblockInSTTRFlag))
            deadBlockInBothPartition=true;
         else if ((!deadblockInSRAMFlag) && (!deadblockInSTTRFlag))
            deadBlockInNeitherPartition=true;
         else if ((deadblockInSRAMFlag) && (!deadblockInSTTRFlag))
            deadBlockInSRAMOnly=true;
         else if ((!deadblockInSRAMFlag) && (deadblockInSTTRFlag))
            deadBlockInSTTROnly=true;
         

         if ((index>=0) && (index<SRAM_ways))
         {
            validSRAMEvictions++;

            if(deadBlockInBothPartition)
            {

               if(isDeadBlock(index, set_index))
                  d0++;
               else
                  e0++;
                           
            }
            else if(deadBlockInNeitherPartition)
            {
               UInt32 indexLocal = 0;
               UInt8 max_bits_local = 0;

               for (UInt32 iter2 = 0; iter2 < m_associativity; iter2++)
               {
                  //if (m_lru_bits_unified[iter2] > max_bits_local) && (isValidReplacement(iter2))
                  if ((m_lru_bits_unified[iter2] > max_bits_local) && (isValidReplacement(iter2)))
                  {
                     indexLocal = iter2;
                     max_bits_local = m_lru_bits_unified[iter2];
                  }
               }

               if((indexLocal>=SRAM_ways) && (indexLocal<m_associativity))
                  g0++;
               else 
                  f0++;

            }
            else if(deadBlockInSRAMOnly)
            {
               if(deadBlockInBothPartition)
               {
                  if(isDeadBlock(index, set_index))
                     b0++;
                  else
                     c0++;
               }

            }
            else if(deadBlockInSTTROnly)
            {
               a0++;
            }

         }



         else if((index>=SRAM_ways) && (index<m_associativity))
         {
            validSTTREvictions++;

            if(deadBlockInBothPartition)
            {
               if(isDeadBlock(index, set_index))
                  d1++;
               else
                  e1++;
            }
            else if(deadBlockInNeitherPartition)
            {

               UInt32 indexLocal = 0;
               UInt8 max_bits_local = 0;

               for (UInt32 iter2 = 0; iter2 < m_associativity; iter2++)
               {
                  //if (m_lru_bits_unified[iter2] > max_bits_local && isValidReplacement(iter2))
                  if ((m_lru_bits_unified[iter2] > max_bits_local) && (isValidReplacement(iter2)))
                  {
                     indexLocal = iter2;
                     max_bits_local = m_lru_bits_unified[iter2];
                  }
               }

               if((indexLocal>=0) && (indexLocal<SRAM_ways))
                  g1++;
               else 
                  f1++;
            }
            else if(deadBlockInSRAMOnly)
            {
               a1++;
            }
            else if(deadBlockInSTTROnly)
            {
               if(deadBlockInBothPartition)
               {
                  if(isDeadBlock(index, set_index))
                     b1++;
                  else
                     c1++;
               }
            }
         }

         //////////////////////////////////////////////////////////////////////////////////

         

   
            
         if((m_cost[index]>K) && (m_state[m_TI[index]]<state_max)) //state should be incremented
            m_state[m_TI[index]]++;
         else if((m_cost[index]<K) && (m_state[m_TI[index]]>0))
            m_state[m_TI[index]]--;         
         
         //check if the victim block is read intense, or write intense or deadblock

         if(m_dcnt[m_TI[index]]>=dcnt_threshold)   //deadblock
            deadblock_counter++;
         else if((m_dcnt[m_TI[index]]<dcnt_threshold)&&(m_cost[index]<K))  //read_intense
            read_intense_block_counter++;
         else if((m_dcnt[m_TI[index]]<dcnt_threshold)&&(m_cost[index]>=K))  //write_intense
            write_intense_block_counter++;
         else
            printf("ERROR!!!!!!\n");


         //Calculating Cmax and Cmin
         if(m_cost[index]<Cmin)
            Cmin = m_cost[index];
         if(m_cost[index]>Cmax)
            Cmax = m_cost[index];



         //Calculating Wmax and Wmin
         if(m_wcnt[m_TI[index]]<Wmin)
            Wmin = m_wcnt[m_TI[index]];
         if(m_wcnt[m_TI[index]]>Wmax)
            Wmax = m_wcnt[m_TI[index]];



         if(m_dcnt[m_TI[index]] != 255)
            m_dcnt[m_TI[index]]++;  //increment deadblock counter on eviction

         if(m_wcnt[m_TI[index]] != 0)
            m_wcnt[m_TI[index]]--;  //decrement write burst counter on eviction

         if((index<SRAM_ways) && (index>=0))
         {
            if(m_dcnt[m_TI[index]]<dcnt_threshold)
               livingSRAMBlocksEvicted++;
            else
               deadSRAMBlocksEvicted++;

         }

         if((m1_flag[index]==1) && (mduringwrite_flag[index]==1))
            doubleMigrationCount++;
         else if ((m1_flag[index]==0) && (mduringwrite_flag[index]==0))
            noMigrationCount++;
         else
            singleMigrationCount++;

         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         
         UInt64 percentageReadBeforeM1             =  0;
         UInt64 percentageWriteBeforeM1            =  0;

         UInt64 percentageReadBeforeMDuringWrite   =  0;
         UInt64 percentageWriteBeforeMDuringWrite  =  0;

         
         if (m_read_before_m1[index]!=0)
            percentageReadBeforeM1             =   ((100 * m_read_before_m1[index])/(m_read_before_m1[index] + m_read_after_m1[index]));
         if (m_write_before_m1[index]!=0)
            percentageWriteBeforeM1            =   ((100 * m_write_before_m1[index])/(m_write_before_m1[index] + m_write_after_m1[index]));

         if (m_read_before_mduringwrite[index]!=0)
            percentageReadBeforeMDuringWrite   =   ((100 * m_read_before_mduringwrite[index])/(m_read_before_mduringwrite[index] + m_read_after_mduringwrite[index])); 
         if (m_write_before_mduringwrite[index]!=0)
            percentageWriteBeforeMDuringWrite  =   ((100 * m_write_before_mduringwrite[index])/(m_write_before_mduringwrite[index] + m_write_after_mduringwrite[index]));
         

         ///////////////////[READ-M1]////////////////////////
         if ((percentageReadBeforeM1>=0)&&(percentageReadBeforeM1<20))
         {
            blocksReadBeforeM1020++;
            blocksReadAfterM180100++;
         }

         else if ((percentageReadBeforeM1>=20)&&(percentageReadBeforeM1<40))
         {
            blocksReadBeforeM12040++;
            blocksReadAfterM16080++;
         }

         else if ((percentageReadBeforeM1>=40)&&(percentageReadBeforeM1<60))
         {
            blocksReadBeforeM14060++;
            blocksReadAfterM14060++;
         }

         else if ((percentageReadBeforeM1>=60)&&(percentageReadBeforeM1<80))
         {
            blocksReadBeforeM16080++;
            blocksReadAfterM12040++;
         }

         else if ((percentageReadBeforeM1>=80)&&(percentageReadBeforeM1<=100))
         {

            blocksReadBeforeM180100++;
            blocksReadAfterM1020++;
         }

         else
         {

         }


         ///////////////////[READ-MDURINGWRITE]////////////////////////
         if ((percentageReadBeforeMDuringWrite>=0)&&(percentageReadBeforeMDuringWrite<20))
         {
            blocksReadBeforeMDuringWrite020++;
            blocksReadAfterMDuringWrite80100++;
         }

         else if ((percentageReadBeforeMDuringWrite>=20)&&(percentageReadBeforeMDuringWrite<40))
         {
            blocksReadBeforeMDuringWrite2040++;
            blocksReadAfterMDuringWrite6080++;
         }

         else if ((percentageReadBeforeMDuringWrite>=40)&&(percentageReadBeforeMDuringWrite<60))
         {
            blocksReadBeforeMDuringWrite4060++;
            blocksReadAfterMDuringWrite4060++;
         }

         else if ((percentageReadBeforeMDuringWrite>=60)&&(percentageReadBeforeMDuringWrite<80))
         {
            blocksReadBeforeMDuringWrite6080++;
            blocksReadAfterMDuringWrite2040++;
         }

         else if ((percentageReadBeforeMDuringWrite>=80)&&(percentageReadBeforeMDuringWrite<=100))
         {
            blocksReadBeforeMDuringWrite80100++;
            blocksReadAfterMDuringWrite020++;
         }

         else
         {
            
         }


         ///////////////////[WRITE-M1]////////////////////////

         if ((percentageWriteBeforeM1>=0)&&(percentageWriteBeforeM1<20))
         {
            blocksWriteBeforeM1020++;
            blocksWriteAfterM180100++;
         }

         else if ((percentageWriteBeforeM1>=20)&&(percentageWriteBeforeM1<40))
         {
            blocksWriteBeforeM12040++;
            blocksWriteAfterM16080++;
         }

         else if ((percentageWriteBeforeM1>=40)&&(percentageWriteBeforeM1<60))
         {
            blocksWriteBeforeM14060++;
            blocksWriteAfterM14060++;
         }

         else if ((percentageWriteBeforeM1>=60)&&(percentageWriteBeforeM1<80))
         {
            blocksWriteBeforeM16080++;
            blocksWriteAfterM12040++;
         }

         else if ((percentageWriteBeforeM1>=80)&&(percentageWriteBeforeM1<=100))
         {
            blocksWriteBeforeM180100++;
            blocksWriteAfterM1020++;
         }

         else
         {

         }


         ///////////////////[WRITE-MDURINGWRITE]////////////////////////
         if ((percentageWriteBeforeMDuringWrite>=0)&&(percentageWriteBeforeMDuringWrite<20))
         {
            blocksWriteBeforeMDuringWrite020++;
            blocksWriteAfterMDuringWrite80100++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=20)&&(percentageWriteBeforeMDuringWrite<40))
         {
            blocksWriteBeforeMDuringWrite2040++;
            blocksWriteAfterMDuringWrite6080++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=40)&&(percentageWriteBeforeMDuringWrite<60))
         {
            blocksWriteBeforeMDuringWrite4060++;
            blocksWriteAfterMDuringWrite4060++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=60)&&(percentageWriteBeforeMDuringWrite<80))
         {
            blocksWriteBeforeMDuringWrite6080++;
            blocksWriteAfterMDuringWrite2040++;
         }

         else if ((percentageWriteBeforeMDuringWrite>=80)&&(percentageWriteBeforeMDuringWrite<=100))
         {
            blocksWriteBeforeMDuringWrite80100++;
            blocksWriteAfterMDuringWrite020++;
         }

         else
         {
            
         }

         if (m1_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-M1]//////////////////////////
            if ((m_read_before_m1[index]+m_read_after_m1[index])>(sf*(m_write_before_m1[index]+m_write_after_m1[index])))
               readIntenseBlocksM1++;

            ///////////////////[WRITE-INTENSE-BLOCKS-M1]//////////////////////////
            else
               writeIntenseBlocksM1++;
         }
         
         if (mduringwrite_flag[index]==1)
         {
            ///////////////////[READ-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            if ((m_read_before_mduringwrite[index]+m_read_after_mduringwrite[index])>(sf*(m_write_before_mduringwrite[index]+m_write_after_mduringwrite[index])))
               readIntenseBlocksMDuringWrite++;

            ///////////////////[WRITE-INTENSE-BLOCKS-MDURINGWRITE]//////////////////////////
            else
               writeIntenseBlocksMDuringWrite++;
         }
         
         

         




         ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






         
         
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

         m_read_before_m1[index]  = 0;
         m_write_before_m1[index] = 0;  
         m_read_after_m1[index]   = 0; 
         m_write_after_m1[index]  = 0; 
         m1_flag[index]           = 0; 

         m_read_before_mduringwrite[index]  = 0;
         m_write_before_mduringwrite[index] = 0;  
         m_read_after_mduringwrite[index]   = 0; 
         m_write_after_mduringwrite[index]  = 0; 
         mduringwrite_flag[index]           = 0;  //If a block has been migrated this is set to 1     

         validBlockEvicted++;

         // Mark our newly-inserted line as most-recently used
         moveToMRU(index);
         m_set_info->incrementAttempt(attempt); 
       

         return index;
         
      } 

   }

   LOG_PRINT_ERROR("Should not reach here");

}


void
CacheSetPHC::updateReplacementIndex(UInt32 accessed_index, UInt8 write_flag, UInt32 set_index)
{
   m_set_info->increment(m_lru_bits[accessed_index]);
   m_set_info->increment(m_lru_bits_unified[accessed_index]);  // for unified LRU stack
   moveToMRU(accessed_index);



   access_counter[accessed_index]++;   //number of accesses to a block

   if(m_dcnt[m_TI[accessed_index]] != 0)
      m_dcnt[m_TI[accessed_index]]--;  //not a deadblock if hit

   m_deadblock[accessed_index] = 1;    //Newton's method

   if(write_flag==1)
   {
      printf("llc write\n");
      m_cost[accessed_index]=m_cost[accessed_index]+Ew;  //cost modification
      write_array[accessed_index]++;   //write_array is the number of writes to a block

      if ((accessed_index>=0) && (accessed_index<m_associativity))
      {
         if(accessed_index<SRAM_ways)
            SRAM_write_hits++;
         else
            STTR_write_hits++;
      }

      if(m1_flag[accessed_index]==0) //The block has not been migrated
      {
         m_write_before_m1[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMWriteBeforeMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMWriteBeforeMigration++;         
      }

      else if(m1_flag[accessed_index]==1)  //The block has been migrated
      {
         m_write_after_m1[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMWriteAfterMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMWriteAfterMigration++;
         
      }
      else
         printf("ERROR!! m1 flag is %d\n", m1_flag[accessed_index]);

      if(mduringwrite_flag[accessed_index]==0) //The block has not been migrated
      {
         m_write_before_mduringwrite[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMWriteBeforeMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMWriteBeforeMigration++;
         
      }
      else if(mduringwrite_flag[accessed_index]==1)  //The block has been migrated
      {
         m_write_after_mduringwrite[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMWriteAfterMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMWriteAfterMigration++;
      }
      else
         printf("ERROR!! mduringwrite flag is %d\n", mduringwrite_flag[accessed_index]);



   }
   else if(write_flag==0)
   {
      m_cost[accessed_index]=m_cost[accessed_index]-Er;
      read_array[accessed_index]++;    //read_array is the number of reads to a block


      if ((accessed_index>=0) && (accessed_index<m_associativity))
      {
         if(accessed_index<SRAM_ways)
            SRAM_reads_hits++;
         else
            STTR_reads_hits++;
      }



      if(m1_flag[accessed_index]==0) //The block has not been migrated
      {
         m_read_before_m1[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMReadBeforeMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMReadBeforeMigration++;
      }
      else if(m1_flag[accessed_index]==1)  //The block has been migrated
      {
         m_read_after_m1[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMReadAfterMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMReadAfterMigration++;


      }
      else
         printf("ERROR!! m1 flag is %d\n", m1_flag[accessed_index]);



      if(mduringwrite_flag[accessed_index]==0) //The block has not been migrated
      {
         m_read_before_mduringwrite[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMReadBeforeMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMReadBeforeMigration++;
      }
      else if(mduringwrite_flag[accessed_index]==1)  //The block has been migrated
      {
         m_read_after_mduringwrite[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMReadAfterMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMReadAfterMigration++;
      }
      else
         printf("ERROR!! mduringwrite flag flag is %d\n", mduringwrite_flag[accessed_index]);



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
CacheSetPHC::updateReplacementIndex2(UInt32 accessed_index, UInt32 set_index, IntPtr eip)
{
   UInt16 eip_truncated = truncatedEipCalculation(eip);

   if(asl2_flag==0)
   {
      printf("\naccessSingleLine2 is called\n");
      asl2_flag = 1; 
   }
   
   if ((accessed_index>=0) && (accessed_index<m_associativity))
   {
      if(accessed_index<SRAM_ways)
         SRAM_write_hits++;
      else
         STTR_write_hits++;



      if(m1_flag[accessed_index]==0) //The block has not been migrated
      {
         m_write_before_m1[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMWriteBeforeMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMWriteBeforeMigration++; 

      }
      else if(m1_flag[accessed_index]==1)  //The block has been migrated
      {
         m_write_after_m1[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMWriteAfterMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMWriteAfterMigration++; 
      }
      else
         printf("ERROR!! m1 flag is %d\n", m1_flag[accessed_index]);



      if(mduringwrite_flag[accessed_index]==0) //The block has not been migrated
      {
         m_write_before_mduringwrite[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMWriteBeforeMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMWriteBeforeMigration++; 

      }
      else if(mduringwrite_flag[accessed_index]==1)  //The block has been migrated
      {
         m_write_after_mduringwrite[accessed_index]++;

         if((accessed_index>=0) && (accessed_index<SRAM_ways)) //SRAM Write hit before migration
            SRAMWriteAfterMigration++;
         else if ((accessed_index>=SRAM_ways) && (SRAM_ways<m_associativity)) //STTRAM Write hit before migration
            STTRAMWriteAfterMigration++; 

      }
      else
         printf("ERROR!! mduringwrite flag flag is %d\n", mduringwrite_flag[accessed_index]);





      m_deadblock[accessed_index] = 1;    //Newton's method

      if(m_dcnt[m_TI[accessed_index]] != 0)
         m_dcnt[m_TI[accessed_index]]--;  //not a deadblock if hit

      if(m_wcnt[m_TI[accessed_index]] != 255)
         m_wcnt[m_TI[accessed_index]]++;  //decrement write burst counter on eviction

      access_counter[accessed_index]++;

      m_cost[accessed_index]=m_cost[accessed_index]+Ew;  //cost modification
      write_array[accessed_index]++;

      if(access_counter[accessed_index]==N_transition)  //after N accesses check if a previosuly predicted read intensive block has changed to write intensive block or not
      {
         if((accessed_index>=0) && (accessed_index<SRAM_ways) && (read_array[accessed_index]>(sf*write_array[accessed_index])))  //SRAM partition. predicted write intensive by PHC
            writeToReadTransitionsAtInterval++;
   
         if((accessed_index>=SRAM_ways) && (accessed_index<m_associativity) && (read_array[accessed_index]<(sf*write_array[accessed_index])))   //STTRAM. Predicted Read intensive
            readToWriteTransitionsAtInterval++;
            
         access_counter[accessed_index] = 0;
      }
      
      
      if((accessed_index>=SRAM_ways) && (accessed_index<m_associativity))
      {
         //TO-DO: modify updateReplacementIndex2 to include eip_truncated
         migrate_during_write(accessed_index, eip_truncated);  //migrates write index blocks from stt to sram during write; 

         /*
         //[ARINDAM]
         //call migrate2() function
         //if the write hit is in STTRAM and the block is write burst, we need to place the block in SRAM
         //migrate2() function will do that
         if(m_wcnt[m_TI[accessed_index]]>W)
         {
            migrate2(accessed_index);
         }
         */
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
   if((accessed_index<SRAM_ways) && (accessed_index>=0))
   {
      for (UInt32 i = 0; i < SRAM_ways; i++)
      {
         if (m_lru_bits[i] < m_lru_bits[accessed_index])
            m_lru_bits[i] ++;
      }
      m_lru_bits[accessed_index] = 0;
   }

   else if((accessed_index<m_associativity) && (accessed_index>=SRAM_ways))
   {
      for (UInt32 i = SRAM_ways; i < m_associativity; i++)
      {
         if (m_lru_bits[i] < m_lru_bits[accessed_index])
            m_lru_bits[i] ++;
      }
      m_lru_bits[accessed_index] = 0;
   }
   else
   {

   }

   //moveToMRU for unified LRU
   if((accessed_index>=0) && (accessed_index<m_associativity))
   {
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (m_lru_bits_unified[i] < m_lru_bits_unified[accessed_index])
            m_lru_bits_unified[i] ++;
      }
      m_lru_bits_unified[accessed_index] = 0;
   }


   
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

bool
CacheSetPHC::isDeadBlock(UInt32 index, UInt32 set_index_local)
{
   //printf("dcnt_threshold_min is %d, dcnt_threshold is %d and dcnt_threshold_plu is %d\n", dcnt_threshold_min, dcnt_threshold, dcnt_threshold_plu);
   if((set_index_local % sampler_fraction)==3)
   {
      if (m_dcnt[m_TI[index]]>=dcnt_threshold_min)
         return true;
      else 
         return false;
   }
   else if ((set_index_local % sampler_fraction)==4)
   {
      if (m_dcnt[m_TI[index]]>=dcnt_threshold_plu)
         return true;
      else 
         return false;
   }
   else
   {
      if (m_dcnt[m_TI[index]]>=dcnt_threshold)
         return true;
      else 
         return false;
   }
   
}


void
CacheSetPHC::migrate(UInt32 sram_index)   //migration during eviction
{
   
   /*
   UInt32 stt_index = -1;
   UInt8 local_max_bits = 0;
   CacheBlockInfo* temp_cache_block_info = CacheBlockInfo::create(CacheBase::SHARED_CACHE);

   UInt8 temp_lru_bits = 0;
   UInt8 temp_lru_bits_unified = 0;
   UInt16 temp_TI = 0;
   UInt8 temp_cost = 0;
   UInt16 temp_write_array = 0;
   UInt16 temp_read_array = 0;
   UInt16 temp_access_counter = 0;
   UInt8 temp_deadblock = 0;

   UInt64 temp_m_read_before_m1  = 0;
   UInt64 temp_m_write_before_m1 = 0;  
   UInt64 temp_m_read_after_m1   = 0; 
   UInt64 temp_m_write_after_m1  = 0; 
   UInt64 temp_m1_flag           = 0;  

   UInt64 temp_m_read_before_mduringwrite    = 0;
   UInt64 temp_m_write_before_mduringwrite   = 0;  
   UInt64 temp_m_read_after_mduringwrite     = 0; 
   UInt64 temp_m_write_after_mduringwrite    = 0; 
   UInt64 temp_mduringwrite_flag             = 0;  
   

   if(m_dcnt[m_TI[sram_index]]<dcnt_threshold)          //PC based deadblock prediction
   //if(m_deadblock[sram_index]!=0)                     //Newton's deadblock prediction (daaip)
   {
      //find stt_index
      for (UInt32 i = SRAM_ways; i < m_associativity; i++)
      {
         if ((m_lru_bits[i] > local_max_bits) && isValidReplacement(i))
         {
            stt_index = i;
            local_max_bits = m_lru_bits[i];
         }
      }
      
      if((stt_index>=SRAM_ways) && (stt_index<m_associativity))
      //swapping the blocks with index as stt_index and sram_index   
      {
         m1Count++;

         m1_flag[sram_index] = 1;   //This block has been migrated


         temp_cache_block_info->clone(m_cache_block_info_array[stt_index]);
         m_cache_block_info_array[stt_index]->clone(m_cache_block_info_array[sram_index]);
         m_cache_block_info_array[sram_index]->clone(temp_cache_block_info);

         /////////////////////////////////////////////////////////////////////
         temp_lru_bits           = m_lru_bits[sram_index];
         temp_lru_bits_unified   = m_lru_bits_unified[sram_index];
         temp_TI                 = m_TI[sram_index]; 
         temp_cost               = m_cost[sram_index]; 
         temp_write_array        = write_array[sram_index];
         temp_read_array         = read_array[sram_index];
         temp_access_counter     = access_counter[sram_index];
         temp_deadblock          = m_deadblock[sram_index];

         temp_m_read_before_m1            =  m_read_before_m1[sram_index];
         temp_m_write_before_m1           =  m_write_before_m1[sram_index]; 
         temp_m_read_after_m1             =  m_read_after_m1[sram_index];
         temp_m_write_after_m1            =  m_write_after_m1[sram_index];
         temp_m1_flag                     =  m1_flag[sram_index];
         temp_m_read_before_mduringwrite  =  m_read_before_mduringwrite[sram_index];
         temp_m_write_before_mduringwrite =  m_write_before_mduringwrite[sram_index];
         temp_m_read_after_mduringwrite   =  m_read_after_mduringwrite[sram_index];
         temp_m_write_after_mduringwrite  =  m_write_after_mduringwrite[sram_index];
         temp_mduringwrite_flag           =  mduringwrite_flag[sram_index];

         /////////////////////////////////////////////////////////////////////
         m_lru_bits[sram_index]                    = m_lru_bits[stt_index];
         m_lru_bits_unified[sram_index]            = m_lru_bits_unified[stt_index];
         m_TI[sram_index]                          = m_TI[stt_index]; 
         m_cost[sram_index]                        = m_cost[stt_index]; 
         write_array[sram_index]                   = write_array[stt_index];
         read_array[sram_index]                    = read_array[stt_index];
         access_counter[sram_index]                = access_counter[stt_index];
         m_deadblock[sram_index]                   = m_deadblock[stt_index];

         m_read_before_m1[sram_index]              =  m_read_before_m1[stt_index];
         m_write_before_m1[sram_index]             =  m_write_before_m1[stt_index];
         m_read_after_m1[sram_index]               =  m_read_after_m1[stt_index];
         m_write_after_m1[sram_index]              =  m_write_after_m1[stt_index];
         m1_flag[sram_index]                       =  m1_flag[stt_index];
         m_read_before_mduringwrite[sram_index]    =  m_read_before_mduringwrite[stt_index];
         m_write_before_mduringwrite[sram_index]   =  m_write_before_mduringwrite[stt_index];
         m_read_after_mduringwrite[sram_index]     =  m_read_after_mduringwrite[stt_index];
         m_write_after_mduringwrite[sram_index]    =  m_write_after_mduringwrite[stt_index];
         mduringwrite_flag[sram_index]             =  mduringwrite_flag[stt_index];

         /////////////////////////////////////////////////////////////////////
         m_lru_bits[stt_index]                  = temp_lru_bits;
         m_lru_bits_unified[stt_index]          = temp_lru_bits_unified;
         m_TI[stt_index]                        = temp_TI; 
         m_cost[stt_index]                      = temp_cost; 
         write_array[stt_index]                 = temp_write_array;
         read_array[stt_index]                  = temp_read_array;
         access_counter[stt_index]              = temp_access_counter;
         m_deadblock[stt_index]                 = temp_deadblock;

         m_read_before_m1[stt_index]            = temp_m_read_before_m1;                       
         m_write_before_m1[stt_index]           = temp_m_write_before_m1;          
         m_read_after_m1[stt_index]             = temp_m_read_after_m1;            
         m_write_after_m1[stt_index]            = temp_m_write_after_m1;           
         m1_flag[stt_index]                     = temp_m1_flag;                    
         m_read_before_mduringwrite[stt_index]  = temp_m_read_before_mduringwrite; 
         m_write_before_mduringwrite[stt_index] = temp_m_write_before_mduringwrite;
         m_read_after_mduringwrite[stt_index]   = temp_m_read_after_mduringwrite;  
         m_write_after_mduringwrite[stt_index]  = temp_m_write_after_mduringwrite; 
         mduringwrite_flag[stt_index]           = temp_mduringwrite_flag;  
         //migrate_flag = 1; //This will be cross checked in cache controller inside insertCacheBlock function, to account for penalty
      }

   }
   else
   {

   }
   */
   
   
}

void
CacheSetPHC::migrate2(UInt32 stt_index) //this function swaps a write burst file which is write hit in STTRAM with another block from SRAM
{
   //sttram_index is the index of the write burst block which is write hit in STTRAM
   //How do we select a block from SRAM with which to swap??
   //We can select a block with lowest m_cost to figure out the most read intense block from SRAM partition
   UInt32 least_cost=255;
   UInt32 sram_index=-1;

   CacheBlockInfo* temp_cache_block_info = CacheBlockInfo::create(CacheBase::SHARED_CACHE);

   UInt8 temp_lru_bits = 0;
   UInt16 temp_TI = 0;
   UInt8 temp_cost = 0;
   UInt16 temp_write_array = 0;
   UInt16 temp_read_array = 0;
   UInt16 temp_access_counter = 0;
   UInt8 temp_deadblock = 0;


   for (UInt32 i=0; i<SRAM_ways; i++)
   {
      if ((m_cost[i]<least_cost) && isValidReplacement(i))
      {
         least_cost=m_cost[i];
         sram_index = i;
      }
   }

   if ((sram_index>=0) && (sram_index<SRAM_ways))
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
   }


}

void
CacheSetPHC::migrate_during_write(UInt32 stt_index, UInt16 eip_truncated)    //called during  writeback. written by ARINDAM

//Motive: If a block to be written is found out to be write intensive, then migrate the block to SRAM
//A block is deadblock if m_dcnt of eip_truncated is less than threshold
//A block is read intensive if state of eip_truncated is less than threshold and it is not a deadblock
//A block is write intensive if it is neither deablock nor  
//QUESTION: How to select the victim block from SRAM??
//If the function is called by updateReplacementIndex2, victim should be the most read intense block

{
   
   /*
   if((m_dcnt[eip_truncated]<dcnt_threshold) && (m_state[eip_truncated]>=state_threshold))   //not dead and write intense
   {
      UInt32 least_cost=255;
      UInt32 sram_index=-1;
      

      CacheBlockInfo* temp_cache_block_info = CacheBlockInfo::create(CacheBase::SHARED_CACHE);

      UInt8 temp_lru_bits           = 0;
      UInt8 temp_lru_bits_unified   = 0;
      UInt16 temp_TI                = 0;
      UInt8 temp_cost               = 0;
      UInt16 temp_write_array       = 0;
      UInt16 temp_read_array        = 0;
      UInt16 temp_access_counter    = 0;
      UInt8 temp_deadblock          = 0;

      UInt64 temp_m_read_before_m1  = 0;
      UInt64 temp_m_write_before_m1 = 0;  
      UInt64 temp_m_read_after_m1   = 0; 
      UInt64 temp_m_write_after_m1  = 0; 
      UInt64 temp_m1_flag           = 0;  

      UInt64 temp_m_read_before_mduringwrite  = 0;
      UInt64 temp_m_write_before_mduringwrite = 0;  
      UInt64 temp_m_read_after_mduringwrite   = 0; 
      UInt64 temp_m_write_after_mduringwrite  = 0; 
      UInt64 temp_mduringwrite_flag           = 0; 

      
      //for (UInt32 i=0; i<SRAM_ways; i++)
      //{
      //   if ((m_cost[i]<least_cost) && isValidReplacement(i))  //most read intense block
      //   {
      //      least_cost=m_cost[i];
      //      sram_index = i;
      //   }
      //}
      

      UInt8 tmp_state[SRAM_ways]={0};
      UInt32 tmp_index_array[4]={0, 1, 2, 3};

      for (UInt32 c=0; c<SRAM_ways; c++)
         tmp_state[c]=m_state[m_TI[c]];

      
      for (UInt32 c=0; c<(SRAM_ways-1); c++)
      {
         for (UInt32 d=0; d<(SRAM_ways-c-1); d++)
         {

            if (tmp_state[d] > tmp_state[d+1]) // For decreasing order use < 
            {
               UInt8 swap_state = tmp_state[d];
               tmp_state[d] = tmp_state[d+1];
               tmp_state[d+1] = swap_state;
              
               UInt32 swap_index = tmp_index_array[d];
               tmp_index_array[d] = tmp_index_array[d+1];
               tmp_index_array[d+1] = swap_index;
            }
         }
      }

      for (UInt32 c=0; c<SRAM_ways; c++)
      {
         //if((isValidReplacement(tmp_index_array[c])) && (m_state[m_TI[tmp_index_array[c]]]<m_state[eip_truncated]))
         if((isValidReplacement(tmp_index_array[c])) && (m_state[m_TI[tmp_index_array[c]]]<m_state[m_TI[stt_index]]))
         {
            sram_index = tmp_index_array[c];
            break;
         }
      }
      
      

      if ((sram_index>=0) && (sram_index<SRAM_ways))
      {
         mDuringWriteCount++;

         mduringwrite_flag[stt_index]=1; //This flag is set if migration is done

         temp_cache_block_info->clone(m_cache_block_info_array[stt_index]);
         m_cache_block_info_array[stt_index]->clone(m_cache_block_info_array[sram_index]);
         m_cache_block_info_array[sram_index]->clone(temp_cache_block_info);

         /////////////////////////////////////////////////////////////////////
         temp_lru_bits           = m_lru_bits[sram_index];
         temp_lru_bits_unified   = m_lru_bits_unified[sram_index];
         temp_TI                 = m_TI[sram_index]; 
         temp_cost               = m_cost[sram_index]; 
         temp_write_array        = write_array[sram_index];
         temp_read_array         = read_array[sram_index];
         temp_access_counter     = access_counter[sram_index];
         temp_deadblock          = m_deadblock[sram_index];

         temp_m_read_before_m1            =  m_read_before_m1[sram_index];
         temp_m_write_before_m1           =  m_write_before_m1[sram_index]; 
         temp_m_read_after_m1             =  m_read_after_m1[sram_index];
         temp_m_write_after_m1            =  m_write_after_m1[sram_index];
         temp_m1_flag                     =  m1_flag[sram_index];
         temp_m_read_before_mduringwrite  =  m_read_before_mduringwrite[sram_index];
         temp_m_write_before_mduringwrite =  m_write_before_mduringwrite[sram_index];
         temp_m_read_after_mduringwrite   =  m_read_after_mduringwrite[sram_index];
         temp_m_write_after_mduringwrite  =  m_write_after_mduringwrite[sram_index];
         temp_mduringwrite_flag           =  mduringwrite_flag[sram_index];

         /////////////////////////////////////////////////////////////////////
         m_lru_bits[sram_index]                    = m_lru_bits[stt_index];
         m_lru_bits_unified[sram_index]            = m_lru_bits_unified[stt_index];
         m_TI[sram_index]                          = m_TI[stt_index]; 
         m_cost[sram_index]                        = m_cost[stt_index]; 
         write_array[sram_index]                   = write_array[stt_index];
         read_array[sram_index]                    = read_array[stt_index];
         access_counter[sram_index]                = access_counter[stt_index];
         m_deadblock[sram_index]                   = m_deadblock[stt_index];

         m_read_before_m1[sram_index]              =  m_read_before_m1[stt_index];
         m_write_before_m1[sram_index]             =  m_write_before_m1[stt_index];
         m_read_after_m1[sram_index]               =  m_read_after_m1[stt_index];
         m_write_after_m1[sram_index]              =  m_write_after_m1[stt_index];
         m1_flag[sram_index]                       =  m1_flag[stt_index];
         m_read_before_mduringwrite[sram_index]    =  m_read_before_mduringwrite[stt_index];
         m_write_before_mduringwrite[sram_index]   =  m_write_before_mduringwrite[stt_index];
         m_read_after_mduringwrite[sram_index]     =  m_read_after_mduringwrite[stt_index];
         m_write_after_mduringwrite[sram_index]    =  m_write_after_mduringwrite[stt_index];
         mduringwrite_flag[sram_index]             =  mduringwrite_flag[stt_index];

         /////////////////////////////////////////////////////////////////////
         m_lru_bits[stt_index]                  = temp_lru_bits;
         m_lru_bits_unified[stt_index]          = temp_lru_bits_unified;
         m_TI[stt_index]                        = temp_TI; 
         m_cost[stt_index]                      = temp_cost; 
         write_array[stt_index]                 = temp_write_array;
         read_array[stt_index]                  = temp_read_array;
         access_counter[stt_index]              = temp_access_counter;
         m_deadblock[stt_index]                 = temp_deadblock;

         m_read_before_m1[stt_index]            = temp_m_read_before_m1;                       
         m_write_before_m1[stt_index]           = temp_m_write_before_m1;          
         m_read_after_m1[stt_index]             = temp_m_read_after_m1;            
         m_write_after_m1[stt_index]            = temp_m_write_after_m1;           
         m1_flag[stt_index]                     = temp_m1_flag;                    
         m_read_before_mduringwrite[stt_index]  = temp_m_read_before_mduringwrite; 
         m_write_before_mduringwrite[stt_index] = temp_m_write_before_mduringwrite;
         m_read_after_mduringwrite[stt_index]   = temp_m_read_after_mduringwrite;  
         m_write_after_mduringwrite[stt_index]  = temp_m_write_after_mduringwrite; 
         mduringwrite_flag[stt_index]           = temp_mduringwrite_flag;          
      }
   }
   
   */
   
   
}

void
CacheSetPHC::swapTwoBlocks(UInt32 sram_index, UInt32 stt_index)
{
   CacheBlockInfo* temp_cache_block_info = CacheBlockInfo::create(CacheBase::SHARED_CACHE);

   UInt8 temp_lru_bits = 0;
   UInt8 temp_lru_bits_unified = 0;
   UInt16 temp_TI = 0;
   UInt8 temp_cost = 0;
   UInt16 temp_write_array = 0;
   UInt16 temp_read_array = 0;
   UInt16 temp_access_counter = 0;
   UInt8 temp_deadblock = 0;

   UInt64 temp_m_read_before_m1  = 0;
   UInt64 temp_m_write_before_m1 = 0;  
   UInt64 temp_m_read_after_m1   = 0; 
   UInt64 temp_m_write_after_m1  = 0; 
   UInt64 temp_m1_flag           = 0;  

   UInt64 temp_m_read_before_mduringwrite    = 0;
   UInt64 temp_m_write_before_mduringwrite   = 0;  
   UInt64 temp_m_read_after_mduringwrite     = 0; 
   UInt64 temp_m_write_after_mduringwrite    = 0; 
   UInt64 temp_mduringwrite_flag             = 0;  

   temp_cache_block_info->clone(m_cache_block_info_array[stt_index]);
   m_cache_block_info_array[stt_index]->clone(m_cache_block_info_array[sram_index]);
   m_cache_block_info_array[sram_index]->clone(temp_cache_block_info);
   /////////////////////////////////////////////////////////////////////
   temp_lru_bits           = m_lru_bits[sram_index];
   temp_lru_bits_unified   = m_lru_bits_unified[sram_index];
   temp_TI                 = m_TI[sram_index]; 
   temp_cost               = m_cost[sram_index]; 
   temp_write_array        = write_array[sram_index];
   temp_read_array         = read_array[sram_index];
   temp_access_counter     = access_counter[sram_index];
   temp_deadblock          = m_deadblock[sram_index];
   temp_m_read_before_m1            =  m_read_before_m1[sram_index];
   temp_m_write_before_m1           =  m_write_before_m1[sram_index]; 
   temp_m_read_after_m1             =  m_read_after_m1[sram_index];
   temp_m_write_after_m1            =  m_write_after_m1[sram_index];
   temp_m1_flag                     =  m1_flag[sram_index];
   temp_m_read_before_mduringwrite  =  m_read_before_mduringwrite[sram_index];
   temp_m_write_before_mduringwrite =  m_write_before_mduringwrite[sram_index];
   temp_m_read_after_mduringwrite   =  m_read_after_mduringwrite[sram_index];
   temp_m_write_after_mduringwrite  =  m_write_after_mduringwrite[sram_index];
   temp_mduringwrite_flag           =  mduringwrite_flag[sram_index];
   /////////////////////////////////////////////////////////////////////
   m_lru_bits[sram_index]                    = m_lru_bits[stt_index];
   m_lru_bits_unified[sram_index]            = m_lru_bits_unified[stt_index];
   m_TI[sram_index]                          = m_TI[stt_index]; 
   m_cost[sram_index]                        = m_cost[stt_index]; 
   write_array[sram_index]                   = write_array[stt_index];
   read_array[sram_index]                    = read_array[stt_index];
   access_counter[sram_index]                = access_counter[stt_index];
   m_deadblock[sram_index]                   = m_deadblock[stt_index];
   m_read_before_m1[sram_index]              =  m_read_before_m1[stt_index];
   m_write_before_m1[sram_index]             =  m_write_before_m1[stt_index];
   m_read_after_m1[sram_index]               =  m_read_after_m1[stt_index];
   m_write_after_m1[sram_index]              =  m_write_after_m1[stt_index];
   m1_flag[sram_index]                       =  m1_flag[stt_index];
   m_read_before_mduringwrite[sram_index]    =  m_read_before_mduringwrite[stt_index];
   m_write_before_mduringwrite[sram_index]   =  m_write_before_mduringwrite[stt_index];
   m_read_after_mduringwrite[sram_index]     =  m_read_after_mduringwrite[stt_index];
   m_write_after_mduringwrite[sram_index]    =  m_write_after_mduringwrite[stt_index];
   mduringwrite_flag[sram_index]             =  mduringwrite_flag[stt_index];
   /////////////////////////////////////////////////////////////////////
   m_lru_bits[stt_index]                  = temp_lru_bits;
   m_lru_bits_unified[stt_index]          = temp_lru_bits_unified;
   m_TI[stt_index]                        = temp_TI; 
   m_cost[stt_index]                      = temp_cost; 
   write_array[stt_index]                 = temp_write_array;
   read_array[stt_index]                  = temp_read_array;
   access_counter[stt_index]              = temp_access_counter;
   m_deadblock[stt_index]                 = temp_deadblock;
   m_read_before_m1[stt_index]            = temp_m_read_before_m1;                       
   m_write_before_m1[stt_index]           = temp_m_write_before_m1;          
   m_read_after_m1[stt_index]             = temp_m_read_after_m1;            
   m_write_after_m1[stt_index]            = temp_m_write_after_m1;           
   m1_flag[stt_index]                     = temp_m1_flag;                    
   m_read_before_mduringwrite[stt_index]  = temp_m_read_before_mduringwrite; 
   m_write_before_mduringwrite[stt_index] = temp_m_write_before_mduringwrite;
   m_read_after_mduringwrite[stt_index]   = temp_m_read_after_mduringwrite;  
   m_write_after_mduringwrite[stt_index]  = temp_m_write_after_mduringwrite; 
   mduringwrite_flag[stt_index]           = temp_mduringwrite_flag;  
}


