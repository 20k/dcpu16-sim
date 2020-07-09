#ifndef BASE_HARDWARE_HPP_INCLUDED
#define BASE_HARDWARE_HPP_INCLUDED

namespace dcpu
{
    namespace sim
    {
        struct CPU;

        struct time_state
        {
            uint64_t time_ms = 0;
        };

        struct world_base
        {

        };

        struct hardware
        {
            uint32_t hardware_id = 0;
            uint16_t hardware_version = 0;
            uint32_t manufacturer_id = 0;

            constexpr virtual void interrupt(world_base* state, CPU& c){}
            constexpr virtual void step(world_base* state, CPU& c){}
        };

        struct clock : hardware
        {
            uint64_t last_time_ms = 0;

            constexpr virtual void interrupt(world_base* state, CPU& c){}
            constexpr virtual void step(world_base* state, CPU& c){}
        };
    }
}

#endif // BASE_HARDWARE_HPP_INCLUDED
