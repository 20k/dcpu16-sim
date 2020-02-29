#include "base_sim.hpp"
#include <dcpu16-asm/base_asm.hpp>

constexpr uint16_t test_helper()
{
    auto [binary_opt, err] = assemble("SET X, 10");

    CPU exec;
    exec.load(binary_opt.value().mem, 0);
    exec.step();

    return exec.regs[X_REG];
}

constexpr uint16_t cpu_func(int X, int i, int icode)
{
    CPU c;

    uint16_t instr = construct_type_a(icode, 0x1f, X_REG);
    uint16_t cst = i;

    c.mem[0] = instr;
    c.mem[1] = cst;
    c.regs[X_REG] = X;

    c.step();

    return c.regs[X_REG];
}

template<long long unsigned int N>
constexpr
std::array<uint16_t, N> cpu_apply(int X, const std::array<uint16_t, N>& csts, int icode)
{
    std::array<uint16_t, N> ret = {};

    for(int i=0; i < (int)N; i++)
    {
        ret[i] = cpu_func(X, csts[i], icode);
    }

    return ret;
}

template<long long unsigned int N>
constexpr
std::array<uint16_t, N> cpu_apply(const std::array<uint16_t, N>& csts, int X, int icode)
{
    std::array<uint16_t, N> ret = {};

    for(int i=0; i < (int)N; i++)
    {
        ret[i] = cpu_func(csts[i], X, icode);
    }

    return ret;
}

template<long long unsigned int N>
constexpr bool array_eq(const std::array<uint16_t, N>& i1, const std::array<uint16_t, N>& i2)
{
    for(int i=0; i < (int)N; i++)
    {
        if(i1[i] != i2[i])
            return false;
    }

    return true;
}

template<typename T>
constexpr
std::array<uint16_t, 8> constexpr_apply(const std::array<uint16_t, 8>& in, T func)
{
    std::array<uint16_t, 8> ret = {};

    for(int i=0; i < 8; i++)
    {
        ret[i] = func(in[i]);
    }

    return ret;
}

constexpr std::array<uint16_t, 8> default_test_values()
{
    return {0, 0xffff, 1, 2, 0xfffe, 0xfffd, 0x1000, 0x1234};
}

constexpr void cspr_tests()
{
    constexpr auto test_values = default_test_values();

    {
        constexpr auto compare_values = test_values;

        constexpr auto applied = cpu_apply(0, test_values, 0x1);

        static_assert(array_eq(applied, compare_values));
    }

    {
        constexpr auto compare_values = constexpr_apply(test_values, [](uint16_t in){return in + 1;});

        constexpr auto applied = cpu_apply(1, test_values, 0x02);

        static_assert(array_eq(applied, compare_values));
    }

    {
        constexpr auto compare_values = constexpr_apply(test_values, [](uint16_t in){return 1 - in;});

        constexpr auto applied = cpu_apply(1, test_values, 0x03);

        static_assert(array_eq(applied, compare_values));
    }

    {
        constexpr auto compare_values = constexpr_apply(test_values, [](uint16_t in){return in - 1;});

        constexpr auto applied = cpu_apply(test_values, 1, 0x03);

        static_assert(array_eq(applied, compare_values));
    }
}

void runtime_tests()
{
    for(int idx=0; idx < 65536; idx++)
    {
        uint16_t i = idx;

        assert(cpu_func(1, i, 0x02) == (uint16_t)(i + 1));
        assert(cpu_func(i, 1, 0x03) == (uint16_t)(i - 1));
    }
}

constexpr
void constexpr_tests()
{
    constexpr uint16_t val = test_helper();
    static_assert(val == 10);
}

int main()
{
    runtime_tests();
    constexpr_tests();
    runtime_tests();

    return 0;
}
