#ifndef HARDWARE_CLOCK_HPP_INCLUDED
#define HARDWARE_CLOCK_HPP_INCLUDED

#include "base_hardware.hpp"
#include "base_sim.hpp"

namespace dcpu
{
    namespace sim
    {
        ///https://github.com/lucaspiller/dcpu-specifications/blob/master/clock.txt
        ///doesn't suffer from interrupts drifting
        struct clock : hardware
        {
            uint64_t time_started_ms = 0;
            uint64_t last_tick_time_ms = 0;
            uint16_t tick_divisor = 0;
            bool on = false;

            uint64_t interrupt_message = 0;

            clock()
            {
                hardware_id = 0x12d0b402;
                hardware_version = 1;
                manufacturer_id = 0xFFFFFFFB;
            }

            constexpr virtual void interrupt(world_base* state, CPU& c) override
            {
                time_state* tstate = dynamic_cast<time_state*>(state);

                if(tstate == nullptr)
                    return;

                if(c.regs[A_REG] == 0)
                {
                    if(c.regs[B_REG] == 0)
                    {
                        on = false;
                        interrupt_message = 0; ///?
                    }
                    else
                    {
                        on = true;
                        tick_divisor = c.regs[B_REG];
                        time_started_ms = tstate->time_ms;
                        last_tick_time_ms = time_started_ms;
                    }
                }

                if(!on)
                    return;

                if(c.regs[A_REG] == 1)
                {
                    uint64_t diff_ms = tstate->time_ms - time_started_ms;

                    uint16_t tick_count = ((double)diff_ms) / ((1000. * (uint64_t)tick_divisor) / 60.);

                    c.regs[C_REG] = tick_count;
                }

                if(c.regs[A_REG] == 2)
                {
                    interrupt_message = c.regs[B_REG];
                }
            }

            constexpr virtual void step(world_base* state, CPU& c) override
            {
                time_state* tstate = dynamic_cast<time_state*>(state);

                if(tstate == nullptr)
                    return;

                if(!on)
                    return;

                if(interrupt_message == 0)
                    return;

                uint64_t diff_ms = tstate->time_ms - last_tick_time_ms;

                uint16_t tick_count = ((double)diff_ms) / ((1000. * (uint64_t)tick_divisor) / 60.);

                if(tick_count > 0)
                {
                    last_tick_time_ms = (uint64_t)(tick_count * ((1000. * (uint64_t)tick_divisor) / 60.)) + last_tick_time_ms;

                    interrupt_type type;
                    type.is_software = 0;
                    type.message = interrupt_message;

                    c.add_interrupt(type);
                }
            }

            constexpr virtual void reset() override
            {
                time_started_ms = 0;
                last_tick_time_ms = 0;
                tick_divisor = 0;
                on = false;
                interrupt_message = 0;
            }

            constexpr virtual hardware* clone() override
            {
                hardware* ret = new hardware;
                *ret = *this;

                return ret;
            }
        };
    }
}

#endif // HARDWARE_CLOCK_HPP_INCLUDED
