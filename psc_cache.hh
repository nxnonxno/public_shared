#include <unordered_map>

#include "base/statistics.hh"
#include "mem/port.hh"
#include "params/PSCache.hh"
#include "sim/clocked_object.hh"
#include "sim/byteswap.hh"
#include "arch/x86/pagetable.hh"

namespace gem5
{
    namespace X86ISA {
        class PSCache : public ClockedObject
        {
            typedef std::unordered_map<Addr, uint8_t*> cacheStore_t;
        private:

            /// Latency to check the cache. Number of cycles for both hit and miss
            const Cycles latency;

            /// The block size for the cache
            const unsigned blockSize;

            /// Number of blocks in the cache (size of cache / block size)
            const unsigned capacity;


            /// For tracking the miss latency
            Tick missTime;

            /// An incredibly simple cache storage. Maps block addresses to data
            
            cacheStore_t cacheStore[4];
            // std::unordered_map<Addr, uint8_t*> cacheStore_l2;
            // std::unordered_map<Addr, uint8_t*> cacheStore_l3;
            // std::unordered_map<Addr, uint8_t*> cacheStore_l4;
            template <typename T>
            inline T
            getLE(uint8_t * &blk) const
            {
                return letoh(*(T*)blk);
            }

            void read_block_to_pte(uint8_t * &blk,PageTableEntry &pte,unsigned blk_size)
            {
                pte = getLE<uint64_t>(blk);
            }
            void write_pte_to_block(uint8_t * &blk,PageTableEntry &pte,unsigned blk_size)
            {
                *(uint64_t*)blk = mbits(pte, 63, 0);
            }
            Addr extract_tag(Addr vaddr,uint8_t level)
            {
                return vaddr;
            }

            /// Cache statistics
        protected:
            struct SimpleCacheStats : public statistics::Group
            {
                SimpleCacheStats(statistics::Group *parent);
                statistics::Scalar hits;
                statistics::Scalar misses;
                statistics::Histogram missLatency;
                statistics::Formula hitRatio;
            } stats;

        public:
            uint8_t access(PageTableEntry &pte,Addr vaddr);
            //void access_psc(PageTableEntry &pte,Addr vaddr,uint8_t* hit_table);
            void access_psc(PacketPtr pkt);
            void insert_psc(uint8_t level,Addr vaddr,PageTableEntry &pte);

            /** constructor
            */
            PSCache(const PSCacheParams &p);

            /**
            * Get a port with a given name and index. This is used at
            * binding time and returns a reference to a protocol-agnostic
            * port.
            *
            * @param if_name Port name
            * @param idx Index in the case of a VectorPort
            *
            * @return A reference to the given port
            */
   

        };
    }
}