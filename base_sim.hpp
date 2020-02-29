#ifndef BASE_SIM_HPP_INCLUDED
#define BASE_SIM_HPP_INCLUDED

#include <tuple>
#include <variant>
#include <assert.h>
#include <dcpu16-asm/stack_vector.hpp>
#include <dcpu16-asm/shared.hpp>
#include <optional>

#define MEM_SIZE 0x10000
#define REG_NUM 12

#define I_REG 6
#define J_REG 7
#define PC_REG 8
#define SP_REG 9
#define EX_REG 10
#define IA_REG 11

namespace instr_type
{
    enum type
    {
        A,
        B,
        C
    };
}

inline
constexpr
std::tuple<uint16_t, uint16_t, uint16_t> decompose_type_a(uint16_t v)
{
    uint16_t o =  v & 0b0000000000011111;
    uint16_t b = (v & 0b0000001111100000) >> 5;
    uint16_t a = (v & 0b1111110000000000) >> 10;

    return {o, a, b};
}


inline
constexpr
std::tuple<uint16_t, uint16_t> decompose_type_b(uint16_t v)
{
    uint16_t o = (v & 0b0000001111100000) >> 5;
    uint16_t a = (v & 0b1111110000000000) >> 10;

    return {o, a};
}

inline
constexpr
uint16_t decompose_type_c(uint16_t v)
{
    return (v >> 10) & 0b0000000000011111;
}

inline
constexpr
instr_type::type get_type(uint16_t v)
{
    auto [o, a, b] = decompose_type_a(v);

    if(o == 0)
    {
        return instr_type::type::B;
    }
    else
    {
        auto [bo, bb] = decompose_type_b(v);

        if(bo == 0)
        {
            return instr_type::type::C;
        }

        return instr_type::type::A;
    }
}

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

struct CPU;

std::optional<location> exec_value_reference(CPU& exec, uint16_t arg, arg_pos::type pos);

struct CPU
{
    std::array<uint16_t, MEM_SIZE> mem;
    std::array<uint16_t, REG_NUM> regs;
    int32_t cycle_count = 0;
    bool skipping = false;

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

        regs[PC_REG] = pc;
    }

    constexpr
    uint16_t index(uint16_t in)
    {
        return mem[in];
    }

    constexpr
    bool step()
    {

        return true;
    }
};


inline
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

inline
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


std::optional<location> exec_value_reference(CPU& exec, uint16_t arg, arg_pos::type pos)
{
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

    uint16_t current_instruction = exec.index(exec.fetch_location(location::reg{PC_REG}));

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

        uint16_t immediate_value = exec.index(exec.fetch_location(location::reg{PC_REG}) + next_word_location);
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
        uint16_t immediate = exec.index(exec.fetch_location(location::reg{PC_REG}) + next_word_location);
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
        uint16_t immediate_value = exec.index(exec.fetch_location(location::reg{PC_REG}) + next_word_location);
        return location::memory{immediate_value};
    }

    if(arg == 0x1f)
    {
        uint16_t immediate_value = exec.index(exec.fetch_location(location::reg{PC_REG}) + next_word_location);

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

#endif // BASE_SIM_HPP_INCLUDED
