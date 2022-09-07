#include "learning_gem5/part4/psc_cache.hh"
#include "base/compiler.hh"
#include "base/random.hh"
#include "debug/PSCache.hh"
#include "sim/system.hh"
#include "base/random.hh"
namespace gem5
{
   namespace X86ISA {
        PSCache::PSCache(const PSCacheParams &p) :
        ClockedObject(p),latency(p.latency),
        blockSize(8),capacity(p.size / blockSize),stats(this)
        {
            
        }
        
        uint8_t 
        PSCache::access(PageTableEntry &pte,Addr vaddr)
        {
            uint8_t hit_table[4];
            schedule(new EventFunctionWrapper([this, pkt]{ access_psc(pkt); },
                                             "accessEvent", true),
                    clockEdge(latency));
           
            for(uint8_t i = 0;i < 4;i++)
            if(hit_table[i] == 1 )
            {
                return i+1;
            }            
            return -1;
        }
        // void 
        // PSCache::access_psc(PageTableEntry &pte,Addr vaddr,uint8_t* hit_table)
        void
        PSCache::access_psc(PacketPtr pkt)
        {
            
        }
        void 
        PSCache::insert_psc(uint8_t level,Addr vaddr,PageTableEntry &pte)
        {
            assert(level >= 1 && level <= 4);     
            if (cacheStore[level - 1].size() >= capacity) {
                // Select random thing to evict. This is a little convoluted since we
                // are using a std::unordered_map. See http://bit.ly/2hrnLP2
                int bucket, bucket_size;
                do {
                    bucket = random_mt.random(0, (int)cacheStore[level - 1].bucket_count() - 1);
                } while ( (bucket_size = cacheStore[level - 1].bucket_size(bucket)) == 0 );
                auto block = std::next(cacheStore[level - 1].begin(bucket),
                                        random_mt.random(0, bucket_size - 1));

                DPRINTF(PSCache, "Removing addr %#x\n", block->first);


                // Delete this entry
                cacheStore[level - 1].erase(block->first);
            }

            DPRINTF(PSCache, "Inserting \n");
            //DDUMP(SimpleCache, pkt->getConstPtr<uint8_t>(), blockSize);

            // Allocate space for the cache block data
            uint8_t *data = new uint8_t[blockSize];

            // Insert the data and address into the cache store
            cacheStore[level - 1][extract_tag(vaddr,level)] = data;

            // Write the data into the cache
            write_pte_to_block(data,pte,blockSize);
    
        }
    }
}