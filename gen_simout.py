#!/usr/bin/env python

import sys, os, getopt, sniper_lib


def generate_simout(jobid = None, resultsdir = None, partial = None, output = sys.stdout, silent = False):

  try:
    res = sniper_lib.get_results(jobid = jobid, resultsdir = resultsdir, partial = partial)
  except (KeyError, ValueError), e:
    if not silent:
      print 'Failed to generated sim.out:', e
    return

  results = res['results']
  config = res['config']
  ncores = int(config['general/total_cores'])

  format_int = lambda v: str(long(v))
  format_pct = lambda v: '%.1f%%' % (100. * v)
  def format_float(digits):
    return lambda v: ('%%.%uf' % digits) % v
  def format_ns(digits):
    return lambda v: ('%%.%uf' % digits) % (v/1e6)

  if 'barrier.global_time_begin' in results:
    time0_begin = results['barrier.global_time_begin']
    time0_end = results['barrier.global_time_end']

  if 'barrier.global_time' in results:
    time0 = results['barrier.global_time'][0]
  else:
    time0 = time0_begin - time0_end



  if sum(results['performance_model.instruction_count']) == 0:
    # core.instructions is less exact, but in cache-only mode it's all there is
    results['performance_model.instruction_count'] = results['core.instructions']

  results['performance_model.elapsed_time_fixed'] = [
    time0
    for c in range(ncores)
  ]
  results['performance_model.cycle_count_fixed'] = [
    results['performance_model.elapsed_time_fixed'][c] * results['fs_to_cycles_cores'][c]
    for c in range(ncores)
  ]
  results['performance_model.ipc'] = [
    i / (c or 1)
    for i, c in zip(results['performance_model.instruction_count'], results['performance_model.cycle_count_fixed'])
  ]
  results['performance_model.nonidle_elapsed_time'] = [
    results['performance_model.elapsed_time'][c] - results['performance_model.idle_elapsed_time'][c]
    for c in range(ncores)
  ]
  results['performance_model.idle_elapsed_time'] = [
    time0 - results['performance_model.nonidle_elapsed_time'][c]
    for c in range(ncores)
  ]
  results['performance_model.idle_elapsed_percent'] = [
    results['performance_model.idle_elapsed_time'][c] / float(time0)
    for c in range(ncores)
  ]

  #udal: added for miss count
  if 'interval_timer.Winner_SRRIP' in results:
    results['interval_timer.win_srrip'] = [float(results['interval_timer.Winner_SRRIP'][core] or 1) for core in range(ncores)]
    
  if 'interval_timer.Winner_DAAIP' in results:
    results['interval_timer.win_daaip'] = [float(results['interval_timer.Winner_DAAIP'][core] or 1) for core in range(ncores)]

  if 'interval_timer.SRRIP_Misses' in results:
    results['interval_timer.srrip_misses'] = [float(results['interval_timer.SRRIP_Misses'][core] or 1) for core in range(ncores)]

  if 'interval_timer.DAAIP_Misses' in results:
    results['interval_timer.daaip_misses'] = [float(results['interval_timer.DAAIP_Misses'][core] or 1) for core in range(ncores)]
    
  if 'interval_timer.SRRIP_Hits' in results:
    results['interval_timer.SRRIP_Hits'] = [float(results['interval_timer.SRRIP_Hits'][core] or 1) for core in range(ncores)]

  if 'interval_timer.ATD_Misses' in results:
    results['interval_timer.ATD_Misses'] = [float(results['interval_timer.ATD_Misses'][core] or 1) for core in range(ncores)]

  if 'interval_timer.ATD_Hits' in results:
    results['interval_timer.ATD_Hits'] = [float(results['interval_timer.ATD_Hits'][core] or 1) for core in range(ncores)] 

  #arindam
#  if 'interval_timer.Read_To_Write_Transitions_At_Interval' in results:
#    results['interval_timer.Read_To_Write_Transitions_At_Interval'] = [float(results['interval_timer.Read_To_Write_Transitions_At_Interval'][core] or 1) for core in range(ncores)]

#  if 'interval_timer.Read_To_Write_Transitions_At_Eviction' in results:
#    results['interval_timer.Read_To_Write_Transitions_At_Eviction'] = [float(results['interval_timer.Read_To_Write_Transitions_At_Eviction'][core] or 1) for core in range(ncores)]

#  if 'interval_timer.Write_To_Read_Transitions_At_Interval' in results:
#    results['interval_timer.Write_To_Read_Transitions_At_Interval'] = [float(results['interval_timer.Write_To_Read_Transitions_At_Interval'][core] or 1) for core in range(ncores)]

  if 'interval_timer.Write_To_Read_Transitions_At_Eviction' in results:
    results['interval_timer.Write_To_Read_Transitions_At_Eviction'] = [float(results['interval_timer.Write_To_Read_Transitions_At_Eviction'][core] or 1) for core in range(ncores)]

  if 'interval_timer.Read_To_Write_Transitions_At_Eviction' in results:
    results['interval_timer.Read_To_Write_Transitions_At_Eviction'] = [float(results['interval_timer.Read_To_Write_Transitions_At_Eviction'][core] or 1) for core in range(ncores)]

  if 'interval_timer.Read_Intense_Block_Counter' in results:
    results['interval_timer.Read_Intense_Block_Counter'] = [float(results['interval_timer.Read_Intense_Block_Counter'][core] or 1) for core in range(ncores)]

  if 'interval_timer.Write_Intense_Block_Counter' in results:
    results['interval_timer.Write_Intense_Block_Counter'] = [float(results['interval_timer.Write_Intense_Block_Counter'][core] or 1) for core in range(ncores)]

  if 'interval_timer.Deadblock_Counter' in results:
    results['interval_timer.Deadblock_Counter'] = [float(results['interval_timer.Deadblock_Counter'][core] or 1) for core in range(ncores)]

  if 'interval_timer.Valid_Block_Evicted' in results:
    results['interval_timer.Valid_Block_Evicted'] = [float(results['interval_timer.Valid_Block_Evicted'][core] or 1) for core in range(ncores)]

  if 'interval_timer.Write_Intense_Blocks_During_Migrate' in results:
    results['interval_timer.Write_Intense_Blocks_During_Migrate'] = [float(results['interval_timer.Write_Intense_Blocks_During_Migrate'][core] or 1) for core in range(ncores)]

  if 'interval_timer.Read_Intense_Blocks_During_Migrate' in results:
    results['interval_timer.Read_Intense_Blocks_During_Migrate'] = [float(results['interval_timer.Read_Intense_Blocks_During_Migrate'][core] or 1) for core in range(ncores)] 




  if 'interval_timer.Read_Intense_Blocks_M' in results:
    results['interval_timer.Read_Intense_Blocks_M'] = [float(results['interval_timer.Read_Intense_Blocks_M'][core] or 1) for core in range(ncores)] 

  if 'interval_timer.Write_Intense_Blocks_M' in results:
    results['interval_timer.Write_Intense_Blocks_M'] = [float(results['interval_timer.Write_Intense_Blocks_M'][core] or 1) for core in range(ncores)] 

  if 'interval_timer.Read_Intense_Blocks_MDuringMigrate' in results:
    results['interval_timer.Read_Intense_Blocks_MDuringMigrate'] = [float(results['interval_timer.Read_Intense_Blocks_MDuringMigrate'][core] or 1) for core in range(ncores)] 

  if 'interval_timer.Write_Intense_Blocks_MDuringMigrate' in results:
    results['interval_timer.Write_Intense_Blocks_MDuringMigrate'] = [float(results['interval_timer.Write_Intense_Blocks_MDuringMigrate'][core] or 1) for core in range(ncores)] 





  if 'interval_timer.M_Count' in results:
    results['interval_timer.M_Count'] = [float(results['interval_timer.M_Count'][core] or 1) for core in range(ncores)] 

  if 'interval_timer.Migrate_During_Write_Count' in results:
    results['interval_timer.Migrate_During_Write_Count'] = [float(results['interval_timer.Migrate_During_Write_Count'][core] or 1) for core in range(ncores)] 



  if 'interval_timer.SRAM_Read_Before_Migration'    in results: 
    results['interval_timer.SRAM_Read_Before_Migration']  = [float(results['interval_timer.SRAM_Read_Before_Migration'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.SRAM_Read_After_Migration'     in results: 
    results['interval_timer.SRAM_Read_After_Migration']   = [float(results['interval_timer.SRAM_Read_After_Migration'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.SRAM_Write_Before_Migration'   in results: 
    results['interval_timer.SRAM_Write_Before_Migration']   = [float(results['interval_timer.SRAM_Write_Before_Migration'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.SRAM_Write_After_Migration'    in results: 
    results['interval_timer.SRAM_Write_After_Migration']  = [float(results['interval_timer.SRAM_Write_After_Migration'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.STTRAM_Read_Before_Migration'  in results: 
    results['interval_timer.STTRAM_Read_Before_Migration']  = [float(results['interval_timer.STTRAM_Read_Before_Migration'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.STTRAM_Read_After_Migration'   in results: 
    results['interval_timer.STTRAM_Read_After_Migration']   = [float(results['interval_timer.STTRAM_Read_After_Migration'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.STTRAM_Write_Before_Migration'   in results: 
    results['interval_timer.STTRAM_Write_Before_Migration'] = [float(results['interval_timer.STTRAM_Write_Before_Migration'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.STTRAM_Write_After_Migration'  in results: 
    results['interval_timer.STTRAM_Write_After_Migration']  = [float(results['interval_timer.STTRAM_Write_After_Migration'][core] or 1) for core in range(ncores)]  



  if 'interval_timer.living_SRAM_Blocks_Evicted'  in results: 
    results['interval_timer.living_SRAM_Blocks_Evicted']  = [float(results['interval_timer.living_SRAM_Blocks_Evicted'][core] or 1) for core in range(ncores)]  

  if 'interval_timer.dead_SRAM_Blocks_Evicted'  in results: 
    results['interval_timer.dead_SRAM_Blocks_Evicted']  = [float(results['interval_timer.dead_SRAM_Blocks_Evicted'][core] or 1) for core in range(ncores)]



  if 'interval_timer.single_Migration_Count'  in results: 
    results['interval_timer.single_Migration_Count']  = [float(results['interval_timer.single_Migration_Count'][core] or 1) for core in range(ncores)]  

  if 'interval_timer.double_Migration_Count'  in results: 
    results['interval_timer.double_Migration_Count']  = [float(results['interval_timer.double_Migration_Count'][core] or 1) for core in range(ncores)]  

  if 'interval_timer.no_Migration_Count'  in results: 
    results['interval_timer.no_Migration_Count']  = [float(results['interval_timer.no_Migration_Count'][core] or 1) for core in range(ncores)]  


  if 'L3.g_STTr_Write'  in results:
    results['L3.g_STTr_Write']  = [float(results['L3.g_STTr_Write'][core] or 1) for core in range(ncores)] 

  if 'L3.g_STTr_Reads'  in results:
    results['L3.g_STTr_Reads']  = [float(results['L3.g_STTr_Reads'][core] or 1) for core in range(ncores)] 
  
  if 'L3.g_Sram_Write'  in results:
    results['L3.g_Sram_Write']  = [float(results['L3.g_Sram_Write'][core] or 1) for core in range(ncores)] 
  
  if 'L3.g_Sram_Reads'  in results:
    results['L3.g_Sram_Reads']  = [float(results['L3.g_Sram_Reads'][core] or 1) for core in range(ncores)] 



  if 'interval_timer.a_SRAM'  in results:   
    results['interval_timer.a_SRAM']  = [float(results['interval_timer.a_SRAM'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.b_SRAM'  in results:   
    results['interval_timer.b_SRAM']  = [float(results['interval_timer.b_SRAM'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.c_SRAM'  in results:   
    results['interval_timer.c_SRAM']  = [float(results['interval_timer.c_SRAM'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.d_SRAM'  in results:   
    results['interval_timer.d_SRAM']  = [float(results['interval_timer.d_SRAM'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.e_SRAM'  in results:   
    results['interval_timer.e_SRAM']  = [float(results['interval_timer.e_SRAM'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.f_SRAM'  in results:   
    results['interval_timer.f_SRAM']  = [float(results['interval_timer.f_SRAM'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.g_SRAM'  in results:   
    results['interval_timer.g_SRAM']  = [float(results['interval_timer.g_SRAM'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.a_STTR'  in results:   
    results['interval_timer.a_STTR']  = [float(results['interval_timer.a_STTR'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.b_STTR'  in results:   
    results['interval_timer.b_STTR']  = [float(results['interval_timer.b_STTR'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.c_STTR'  in results:   
    results['interval_timer.c_STTR']  = [float(results['interval_timer.c_STTR'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.d_STTR'  in results:   
    results['interval_timer.d_STTR']  = [float(results['interval_timer.d_STTR'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.e_STTR'  in results:   
    results['interval_timer.e_STTR']  = [float(results['interval_timer.e_STTR'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.f_STTR'  in results:   
    results['interval_timer.f_STTR']  = [float(results['interval_timer.f_STTR'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.g_STTR'  in results:   
    results['interval_timer.g_STTR']  = [float(results['interval_timer.g_STTR'][core] or 1) for core in range(ncores)]  



  if 'interval_timer.force_Migration_SramToSTTram'  in results:   
    results['interval_timer.force_Migration_SramToSTTram']  = [float(results['interval_timer.force_Migration_SramToSTTram'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.force_Migration_STTramToSram'  in results:   
    results['interval_timer.force_Migration_STTramToSram']  = [float(results['interval_timer.force_Migration_STTramToSram'][core] or 1) for core in range(ncores)]



  if 'interval_timer.SRAM_reads_hits'  in results:   
    results['interval_timer.SRAM_reads_hits']  = [float(results['interval_timer.SRAM_reads_hits'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.SRAM_write_hits'  in results:   
    results['interval_timer.SRAM_write_hits']  = [float(results['interval_timer.SRAM_write_hits'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.STTR_reads_hits'  in results:   
    results['interval_timer.STTR_reads_hits']  = [float(results['interval_timer.STTR_reads_hits'][core] or 1) for core in range(ncores)]  
  
  if 'interval_timer.STTR_write_hits'  in results:   
    results['interval_timer.STTR_write_hits']  = [float(results['interval_timer.STTR_write_hits'][core] or 1) for core in range(ncores)] 


  if 'interval_timer.g_SRAM_misses'  in results:   
    results['interval_timer.g_SRAM_misses']  = [float(results['interval_timer.g_SRAM_misses'][core] or 1) for core in range(ncores)]    

  if 'interval_timer.g_STTRAM_misses'  in results:   
    results['interval_timer.g_STTRAM_misses']  = [float(results['interval_timer.g_STTRAM_misses'][core] or 1) for core in range(ncores)]    




  


    
  template = [
    ('  Instructions', 'performance_model.instruction_count', str),
    ('  Cycles',       'performance_model.cycle_count_fixed', format_int),
    ('  IPC',          'performance_model.ipc', format_float(3)),
    ('  Time (ns)',    'performance_model.elapsed_time_fixed', format_ns(0)),
    ('  Idle time (ns)', 'performance_model.idle_elapsed_time', format_ns(0)),
    ('  Idle time (%)',  'performance_model.idle_elapsed_percent', format_pct),
    #('ATD', '', ''),
    #('  SRRIP Wins    ',   'interval_timer.win_srrip', format_int),
    #('  DAAIP Wins    ',   'interval_timer.win_daaip', format_int),
    #('  SRRIP Misses    ',   'interval_timer.SRRIP_Misses', format_int),
    #('  ATD Misses    ',   'interval_timer.ATD_Misses', format_int),
    #('  SRRIP Hits    ',   'interval_timer.SRRIP_Hits', format_int),
    #('  ATD hits    ',   'interval_timer.ATD_Hits', format_int),
    #('  DAAIP Misses    ',   'interval_timer.daaip_misses', format_int),

    #arindam
    #('  Write Threshold',             'interval_timer.Write_Threshold', format_int),
    #('  Read Threshold',              'interval_timer.Read_Threshold', format_int),
    #('  Read to Write Transition Count at an Interval',     'interval_timer.Read_To_Write_Transitions_At_Interval', format_int),
    #('  Read to Write Transition Count at an Eviction',     'interval_timer.Read_To_Write_Transitions_At_Eviction', format_int),
    #('  Write to Read Transition Count at an Interval',     'interval_timer.Write_To_Read_Transitions_At_Interval', format_int),
    ('  Write to Read Transition Count at an Eviction',     'interval_timer.Write_To_Read_Transitions_At_Eviction', format_int),
    ('  Read to Write Transition Count at an Eviction',     'interval_timer.Read_To_Write_Transitions_At_Eviction', format_int),

    ('  Read Intense Block Counter',            'interval_timer.Read_Intense_Block_Counter',               format_int),
    ('  Write Intense Block Counter',           'interval_timer.Write_Intense_Block_Counter',              format_int),
    ('  Deadblock Counter',                     'interval_timer.Deadblock_Counter',                        format_int),
    ('  Valid Blocks Evicted',                  'interval_timer.Valid_Block_Evicted',                      format_int),
    #('  Write Intense Blocks During Migrate',   'interval_timer.Write_Intense_Blocks_During_Migrate',      format_int),
    #('  Read Intense Blocks During Migrate',    'interval_timer.Read_Intense_Blocks_During_Migrate',       format_int),

    ('  Read Intense Blocks M',                'interval_timer.Read_Intense_Blocks_M',                   format_int),
    ('  Write Intense Blocks M',               'interval_timer.Write_Intense_Blocks_M',                  format_int),
    ('  Read Intense Blocks MDuringMigrate',    'interval_timer.Read_Intense_Blocks_MDuringMigrate',       format_int),
    ('  Write Intense Blocks MDuringMigrate',   'interval_timer.Write_Intense_Blocks_MDuringMigrate',      format_int),
    ('  M Count',                              'interval_timer.M_Count',                                 format_int),
    ('  Migrate During Write Count',            'interval_timer.Migrate_During_Write_Count',               format_int),
    
    ('  SRAM Read Before Migration',            'interval_timer.SRAM_Read_Before_Migration',              format_int),
    ('  SRAM Read After Migration',             'interval_timer.SRAM_Read_After_Migration',               format_int),
    ('  SRAM Write Before Migration',           'interval_timer.SRAM_Write_Before_Migration',             format_int),
    ('  SRAM Write After Migration',            'interval_timer.SRAM_Write_After_Migration',              format_int),
    ('  STTRAM Read Before Migration',          'interval_timer.STTRAM_Read_Before_Migration',            format_int),
    ('  STTRAM Read After Migration',           'interval_timer.STTRAM_Read_After_Migration',             format_int),
    ('  STTRAM Write Before Migration',         'interval_timer.STTRAM_Write_Before_Migration',           format_int),
    ('  STTRAM Write After Migration',          'interval_timer.STTRAM_Write_After_Migration',            format_int),

    ('  living SRAM Blocks Evicted',          'interval_timer.living_SRAM_Blocks_Evicted',            format_int),
    ('  dead SRAM Blocks Evicted',          'interval_timer.dead_SRAM_Blocks_Evicted',            format_int),

    ('  single Migration Count',          'interval_timer.single_Migration_Count',            format_int),
    ('  double Migration Count',          'interval_timer.double_Migration_Count',            format_int),
    ('  no Migration Count',              'interval_timer.no_Migration_Count',                format_int),

    ('  g STTr Write',              'L3.g_STTr_Write',                format_int),
    ('  g STTr Reads',              'L3.g_STTr_Reads',                format_int),
    ('  g Sram Write',              'L3.g_Sram_Write',                format_int),
    ('  g Sram Reads',              'L3.g_Sram_Reads',                format_int),

    ('  a SRAM',              'interval_timer.a_SRAM',                format_int),
    ('  b SRAM',              'interval_timer.b_SRAM',                format_int),
    ('  c SRAM',              'interval_timer.c_SRAM',                format_int),
    ('  d SRAM',              'interval_timer.d_SRAM',                format_int),
    ('  e SRAM',              'interval_timer.e_SRAM',                format_int),
    ('  f SRAM',              'interval_timer.f_SRAM',                format_int),
    ('  g SRAM',              'interval_timer.g_SRAM',                format_int),
    ('  a STTR',              'interval_timer.a_STTR',                format_int),
    ('  b STTR',              'interval_timer.b_STTR',                format_int),
    ('  c STTR',              'interval_timer.c_STTR',                format_int),
    ('  d STTR',              'interval_timer.d_STTR',                format_int),
    ('  e STTR',              'interval_timer.e_STTR',                format_int),
    ('  f STTR',              'interval_timer.f_STTR',                format_int),
    ('  g STTR',              'interval_timer.g_STTR',                format_int),

    ('  force Migration SramToSTTram',              'interval_timer.force_Migration_SramToSTTram',                format_int),
    ('  force Migration STTramToSram',              'interval_timer.force_Migration_STTramToSram',                format_int),

    ('  SRAM reads hits',              'interval_timer.SRAM_reads_hits',                format_int),
    ('  SRAM write hits',              'interval_timer.SRAM_write_hits',                format_int),
    ('  STTR reads hits',              'interval_timer.STTR_reads_hits',                format_int),
    ('  STTR write hits',              'interval_timer.STTR_write_hits',                format_int),

    ('  g_SRAM_misses',              'interval_timer.g_SRAM_misses',                format_int),
    ('  g_STTRAM_misses',              'interval_timer.g_STTRAM_misses',                format_int),


  ]

  if 'branch_predictor.num-incorrect' in results:
    results['branch_predictor.missrate'] = [ 100 * float(results['branch_predictor.num-incorrect'][core])
      / ((results['branch_predictor.num-correct'][core] + results['branch_predictor.num-incorrect'][core]) or 1) for core in range(ncores) ]
    results['branch_predictor.mpki'] = [ 1000 * float(results['branch_predictor.num-incorrect'][core])
      / (results['performance_model.instruction_count'][core] or 1) for core in range(ncores) ]
    template += [
      ('Branch predictor stats', '', ''),
      ('  num correct',  'branch_predictor.num-correct', str),
      ('  num incorrect','branch_predictor.num-incorrect', str),
      ('  misprediction rate', 'branch_predictor.missrate', lambda v: '%.2f%%' % v),
      ('  mpki', 'branch_predictor.mpki', lambda v: '%.2f' % v),
    ]

  template += [
    ('TLB Summary', '', ''),
  ]

  for tlb in ('itlb', 'dtlb', 'stlb'):
    if '%s.access'%tlb in results:
      results['%s.missrate'%tlb] = map(lambda (a,b): 100*a/float(b or 1), zip(results['%s.miss'%tlb], results['%s.access'%tlb]))
      results['%s.mpki'%tlb] = map(lambda (a,b): 1000*a/float(b or 1), zip(results['%s.miss'%tlb], results['performance_model.instruction_count']))
      template.extend([
        ('  %s' % {'itlb': 'I-TLB', 'dtlb': 'D-TLB', 'stlb': 'L2 TLB'}[tlb], '', ''),
        ('    num accesses', '%s.access'%tlb, str),
        ('    num misses', '%s.miss'%tlb, str),
        ('    miss rate', '%s.missrate'%tlb, lambda v: '%.2f%%' % v),
        ('    mpki', '%s.mpki'%tlb, lambda v: '%.2f' % v),
      ])

  template += [
    ('Cache Summary', '', ''),
  ]
  allcaches = [ 'L1-I', 'L1-D' ] + [ 'L%u'%l for l in range(2, 5) ]
  existcaches = [ c for c in allcaches if '%s.loads'%c in results ]
  for c in existcaches:
    results['%s.accesses'%c] = map(sum, zip(results['%s.loads'%c], results['%s.stores'%c]))
    results['%s.misses'%c] = map(sum, zip(results['%s.load-misses'%c], results.get('%s.store-misses-I'%c, results['%s.store-misses'%c])))
    results['%s.missrate'%c] = map(lambda (a,b): 100*a/float(b) if b else float('inf'), zip(results['%s.misses'%c], results['%s.accesses'%c]))
    results['%s.mpki'%c] = map(lambda (a,b): 1000*a/float(b) if b else float('inf'), zip(results['%s.misses'%c], results['performance_model.instruction_count']))
    template.extend([
      ('  Cache %s'%c, '', ''),
      ('    num cache accesses', '%s.accesses'%c, str),
      ('    num cache misses', '%s.misses'%c, str),
      ('    miss rate', '%s.missrate'%c, lambda v: '%.2f%%' % v),
      ('    mpki', '%s.mpki'%c, lambda v: '%.2f' % v),
    ])

############################################################




 # results['L3.NumberOfL3WriteFromDirectory'] = [float(results['L3.NumberOfL3WriteFromDirectory'][c]) //sn anushree
 #   for c in range(ncores)
 # ]

 # results['L3.NumberOfL3WriteFromL2'] = [float(results['L3.NumberOfL3WriteFromL2'][c]) //sn anushree
 #   for c in range(ncores)
 # ]

#template.extend([
#  ('  L3NumberOfL3WritesFromL2', 'L3.NumberOfL3WriteFromL2', format_int),
#  ('  L3NumberOfL3WritesFromDirectory', 'L3.NumberOfL3WriteFromDirectory', format_int),
#])
############################################################

  allcaches = [ 'nuca-cache', 'dram-cache' ]
  existcaches = [ c for c in allcaches if '%s.reads'%c in results ]
  for c in existcaches:
    results['%s.accesses'%c] = map(sum, zip(results['%s.reads'%c], results['%s.writes'%c]))
    results['%s.misses'%c] = map(sum, zip(results['%s.read-misses'%c], results['%s.write-misses'%c]))
    results['%s.missrate'%c] = map(lambda (a,b): 100*a/float(b) if b else float('inf'), zip(results['%s.misses'%c], results['%s.accesses'%c]))
    icount = sum(results['performance_model.instruction_count'])
    icount /= len([ v for v in results['%s.accesses'%c] if v ]) # Assume instructions are evenly divided over all cache slices
    results['%s.mpki'%c] = map(lambda a: 1000*a/float(icount) if icount else float('inf'), results['%s.misses'%c])
    template.extend([
      ('  %s cache'% c.split('-')[0].upper(), '', ''),
      ('    num cache accesses', '%s.accesses'%c, str),
      ('    num cache misses', '%s.misses'%c, str),
      ('    miss rate', '%s.missrate'%c, lambda v: '%.2f%%' % v),
      ('    mpki', '%s.mpki'%c, lambda v: '%.2f' % v),
    ])
    
  results['dram.accesses'] = map(sum, zip(results['dram.reads'], results['dram.writes']))
  results['dram.avglatency'] = map(lambda (a,b): a/b if b else float('inf'), zip(results['dram.total-access-latency'], results['dram.accesses']))
  template += [
    ('DRAM summary', '', ''),
    ('  num dram accesses', 'dram.accesses', str),
    ('  average dram access latency (ns)', 'dram.avglatency', format_ns(2)),
  ]
  if 'dram.total-read-queueing-delay' in results:
    results['dram.avgqueueread'] = map(lambda (a,b): a/(b or 1), zip(results['dram.total-read-queueing-delay'], results['dram.reads']))
    results['dram.avgqueuewrite'] = map(lambda (a,b): a/(b or 1), zip(results['dram.total-write-queueing-delay'], results['dram.writes']))
    template.append(('  average dram read queueing delay', 'dram.avgqueueread', format_ns(2)))
    template.append(('  average dram write queueing delay', 'dram.avgqueuewrite', format_ns(2)))
  else:
    results['dram.avgqueue'] = map(lambda (a,b): a/(b or 1), zip(results.get('dram.total-queueing-delay', [0]*ncores), results['dram.accesses']))
    template.append(('  average dram queueing delay', 'dram.avgqueue', format_ns(2)))
  if 'dram-queue.total-time-used' in results:
    results['dram.bandwidth'] = map(lambda a: 100*a/time0 if time0 else float('inf'), results['dram-queue.total-time-used'])
    template.append(('  average dram bandwidth utilization', 'dram.bandwidth', lambda v: '%.2f%%' % v))

  if 'L1-D.loads-where-dram-local' in results:
    results['L1-D.loads-where-dram'] = map(sum, zip(results['L1-D.loads-where-dram-local'], results['L1-D.loads-where-dram-remote']))
    results['L1-D.stores-where-dram'] = map(sum, zip(results['L1-D.stores-where-dram-local'], results['L1-D.stores-where-dram-remote']))
    template.extend([
        ('Coherency Traffic', '', ''),
        ('  num loads from dram', 'L1-D.loads-where-dram' , str),
        #('  num stores from dram', 'L1-D.stores-where-dram' , str),
        ('  num loads from dram cache', 'L1-D.loads-where-dram-cache' , str),
        #('  num stores from dram cache', 'L1-D.stores-where-dram-cache' , str),
        ('  num loads from remote cache', 'L1-D.loads-where-cache-remote' , str),
        #('  num stores from remote cache', 'L1-D.stores-where-cache-remote' , str),
      ])

  lines = []
  lines.append([''] + [ 'Core %u' % i for i in range(ncores) ])

  for title, name, func in template:
    line = [ title ]
    if name and name in results:
      for core in range(ncores):
        line.append(' '+func(results[name][core]))
    else:
      line += [''] * ncores
    lines.append(line)


  widths = [ max(10, max([ len(l[i]) for l in lines ])) for i in range(len(lines[0])) ]
  for j, line in enumerate(lines):
    output.write(' | '.join([ ('%%%s%us' % ((j==0 or i==0) and '-' or '', widths[i])) % line[i] for i in range(len(line)) ]) + '\n')



if __name__ == '__main__':
  def usage():
    print 'Usage:', sys.argv[0], '[-h (help)] [--partial <section-start>:<section-end> (default: roi-begin:roi-end)] [-d <resultsdir (default: .)>]'

  jobid = 0
  resultsdir = '.'
  partial = None

  try:
    opts, args = getopt.getopt(sys.argv[1:], "hj:d:", [ 'partial=' ])
  except getopt.GetoptError, e:
    print e
    usage()
    sys.exit()
  for o, a in opts:
    if o == '-h':
      usage()
      sys.exit()
    if o == '-d':
      resultsdir = a
    if o == '-j':
      jobid = long(a)
    if o == '--partial':
      if ':' not in a:
        sys.stderr.write('--partial=<from>:<to>\n')
        usage()
      partial = a.split(':')

  if args:
    usage()
    sys.exit(-1)

  generate_simout(jobid = jobid, resultsdir = resultsdir, partial = partial)

