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

constexpr
void constexpr_tests()
{
    constexpr uint16_t val = test_helper();
    static_assert(val == 10);
}

int main()
{
    constexpr_tests();

    return 0;
}
