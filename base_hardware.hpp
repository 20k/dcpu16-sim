#ifndef BASE_HARDWARE_HPP_INCLUDED
#define BASE_HARDWARE_HPP_INCLUDED

#include <stdint.h>
#include <span>
#include <vector>

namespace dcpu
{
    namespace sim
    {
        struct CPU;

        struct time_state
        {
            uint64_t time_ms = 0;

            virtual ~time_state(){}
        };

        struct world_base
        {
            virtual ~world_base(){}
        };

        struct hardware
        {
            uint32_t hardware_id = 0;
            uint16_t hardware_version = 0;
            uint32_t manufacturer_id = 0;

            constexpr virtual void interrupt2(std::span<hardware*> all_hardware, world_base* state, CPU& c){return interrupt(state, c);}
            constexpr virtual void interrupt(world_base* state, CPU& c){}
            constexpr virtual void step(world_base* state, CPU& c){}
            constexpr virtual void reset(){}
            constexpr virtual hardware* clone(){return new hardware(*this);}
        };
    }
}

#endif // BASE_HARDWARE_HPP_INCLUDED
