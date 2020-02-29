#include "base_sim.hpp"
#include <dcpu16-asm/base_asm.hpp>

constexpr
void constexpr_tests()
{
    auto [binary_opt, err] = assemble("SET X, 10");

    CPU exec;
    exec.load(binary_opt.value().mem, 0);
}

int main()
{
    constexpr_tests();

    return 0;
}
