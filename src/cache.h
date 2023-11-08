/*******************************************************
                          cache.h
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

enum state_e : uint8_t {
   INVALID = 0,
   CLEAN = 1,
   MODIFIED = 2,
};

class cacheLine 
{
protected:
   ulong tag;
   state_e state;
   ulong seq; 
 
public:
   cacheLine()                   { tag = 0; state = INVALID; }
   ulong get_tag()               { return tag; }
   ulong get_state()             { return state;}
   ulong get_seq()               { return seq; }
   void set_seq(ulong Seq)       { seq = Seq;}
   void set_state(state_e flags) { state = flags;}
   void set_tag(ulong a)         { tag = a; }
   void invalidate()             { tag = 0; state = INVALID; }
   bool is_valid()               { return (state != INVALID); }
};

class Cache
{
protected:
   ulong size_, block_size_, assoc_, num_sets_, num_index_bits_, num_block_offset_bits_, tag_mask_, num_blocks_;
   ulong num_reads_, num_read_misses_, num_writes_, num_write_misses_, num_write_backs_;

   //******///
   //add coherence counters here///
   //******///

   cacheLine **cache;
   ulong calc_tag(ulong addr)     { return (addr >> num_block_offset_bits_);}
   ulong calc_index(ulong addr)   { return ((addr >> num_block_offset_bits_) & tag_mask_);}
   ulong calcAddr4Tag(ulong tag)  { return (tag << (num_block_offset_bits_));}
   
public:
    ulong currentCycle;  
     
    Cache(int,int,int);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine *findLine(ulong addr);
   cacheLine *getLRU(ulong);
   
   ulong get_read_misses()    {return num_read_misses_;} 
   ulong get_write_misses()   {return num_write_misses_;} 
   ulong get_reads()          {return num_reads_;}       
   ulong get_writes()         {return num_writes_;}
   ulong get_write_backs()    {return num_write_backs_;}
   
   void write_back(ulong)     {num_write_backs_++;}
   void Access(ulong,uchar);
   void print_stats();
   void update_LRU(cacheLine *);

   //******//
   //add other functions to handle bus transactions///
   //******///

};

#endif
