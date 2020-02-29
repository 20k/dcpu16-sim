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

constexpr uint16_t cpu_func(int i, int icode)
{
    CPU c;

    uint16_t instr = construct_type_a(icode, 0x1f, X_REG);
    uint16_t cst = i;

    c.mem[0] = instr;
    c.mem[1] = cst;

    c.step();

    return c.regs[X_REG];
}

template<long long unsigned int N>
constexpr
std::array<uint16_t, N> cpu_apply(const std::array<uint16_t, N>& csts, int icode)
{
    std::array<uint16_t, N> ret = {};

    for(int i=0; i < (int)N; i++)
    {
        ret[i] = cpu_func(csts[i], icode);
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

constexpr void test_helper3()
{
    constexpr std::array<uint16_t, 8> test_values = {0, 0xffff, 1, 2, 0xfffe, 0xfffd, 0x1000, 0x1234};
    constexpr std::array<uint16_t, 8> compare_values = test_values;

    constexpr std::array<uint16_t, 8> applied = cpu_apply(test_values, 0x1);

    static_assert(array_eq(applied, compare_values));
}

constexpr
void constexpr_tests()
{
    constexpr uint16_t val = test_helper();
    static_assert(val == 10);
}

void runtime_tests()
{
    /*for(int i=0; i < 65536; i++)
    {
        uint16_t v1 = test_helper2(i);

        assert(i == v1);
    }*/
}

int main()
{
    constexpr_tests();
    runtime_tests();

    return 0;
}
