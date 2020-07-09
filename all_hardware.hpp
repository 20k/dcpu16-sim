#ifndef ALL_HARDWARE_HPP_INCLUDED
#define ALL_HARDWARE_HPP_INCLUDED

#include "hardware_clock.hpp"
#include <memory>

namespace dcpu
{
    namespace sim
    {
        enum class hardware_type
        {
            CLOCK,
            NONE
        };

        constexpr hardware* make_hardware(hardware_type type)
        {
            if(type == hardware_type::CLOCK)
                return new clock;

            return new hardware;
        }
    }
}


#endif // ALL_HARDWARE_HPP_INCLUDED
