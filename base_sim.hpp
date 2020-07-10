#ifndef BASE_SIM_HPP_INCLUDED
#define BASE_SIM_HPP_INCLUDED

#include <tuple>
#include <variant>
#include <assert.h>
#include <dcpu16-asm/stack_vector.hpp>
#include <dcpu16-asm/shared.hpp>
#include <optional>
#include <stdint.h>
#include <vector>
#include <tuple>
//#include <cstdio>
#include "base_hardware.hpp"

#define MEM_SIZE 0x10000
#define REG_NUM 12
#define MAX_INTERRUPTS 256

#define A_REG 0
#define B_REG 1
#define C_REG 2
#define X_REG 3
#define Y_REG 4
#define Z_REG 5
#define I_REG 6
#define J_REG 7
#define PC_REG 8
#define SP_REG 9
#define EX_REG 10
#define IA_REG 11

namespace dcpu
{

namespace instr_type
{
    enum type
    {
        A,
        B,
        C
    };
}

constexpr
std::tuple<uint16_t, uint16_t, uint16_t> decompose_type_a(uint16_t v)
{
    uint16_t o =  v & 0b0000000000011111;
    uint16_t b = (v & 0b0000001111100000) >> 5;
    uint16_t a = (v & 0b1111110000000000) >> 10;

    return {o, a, b};
}

constexpr
std::tuple<uint16_t, uint16_t> decompose_type_b(uint16_t v)
{
    uint16_t o = (v & 0b0000001111100000) >> 5;
    uint16_t a = (v & 0b1111110000000000) >> 10;

    return {o, a};
}

constexpr
uint16_t decompose_type_c(uint16_t v)
{
    return (v >> 10) & 0b0000000000111111;
}

constexpr
instr_type::type get_type(uint16_t v)
{
    auto [o, a, b] = decompose_type_a(v);

    if(o == 0)
    {
        auto [bo, bb] = decompose_type_b(v);

        if(bo == 0)
        {
            return instr_type::type::C;
        }

        return instr_type::type::B;
    }
    else
    {
        return instr_type::type::A;
    }
}

namespace sim
{
    // so
    // in C++ we could use pointers, and a table of constants
    struct location
    {
        struct memory{uint16_t v = 0;};
        struct reg{uint16_t v = 0;};
        struct constant{uint16_t v = 0;};

        std::variant<memory, reg, constant> l;

        constexpr location(memory m) : l{m}
        {
        }

        constexpr location(reg r) : l{r}
        {
        }

        constexpr location(constant c) : l{c}
        {
        }
    };
}

constexpr
std::pair<bool, uint16_t> overflow_add(uint16_t v1, uint16_t v2)
{
    uint32_t sum = (uint32_t)v1 + (uint32_t)v2;

    return {sum > 0xffff, sum & 0xffff};
}

namespace sim
{
    struct CPU;
}

constexpr
std::optional<dcpu::sim::location> exec_value_reference(dcpu::sim::CPU& exec, uint16_t instruction_location, uint16_t arg, arg_pos::type pos);

constexpr
int get_instruction_length(uint16_t v);

constexpr
uint16_t get_cycle_time_arg(uint16_t arg)
{
    if(arg >= 0x10 && arg <= 0x17)
        return 1;

    if(arg == 0x1a)
        return 1;

    if(arg == 0x1e)
        return 1;

    if(arg == 0x1f)
        return 1;

    return 0;
}

struct interrupt_type
{
    uint16_t is_software = 0;
    uint16_t message = 0;

    constexpr interrupt_type(){}
};

constexpr
uint16_t get_cycle_time_instr(uint16_t instr)
{
    instr_type::type type = get_type(instr);
    uint16_t time = 0;

    if(type == instr_type::A)
    {
        auto [o, a, b] = decompose_type_a(instr);

        time += get_cycle_time_arg(a);
        time += get_cycle_time_arg(b);

        std::array<uint16_t, 32> times = {0, 1, 2, 2, 2, 2, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 3, 3, 4, 4, 2, 2};

        time += times[o];
    }

    if(type == instr_type::B)
    {
        auto [o, a] = decompose_type_b(instr);

        time += get_cycle_time_arg(a);

        std::array<uint16_t, 32> times = {0, 3, 0, 0, 0, 0, 0, 0, 4, 1, 1, 3, 2, 0, 0, 0, 2, 4, 4, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0};

        time += times[o];
    }

    if(type == instr_type::C)
    {
        time = 1;
    }

    return time;
}

template<typename T, long long unsigned int N>
struct stack_ring
{
    std::array<T, N> mem;
    uint16_t start = 0;
    uint16_t fin = 0;

    constexpr
    stack_ring(){}

    constexpr
    bool push_back(T in)
    {
        uint16_t udiff = fin - start;

        if(udiff > N)
            return false;

        uint16_t cirq = fin % N;

        mem[cirq] = in;

        fin++;

        return true;
    }

    constexpr
    T pop_front()
    {
        T val = mem[start % N];

        start++;

        return val;
    }

    void clear()
    {
        fin = start;
    }

    constexpr
    uint16_t size()
    {
        return (fin - start);
    }
};

#define NUM_CHANNELS 256

struct fabric_slot
{
    uint16_t last_written_id = 0;
    uint16_t last_read_id = 0;

    uint16_t next_written_id = 0;
    uint16_t next_read_id = 0;

    std::optional<uint16_t> current_value = std::nullopt;
};

namespace sim
{
    struct fabric
    {
        std::array<fabric_slot, NUM_CHANNELS> channels;
    };
}

struct presented_info
{
    bool has_value = false;
    uint16_t target = 0;
    uint16_t value = 0;

    std::optional<uint16_t> assigned_id = std::nullopt;
};

struct waiting_info
{
    bool has_value = false;
    uint16_t target = 0;
    dcpu::sim::location loc = dcpu::sim::location::constant{0};

    std::optional<uint16_t> assigned_id = std::nullopt;
};

namespace sim
{
    struct world_base;
    struct hardware;

    struct CPU
    {
        std::array<uint16_t, MEM_SIZE> mem = {};
        std::array<uint16_t, REG_NUM> regs = {};

        presented_info presented_value; ///pass a message to another piece of hardware
        waiting_info waiting_location; ///waiting to receive a message from a piece of hardware
        //std::array<bool, 65536> pending_writes = {};
        //std::array<bool, 65536> pending_reads = {};

        stack_ring<interrupt_type, MAX_INTERRUPTS> interrupts;

        uint16_t interrupt_dequeueing_enabled = 0;
        uint64_t cycle_count = 0;
        uint64_t next_instruction_cycle = 0;
        bool skipping = false;
        uint16_t hwid = -1;
        int Hz = 1000;

        constexpr
        CPU(uint16_t _hwid){hwid = _hwid;}

        constexpr
        CPU(){}

        constexpr
        uint16_t fetch_location(location l)
        {
            if(std::holds_alternative<location::reg>(l.l))
            {
                return regs[std::get<location::reg>(l.l).v];
            }

            if(std::holds_alternative<location::memory>(l.l))
            {
                return mem[std::get<location::memory>(l.l).v];
            }

            if(std::holds_alternative<location::constant>(l.l))
            {
                return std::get<location::constant>(l.l).v;
            }

            return 0xDEAD;
        }

        constexpr
        void set_location(location l, uint16_t val)
        {
            if(std::holds_alternative<location::reg>(l.l))
            {
                regs[std::get<location::reg>(l.l).v] = val;
            }

            if(std::holds_alternative<location::memory>(l.l))
            {
                mem[std::get<location::memory>(l.l).v] = val;
            }

            if(std::holds_alternative<location::constant>(l.l))
            {
                // nothing
            }
        }

        constexpr
        void load(stack_vector<uint16_t, MEM_SIZE>& in, uint16_t pc)
        {
            for(uint16_t i=0; i < (uint16_t)in.idx; i++)
            {
                uint16_t type = i + pc;

                mem[type] = in.svec[i];
            }

            //regs[PC_REG] = pc;
        }

        constexpr
        uint16_t index(uint16_t in)
        {
            return mem[in];
        }

        constexpr
        bool can_add_interrupts()
        {
            return fetch_location(location::reg{IA_REG}) == 0;
        }

        constexpr
        bool add_interrupt(interrupt_type type)
        {
             // HCF
            if(interrupts.size() > 256)
                return true;

            if(can_add_interrupts())
            {
                interrupts.push_back(type);
            }

            return false;
        }

        constexpr
        bool step(fabric* fabric_opt = nullptr, stack_vector<hardware*, 65536>* hardware_opt = nullptr, world_base* world_component = nullptr)
        {
            if(presented_value.has_value)
            {
                next_instruction_cycle++;
                return false;
            }

            if(waiting_location.has_value)
            {
                next_instruction_cycle++;
                return false;
            }

            uint16_t instruction_location = regs[PC_REG];
            uint16_t instr = mem[instruction_location];

            regs[PC_REG] += (uint16_t)get_instruction_length(instr);

            {
                auto [o, a, b] = decompose_type_a(instr);

                if(o == 0x10 || o == 0x11 || o == 0x12 || o == 0x13 || o == 0x14 || o == 0x15 || o == 0x16 || o == 0x17)
                {
                    if(skipping)
                    {
                        next_instruction_cycle++;
                        return false;
                    }
                }

                ///extensions for multiprocessor, ifw/ifr
                if(get_type(instr) == instr_type::B)
                {
                    auto [o, a] = decompose_type_b(instr);

                    if(o == 0x1a || o == 0x1b)
                    {
                        if(skipping)
                        {
                            next_instruction_cycle++;
                            return false;
                        }
                    }
                }

                ///NEED TO TEST THIS WITH CYCLE ACCURACY8
                if(skipping)
                {
                    next_instruction_cycle++;
                    skipping = false;
                    return false;
                }
            }

            next_instruction_cycle += get_cycle_time_instr(instr);

            instr_type::type type = get_type(instr);

            if(type == instr_type::A)
            {
                auto [o, a, b] = decompose_type_a(instr);

                auto exec_a_opt = exec_value_reference(*this, instruction_location, a, arg_pos::A);
                auto exec_b_opt = exec_value_reference(*this, instruction_location, b, arg_pos::B);

                if(!exec_a_opt.has_value())
                {
                    return true;
                }

                if(!exec_b_opt.has_value())
                {
                    return true;
                }

                location b_location = exec_b_opt.value();

                uint16_t a_value = fetch_location(exec_a_opt.value());
                uint16_t b_value = fetch_location(exec_b_opt.value());

                location ex_location = location::reg{EX_REG};

                if(o == 0x01)
                {
                    set_location(b_location, a_value);
                }

                else if(o == 0x02)
                {
                    uint32_t sum = (uint32_t)b_value + (uint32_t)a_value;

                    uint16_t overflow = sum > 0xffff;

                    set_location(b_location, sum & 0xffff);

                    set_location(ex_location, overflow);
                }

                else if(o == 0x03)
                {
                    uint32_t sum = (uint32_t)b_value - (uint32_t)a_value;

                    uint16_t overflow = sum > 0xffff ? 0xffff : 0;

                    set_location(b_location, sum & 0xffff);
                    set_location(ex_location, overflow);
                }

                else if(o == 0x04)
                {
                    uint32_t mult = (uint32_t)b_value * (uint32_t)a_value;
                    uint32_t overflow = (mult >> 16) & 0xffff;

                    set_location(b_location, mult & 0xffff);
                    set_location(ex_location, overflow & 0xffff);
                }

                else if(o == 0x05)
                {
                    int16_t signed_a = a_value;
                    int16_t signed_b = b_value;

                    int64_t signed_extended_a = signed_a;
                    int64_t signed_extended_b = signed_b;

                    int64_t signed_mul = signed_extended_b * signed_extended_a;

                    int32_t truncate = signed_mul;
                    uint32_t shorted = truncate;

                    uint32_t overflow = (shorted >> 16) & 0xffff;

                    set_location(b_location, shorted & 0xffff);
                    set_location(ex_location, overflow & 0xffff);
                }

                else if(o == 0x06)
                {
                    if(a_value == 0)
                    {
                        set_location(b_location, 0);
                        set_location(ex_location, 0);
                    }
                    else
                    {
                        uint16_t sum = b_value / a_value;
                        uint32_t high_sum = ((uint32_t)b_value << 16) / (uint32_t)a_value;

                        set_location(b_location, sum);
                        set_location(ex_location, high_sum & 0xffff);
                    }
                }

                else if(o == 0x07)
                {
                    if(a_value == 0)
                    {
                        set_location(b_location, 0);
                        set_location(ex_location, 0);
                    }
                    else
                    {
                        int16_t signed_b = b_value;
                        int16_t signed_a = a_value;

                        uint32_t signed_res = (int32_t)signed_b / (int32_t)signed_a;
                        uint32_t overflow = ((int64_t)signed_b << 16) / ((int64_t)signed_a);

                        set_location(b_location, signed_res & 0xffff);
                        set_location(ex_location, overflow & 0xffff);
                    }
                }

                else if(o == 0x08)
                {
                    if(a_value == 0)
                    {
                        set_location(b_location, 0);
                    }
                    else
                    {
                        set_location(b_location, b_value % a_value);
                    }
                }

                else if(o == 0x09)
                {
                    if(a_value == 0)
                    {
                        set_location(b_location, 0);
                    }
                    else
                    {
                        int16_t signed_a = a_value;
                        int16_t signed_b = b_value;

                        int16_t res = signed_b % signed_a;
                        uint16_t sres = res;

                        set_location(b_location, sres);
                    }
                }

                else if(o == 0x0a)
                {
                    set_location(b_location, b_value & a_value);
                }

                else if(o == 0x0b)
                {
                    set_location(b_location, b_value | a_value);
                }

                else if(o == 0x0c)
                {
                    set_location(b_location, b_value ^ a_value);
                }

                else if(o == 0x0d)
                {
                    uint16_t rshift = b_value >> a_value;
                    uint16_t ushift = (((uint32_t)b_value << 16) >> ((uint32_t)a_value)) & 0xffff;

                    set_location(b_location, rshift);
                    set_location(ex_location, ushift);
                }

                else if(o == 0x0e)
                {
                    int16_t signed_b = b_value;

                    // The reason for int64_t here is that shifting into the top bit is UB
                    // C++20 defines signed integers as being 2s complement, but I do not know if they've stripped this
                    // Particular piece of UB out of the standard
                    int64_t signed_extended_b = signed_b;

                    uint16_t signed_shift = signed_b >> a_value;
                    uint16_t overflow = (uint64_t)((signed_extended_b << 16) >> a_value) & 0xffff;

                    set_location(b_location, signed_shift);
                    set_location(ex_location, overflow);
                }

                else if(o == 0x0f)
                {
                    uint16_t left_shifted = b_value << a_value;
                    uint16_t overflow = (((uint32_t)b_value << (uint32_t)a_value) >> 16) & 0xffff;

                    set_location(b_location, left_shifted);
                    set_location(ex_location, overflow);
                }

                else if(o == 0x10)
                {
                    skipping = !((b_value & a_value) != 0);
                }

                else if(o == 0x11)
                {
                    skipping = !((b_value & a_value) == 0);
                }

                else if(o == 0x12)
                {
                    skipping = !(b_value == a_value);
                }

                else if(o == 0x13)
                {
                    skipping = !(b_value != a_value);
                }

                else if(o == 0x14)
                {
                    skipping = !(b_value > a_value);
                }

                else if(o == 0x15)
                {
                    skipping = !((int16_t)b_value > (int16_t)a_value);
                }

                else if(o == 0x16)
                {
                    skipping = !(b_value < a_value);
                }

                else if(o == 0x17)
                {
                    skipping = !((int16_t)b_value < (int16_t)a_value);
                }

                else if(o == 0x1a)
                {
                    auto [o1, v1] = overflow_add(b_value, a_value);
                    auto [o2, v2] = overflow_add(v1, regs[EX_REG]);

                    uint16_t overflow = o1 || o2;

                    set_location(b_location, v2);
                    set_location(ex_location, overflow);
                }

                // https://www.reddit.com/r/dcpu16/comments/t4uuq/where_is_the_17_spec_with_the_sbx_fix/
                // https://www.reddit.com/r/dcpu16/comments/t4uuq/where_is_the_17_spec_with_the_sbx_fix/c4jrgkf/
                else if(o == 0x1b)
                {
                    int32_t unsigned_a = a_value;
                    int32_t unsigned_b = b_value;

                    int32_t intermediate = unsigned_b - unsigned_a;

                    int32_t signed_ex = (int16_t)fetch_location(ex_location);

                    int32_t result = intermediate + signed_ex;

                    bool underflow = result < 0;

                    uint16_t fin = a_value + b_value + fetch_location(ex_location);

                    set_location(b_location, fin);

                    if(underflow)
                        set_location(ex_location, 0xffff);
                    else
                        set_location(ex_location, 0);
                }

                ///extension for multiprocessor, sends a value on a channel
                else if(o == 0x1c)
                {
                    ///HCF
                    /*if(fabric_opt && std::get<1>(fabric_opt->channels[a_value % NUM_CHANNELS]))
                    {
                        return true;
                    }*/

                    /*if(!fabric_opt)
                        return true;

                    fabric_slot& slot = fabric_opt->channels[a_value % NUM_CHANNELS];

                    uint16_t next = slot.next_written_id++;*/

                    ///a value is channel id, b value is value
                    //presented_value = {true, a_value, b_value};

                    presented_value = presented_info();
                    presented_value.has_value = true;
                    presented_value.target = a_value;
                    presented_value.value = b_value;
                }

                else if(o == 0x1d)
                {
                    ///HCF
                    /*if(fabric_opt && std::get<0>(fabric_opt->channels[a_value % NUM_CHANNELS]))
                    {
                        return true;
                    }*/

                    /*if(!fabric_opt)
                        return true;

                    fabric_slot& slot = fabric_opt->channels[a_value % NUM_CHANNELS];

                    uint16_t next = slot.next_read_id++;*/

                    waiting_location = waiting_info();
                    waiting_location.has_value = true;
                    waiting_location.target = a_value;
                    waiting_location.loc = b_location;
                }

                else if(o == 0x1e)
                {
                    set_location(b_location, a_value);

                    regs[I_REG]++;
                    regs[J_REG]++;
                }

                else if(o == 0x1f)
                {
                    set_location(b_location, a_value);

                    regs[I_REG]--;
                    regs[J_REG]--;
                }

                else
                {
                    return true;
                }

                return false;
            }

            else if(type == instr_type::B)
            {
                auto [o, a] = decompose_type_b(instr);

                auto exec_a_opt = exec_value_reference(*this, instruction_location, a, arg_pos::A);

                if(!exec_a_opt.has_value())
                    return true;

                location a_location = exec_a_opt.value();

                uint16_t a_value = fetch_location(exec_a_opt.value());

                if(o == 0x01)
                {
                    uint16_t stack_address = regs[SP_REG] - 1;

                    regs[SP_REG] = stack_address;

                    uint16_t naddr = instruction_location + (uint16_t)get_instruction_length(instr);

                    set_location(location::memory{stack_address}, naddr);
                    set_location(location::reg{PC_REG}, a_value);
                }

                // INT
                else if(o == 0x08)
                {
                    interrupt_type type;
                    type.is_software = 1;
                    type.message = a_value;

                    if(add_interrupt(type))
                        return true;
                }

                // IAG
                else if(o == 0x09)
                {
                    uint16_t IA_val = regs[IA_REG];

                    set_location(a_location, IA_val);
                }

                // IAS
                else if(o == 0x0a)
                {
                    set_location(location::reg{IA_REG}, a_value);

                    // https://raw.githubusercontent.com/jonpovey/das/irq-spec/dcpu-16.txt
                    // Whenever IA == 0, queue is empty
                    // This is the only way IA can be set to 0
                    if(a_value == 0)
                    {
                        interrupts.clear();
                    }
                }

                // RFI
                else if(o == 0x0b)
                {
                    interrupt_dequeueing_enabled = 1;

                    uint16_t reg_A_val = mem[regs[SP_REG]++];
                    uint16_t reg_PC_val = mem[regs[SP_REG]++];

                    regs[A_REG] = reg_A_val;
                    regs[PC_REG] = reg_PC_val;
                }

                // IAQ
                else if(o == 0x0c)
                {
                    uint16_t last_val = interrupt_dequeueing_enabled;

                    if(a_value != 0)
                    {
                        interrupt_dequeueing_enabled = 0;
                    }
                    else
                    {
                        interrupt_dequeueing_enabled = 1;
                    }

                    set_location(a_location, last_val);
                }

                // HWN
                else if(o == 0x10)
                {
                    if(hardware_opt != nullptr)
                        set_location(a_location, hardware_opt->size());
                    else
                        set_location(a_location, 0);
                }

                // HWQ
                else if(o == 0x11)
                {
                    uint32_t HID = 0;
                    uint32_t MID = 0;
                    uint16_t VERSION = 0;

                    if(hardware_opt)
                    {
                        if(a_value < hardware_opt->size())
                        {
                            HID = (*hardware_opt)[a_value]->hardware_id;
                            MID = (*hardware_opt)[a_value]->manufacturer_id;
                            VERSION = (*hardware_opt)[a_value]->hardware_version;
                        }
                    }

                    uint16_t lid = HID >> 16;
                    uint16_t vid = HID & 0xffff;

                    uint16_t lmid = MID >> 16;
                    uint16_t vmid = MID & 0xffff;

                    set_location(location::reg{A_REG}, vid);
                    set_location(location::reg{B_REG}, lid);

                    set_location(location::reg{C_REG}, VERSION);

                    set_location(location::reg{X_REG}, vmid);
                    set_location(location::reg{Y_REG}, lmid);
                }

                // HWI
                else if(o == 0x12)
                {
                    if(hardware_opt)
                    {
                        if(a_value < hardware_opt->size())
                        {
                            (*hardware_opt)[a_value]->interrupt(world_component, *this);
                        }
                    }
                }

                // IFW
                else if(o == 0x1a)
                {
                    if(fabric_opt)
                    {
                        fabric_slot& slot = fabric_opt->channels[a_value % NUM_CHANNELS];

                        skipping = !(slot.last_written_id != slot.next_written_id);
                    }
                    else
                    {
                        skipping = true;
                    }

                    //skipping = !pending_writes[a_value];
                }

                else if(o == 0x1b)
                {
                    if(fabric_opt)
                    {
                        fabric_slot& slot = fabric_opt->channels[a_value % NUM_CHANNELS];

                        skipping = !(slot.last_read_id != slot.next_read_id);

                        //skipping = !std::get<1>(fabric_opt->channels[a_value % NUM_CHANNELS]);
                    }
                    else
                    {
                        skipping = true;
                    }

                    //skipping = !pending_reads[a_value];
                }

                else
                {
                    return true;
                }
            }

            else if(type == instr_type::C)
            {
                auto o = decompose_type_c(instr);

                // BRK
                if(o == 0)
                    return true;

                return true;
            }

            // EXECUTE INTERRUPTS
            if(interrupt_dequeueing_enabled && fetch_location(location::reg{IA_REG}) != 0 && interrupts.size() > 0)
            {
                interrupt_dequeueing_enabled = 0;

                interrupt_type next = interrupts.pop_front();

                push_stack_value(regs[PC_REG]);
                push_stack_value(regs[A_REG]);

                regs[PC_REG] = regs[IA_REG];
                regs[A_REG] = next.message;
            }

            if(hardware_opt)
            {
                for(int i=0; i < (int)hardware_opt->size(); i++)
                {
                    (*hardware_opt)[i]->step(world_component, *this);
                }
            }

            return false;
        }

        void push_stack_value(uint16_t val)
        {
            regs[SP_REG]--;
            mem[regs[SP_REG]] = val;
        }

        uint16_t pop_stack_value()
        {
            uint16_t last = mem[regs[SP_REG]];
            regs[SP_REG]++;

            return last;
        }

        constexpr
        bool cycle_step(fabric* fabric_opt = nullptr, stack_vector<hardware*, 65536>* hardware_opt = nullptr, world_base* world_component = nullptr)
        {
            bool res = false;

            if(cycle_count == next_instruction_cycle)
            {
                res = step(fabric_opt, hardware_opt, world_component);
            }
            else
            {
                if(hardware_opt)
                {
                    for(int i=0; i < (int)hardware_opt->size(); i++)
                    {
                        (*hardware_opt)[i]->step(world_component, *this);
                    }
                }
            }

            cycle_count++;
            return res;
        }
    };

    template<int N>
    constexpr
    void resolve_interprocessor_communication(stack_vector<CPU*, N>& in, fabric& f)
    {
        for(int i=0; i < (int)in.size(); i++)
        {
            CPU& c1 = *in[i];

            if(c1.presented_value.has_value)
            {
                //printf("Presenting HV\n");

                if(!c1.presented_value.assigned_id.has_value())
                {
                    fabric_slot& slot = f.channels[c1.presented_value.target % NUM_CHANNELS];

                    c1.presented_value.assigned_id = slot.next_written_id++;

                    //printf("Presenting %i\n", c1.presented_value.assigned_id.value());
                }
            }

            if(c1.waiting_location.has_value)
            {
                //printf("Waiting HV\n");

                if(!c1.waiting_location.assigned_id.has_value())
                {
                    fabric_slot& slot = f.channels[c1.waiting_location.target % NUM_CHANNELS];

                    //printf("SLOT %i for %i\n", c1.waiting_location.target, i);

                    c1.waiting_location.assigned_id = slot.next_read_id++;

                    //printf("Waiting %i\n", c1.waiting_location.assigned_id.value());
                }
            }
        }

        ///write values to fabric
        for(int i=0; i < (int)in.size(); i++)
        {
            CPU& c1 = *in[i];

            if(!c1.presented_value.has_value)
                continue;

            fabric_slot& slot = f.channels[c1.presented_value.target % NUM_CHANNELS];

            ///already a value written this tick
            if(slot.current_value.has_value())
                continue;

            ///no active readers, don't write the value
            if(slot.last_read_id == slot.next_read_id)
                continue;

            if(c1.presented_value.assigned_id.value() == slot.last_written_id)
            {
                //printf("Writing value\n");

                slot.current_value = c1.presented_value.value;
                slot.last_written_id++;

                c1.presented_value.has_value = false;
            }
        }

        for(int i=0; i < (int)in.size(); i++)
        {
            CPU& c1 = *in[i];

            if(!c1.waiting_location.has_value)
                continue;

            fabric_slot& slot = f.channels[c1.waiting_location.target % NUM_CHANNELS];

            if(!slot.current_value.has_value())
                continue;

            if(c1.waiting_location.assigned_id.value() == slot.last_read_id)
            {
                //printf("Reading value\n");

                c1.set_location(c1.waiting_location.loc, slot.current_value.value());

                slot.current_value = std::optional<uint16_t>();
                slot.last_read_id++;

                //printf("Cleared %i\n", c1.presented_value.target);

                c1.waiting_location.has_value = false;
            }
        }
    }

    template<int N>
    constexpr
    void step_all(stack_vector<CPU, N>& in, fabric& f)
    {
        for(int i=0; i < (int)in.size(); i++)
        {
            in[i].step(&f);
        }

        stack_vector<CPU*, N> CPUs;

        for(int i=0; i < (int)in.size(); i++)
        {
            CPUs.push_back(&in[i]);
        }

        resolve_interprocessor_communication(CPUs, f);
    }
}

constexpr
int get_word_extra(uint16_t arg)
{
    if(arg >= 0x10 && arg <= 0x17)
    {
        return 1;
    }

    if(arg == 0x1a)
    {
        return 1;
    }

    if(arg == 0x1e)
    {
        return 1;
    }

    if(arg == 0x1f)
    {
        return 1;
    }

    return 0;
}

constexpr
int get_instruction_length(uint16_t v)
{
    if(get_type(v) == instr_type::A)
    {
        auto [o, a, b] = decompose_type_a(v);

        return 1 + get_word_extra(a) + get_word_extra(b);
    }

    else if(get_type(v) == instr_type::B)
    {
        auto [o, a] = decompose_type_b(v);

        // can only ever be 1 + 0
        return 1 + get_word_extra(a);
    }

    else if(get_type(v) == instr_type::C)
    {
        return 1;
    }

    else
    {
        return 0;
    }
}

constexpr
std::optional<dcpu::sim::location> exec_value_reference(dcpu::sim::CPU& exec, uint16_t instruction_location, uint16_t arg, arg_pos::type pos)
{
    using namespace dcpu::sim;

    if(arg >= 0 && arg <= 0x07)
    {
        return location::reg{arg};
    }

    if(arg >= 0x08 && arg <= 0x0f)
    {
        uint16_t reg_offset = arg - 0x08;

        uint16_t reg_value = exec.fetch_location(location::reg{reg_offset});

        return location::memory{exec.fetch_location(location::memory{reg_value})};
    }

    uint16_t current_instruction = exec.index(instruction_location);

    uint16_t next_word_location = 0;

    if(pos == arg_pos::A)
    {
        next_word_location = 1;
    }

    if(pos == arg_pos::B)
    {
        if(get_instruction_length(current_instruction) == 3)
        {
            next_word_location = 2;
        }
        else
        {
            next_word_location = 1;
        }
    }

    if(arg >= 0x10 && arg <= 0x17)
    {
        uint16_t reg_offset = arg - 0x10;

        uint16_t immediate_value = exec.index(instruction_location + next_word_location);
        uint16_t reg_wrap_value = exec.fetch_location(location::reg{reg_offset});

        uint16_t fin = immediate_value + reg_wrap_value;

        return location::memory{fin};
    }

    if(arg == 0x18)
    {
        if(pos == arg_pos::A)
        {
            uint16_t reg_val = exec.fetch_location(location::reg{SP_REG}) - 1;

            exec.set_location(location::reg{SP_REG}, reg_val);

            return location::memory{reg_val};
        }

        if(pos == arg_pos::B)
        {
            uint16_t reg_val = exec.fetch_location(location::reg{SP_REG});

            exec.set_location(location::reg{SP_REG}, reg_val + 1);

            return location::memory{reg_val};
        }
    }

    if(arg == 0x19)
    {
        return location::memory{exec.fetch_location(location::reg{SP_REG})};
    }

    if(arg == 0x1a)
    {
        uint16_t immediate = exec.index(instruction_location + next_word_location);
        uint16_t reg_value = exec.fetch_location(location::reg{SP_REG});

        uint16_t added = immediate + reg_value;

        return location::memory{added};
    }

    if(arg == 0x1b)
    {
        return location::reg{SP_REG};
    }

    if(arg == 0x1c)
    {
        return location::reg{PC_REG};
    }

    if(arg == 0x1d)
    {
        return location::reg{EX_REG};
    }

    if(arg == 0x1e)
    {
        uint16_t immediate_value = exec.index(instruction_location + next_word_location);
        return location::memory{immediate_value};
    }

    if(arg == 0x1f)
    {
        uint16_t immediate_value = exec.index(instruction_location + next_word_location);

        return location::constant{immediate_value};
    }

    if(arg >= 0x20 && arg <= 0x3f)
    {
        if(arg == 0x20)
        {
            return location::constant{0xffff};
        }
        else
        {
            return location::constant{(uint16_t)(arg - 0x21)};
        }
    }

    return std::nullopt;
}
}

#endif // BASE_SIM_HPP_INCLUDED
