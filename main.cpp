#include "base_sim.hpp"
#include <dcpu16-asm/base_asm.hpp>
#include <stdio.h>

#ifdef STATIC_CHECK_UB_DECODER
constexpr int ub_validate_decoder()
{
    auto [binary_opt, err] = assemble("SET X, 10");

    CPU exec;
    exec.load(binary_opt.value().mem, 0);

    int valid_states = 0;

    for(int i=0; i < 65536; i++)
    {
        valid_states += exec_value_reference(exec, 0, i, arg_pos::A).has_value();
        valid_states += exec_value_reference(exec, 0, i, arg_pos::B).has_value();
    }

    return valid_states;
}

constexpr uint64_t ub_validate_cycle_time()
{
    uint64_t v = 0;

    for(int i=0; i < 65536; i++)
    {
        v += get_cycle_time_instr(i);
    }

    return v;
}

#endif // STATIC_CHECK_UB_DECODER

constexpr uint16_t cycle_test()
{
    auto [binary_opt, err] = assemble("ADD X, 10");

    CPU exec;

    exec.load(binary_opt.value().mem, 0);
    exec.step();

    return exec.next_instruction_cycle;
}

constexpr uint16_t cycle_test2()
{
    auto [binary_opt, err] = assemble("ADD X, 9999");

    CPU exec;

    exec.load(binary_opt.value().mem, 0);
    exec.step();

    return exec.next_instruction_cycle;
}

constexpr uint16_t test_helper()
{
    auto [binary_opt, err] = assemble("SET X, 10");

    CPU exec;

    exec.load(binary_opt.value().mem, 0);
    exec.step();

    return exec.regs[X_REG];
}

constexpr uint16_t cpu_func(uint16_t X, uint16_t i, uint16_t icode)
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

constexpr
uint16_t multiprocess_1()
{
    auto [binary_opt, err] = assemble("SET X, 10\nSND X, 1");
    auto [binary_opt2, err2] = assemble("RCV Y, 0");

    stack_vector<CPU, 4> cpus;
    CPU& c1 = cpus.emplace_back();
    CPU& c2 = cpus.emplace_back();

    c1.hwid = 0;
    c2.hwid = 1;

    c1.load(binary_opt.value().mem, 0);
    c2.load(binary_opt2.value().mem, 0);

    step_all(cpus);
    step_all(cpus);

    return c2.fetch_location(location::reg{Y_REG});
}

void multiprocessor_tests()
{
    uint16_t val = multiprocess_1();

    printf("%i val\n", val);

    static_assert(multiprocess_1() == 10);
}

template<long long unsigned int N>
constexpr
std::array<uint16_t, N> cpu_apply(int X, const std::array<uint16_t, N>& csts, uint16_t icode)
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
std::array<uint16_t, N> cpu_apply(const std::array<uint16_t, N>& csts, uint16_t X, uint16_t icode)
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
        constexpr auto compare_values = constexpr_apply(test_values, [](uint16_t in) -> uint16_t {return in + 1;});

        constexpr auto applied = cpu_apply(1, test_values, 0x02);

        static_assert(array_eq(applied, compare_values));
    }

    {
        constexpr auto compare_values = constexpr_apply(test_values, [](uint16_t in) -> uint16_t {return 1 - in;});

        constexpr auto applied = cpu_apply(1, test_values, 0x03);

        static_assert(array_eq(applied, compare_values));
    }

    {
        constexpr auto compare_values = constexpr_apply(test_values, [](uint16_t in) -> uint16_t {return in - 1;});

        constexpr auto applied = cpu_apply(test_values, 1, 0x03);

        static_assert(array_eq(applied, compare_values));
    }

    {
        constexpr auto compare_values = constexpr_apply(test_values, [](uint16_t in) -> uint16_t {return in * 7;});

        constexpr auto applied = cpu_apply(test_values, 7, 0x04);

        static_assert(array_eq(applied, compare_values));
    }

    {
        // Something really weird going on with this lambda, should only need to be uint16_t but overflows
        // This is only for constructing the result array, so it should be fine
        auto lambda = [](uint32_t in) -> uint16_t {return in * 65534;};

        constexpr auto compare_values = constexpr_apply(test_values, lambda);

        constexpr auto applied = cpu_apply(test_values, 65534, 0x04);

        static_assert(array_eq(applied, compare_values));
    }

    // not much point testing this, its the upper 16 bits which are apparently different
    {
        auto signed_mul = [](int64_t val) -> uint16_t {return val * -5;};

        constexpr auto compare_values = constexpr_apply(test_values, signed_mul);

        constexpr auto applied = cpu_apply(test_values, -5, 0x05);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto unsigned_div = [](uint16_t val) -> uint16_t {return val / 53;};

        constexpr auto compare_values = constexpr_apply(test_values, unsigned_div);

        constexpr auto applied = cpu_apply(test_values, 53, 0x06);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto signed_div = [](int16_t val) -> uint16_t {return val / -153;};

        constexpr auto compare_values = constexpr_apply(test_values, signed_div);

        constexpr auto applied = cpu_apply(test_values, -153, 0x07);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto unsigned_mod = [](uint16_t val) -> uint16_t {return val % 1723;};

        constexpr auto compare_values = constexpr_apply(test_values, unsigned_mod);

        constexpr auto applied = cpu_apply(test_values, 1723, 0x08);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto unsigned_mod = [](uint16_t val) -> uint16_t {if(val == 0) {return 0;} return 1723 % val;};

        constexpr auto compare_values = constexpr_apply(test_values, unsigned_mod);

        constexpr auto applied = cpu_apply(1723, test_values, 0x08);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto signed_mod = [](int16_t val) -> uint16_t {if(val == 0) {return 0;} return -1723 % val;};

        constexpr auto compare_values = constexpr_apply(test_values, signed_mod);

        constexpr auto applied = cpu_apply(-1723, test_values, 0x09);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto sand = [](uint16_t val) -> uint16_t {return val & 1723;};

        constexpr auto compare_values = constexpr_apply(test_values, sand);

        constexpr auto applied = cpu_apply(test_values, 1723, 0x0a);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto sand = [](uint16_t val) -> uint16_t {return val | 1723;};

        constexpr auto compare_values = constexpr_apply(test_values, sand);

        constexpr auto applied = cpu_apply(test_values, 1723, 0x0b);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto sand = [](uint16_t val) -> uint16_t {return val ^ 1723;};

        constexpr auto compare_values = constexpr_apply(test_values, sand);

        constexpr auto applied = cpu_apply(test_values, 1723, 0x0c);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto sand = [](uint16_t val) -> uint16_t {return val >> 12;};

        constexpr auto compare_values = constexpr_apply(test_values, sand);

        constexpr auto applied = cpu_apply(test_values, 12, 0x0d);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto sand = [](int16_t val) -> uint16_t {return val >> 12;};

        constexpr auto compare_values = constexpr_apply(test_values, sand);

        constexpr auto applied = cpu_apply(test_values, 12, 0x0e);

        static_assert(array_eq(applied, compare_values));
    }

    {
        auto sand = [](uint16_t val) -> uint16_t {return val << 12;};

        constexpr auto compare_values = constexpr_apply(test_values, sand);

        constexpr auto applied = cpu_apply(test_values, 12, 0x0f);

        static_assert(array_eq(applied, compare_values));
    }

    {
        constexpr uint16_t m1 = 0xffff;
        constexpr int16_t m2 = m1;

        static_assert(m2 == -1);
    }
}

#define RTASSERT(x) {if((x) == false){printf("Failed %i %s\n", i, #x); assert(#x && false);}}

void runtime_tests()
{
    for(int i=0; i < 0x1f; i++)
    {
        if(i != 0)
        {
            RTASSERT(get_type(construct_type_a(i, 0, 0)) == instr_type::A);
            RTASSERT(get_type(construct_type_b(i, 0)) == instr_type::B);
        }

        RTASSERT(get_type(construct_type_c(i)) == instr_type::C);
    }

    for(int idx=0; idx < 65536; idx++)
    {
        uint16_t i = idx;
        int16_t si = i;

        RTASSERT(cpu_func(1, i, 0x01) == (uint16_t)(i));
        RTASSERT(cpu_func(1, i, 0x02) == (uint16_t)(i + 1));
        RTASSERT(cpu_func(i, 1, 0x03) == (uint16_t)(i - 1));
        RTASSERT(cpu_func(i, 7, 0x04) == (uint16_t)(i * 7));
        RTASSERT(cpu_func(i, 65534, 0x04) == (uint16_t)(i * 65534));
        RTASSERT(cpu_func(i, -5, 0x05) == (uint16_t)(si * -5));
        RTASSERT(cpu_func(i, 14, 0x06) == (uint16_t)(i / 14));
        RTASSERT(cpu_func(i, -14, 0x07) == (uint16_t)(si / -14));
        RTASSERT(cpu_func(i, -14, 0x07) == (uint16_t)(si / -14));
        RTASSERT(cpu_func(i, 1723, 0x08) == (uint16_t)(i % 1723));
        RTASSERT(cpu_func(i, -1723, 0x09) == (uint16_t)(si % -1723));
        RTASSERT(cpu_func(i, 1723, 0x0a) == (uint16_t)(i & 1723));
        RTASSERT(cpu_func(i, 1723, 0x0b) == (uint16_t)(i | 1723));
        RTASSERT(cpu_func(i, 1723, 0x0c) == (uint16_t)(i ^ 1723));
        RTASSERT(cpu_func(i, 12, 0x0d) == (uint16_t)(i >> 12));
        RTASSERT(cpu_func(i, 12, 0x0e) == (uint16_t)(si >> 12));
        RTASSERT(cpu_func(i, 12, 0x0f) == (uint16_t)(i << 12));
    }

    #ifdef LONG
    // All passed
    for(int jidx = 0; jidx < 65536; jidx++)
    {
        uint16_t j = jidx;
        int16_t sj = jidx;

        for(int idx=0; idx < 65536; idx++)
        {
            uint16_t i = idx;
            int16_t si = i;

            RTASSERT(cpu_func(j, i, 0x01) == (uint16_t)(i));
            RTASSERT(cpu_func(j, i, 0x02) == (uint16_t)(i + j));
            RTASSERT(cpu_func(i, j, 0x03) == (uint16_t)(i - j));
            RTASSERT(cpu_func(i, j, 0x04) == (uint16_t)(i * j));
            RTASSERT(cpu_func(i, j, 0x04) == (uint16_t)(i * j));
            RTASSERT(cpu_func(i, j, 0x05) == (uint16_t)(si * j));
            RTASSERT(cpu_func(i, j, 0x06) == (uint16_t)((j == 0) ? 0 : i / j));
            RTASSERT(cpu_func(i, j, 0x07) == (uint16_t)((sj == 0) ? 0 : si / sj));
            RTASSERT(cpu_func(i, j, 0x07) == (uint16_t)(sj == 0 ? 0 : si / sj));
            RTASSERT(cpu_func(i, j, 0x08) == (uint16_t)(j == 0 ? 0 : i % j));
            RTASSERT(cpu_func(i, j, 0x09) == (uint16_t)(sj == 0 ? 0 : si % sj));
            RTASSERT(cpu_func(i, j, 0x0a) == (uint16_t)(i & j));
            RTASSERT(cpu_func(i, j, 0x0b) == (uint16_t)(i | j));
            RTASSERT(cpu_func(i, j, 0x0c) == (uint16_t)(i ^ j));
            RTASSERT(cpu_func(i, j, 0x0d) == (uint16_t)(i >> j));
            RTASSERT(cpu_func(i, j, 0x0e) == (uint16_t)(si >> j));
            RTASSERT(cpu_func(i, j, 0x0f) == (uint16_t)(i << j));
        }
    }
    #endif // LONG

    #ifdef SELF_TEST
    #pragma omp parallel for
    for(int jidx = 0; jidx < 65536; jidx++)
    {
        for(int i = 0; i < 65536; i++)
        {
            RTASSERT(cpu_func(jidx, i, 0x02) == (uint16_t)(jidx + i));
        }
    }

    printf("Validated +\n");

    #pragma omp parallel for
    for(int jidx = 0; jidx < 65536; jidx++)
    {
        for(int i = 0; i < 65536; i++)
        {
            RTASSERT(cpu_func(jidx, i, 0x03) == (uint16_t)(jidx - i));
        }
    }

    printf("Validated -\n");

    #pragma omp parallel for
    for(int jidx = 0; jidx < 65536; jidx++)
    {
        for(int i = 0; i < 65536; i++)
        {
            RTASSERT(cpu_func(jidx, i, 0x04) == (uint16_t)(jidx * i));
        }
    }

    printf("Validated *\n");

    #pragma omp parallel for
    for(int jidx = 0; jidx < 65536; jidx++)
    {
        for(int i = 0; i < 65536; i++)
        {
            RTASSERT(cpu_func(jidx, i, 0x06) == (uint16_t)(i == 0 ? 0 : jidx / i));
        }
    }

    printf("Validated /\n");
    #endif // SELF_TEST
}

constexpr
void constexpr_tests()
{
    constexpr uint16_t val = test_helper();
    static_assert(val == 10);

    #ifdef STATIC_CHECK_UB_DECODER
    static_assert(ub_validate_decoder() > 0);
    static_assert(ub_validate_cycle_time() > 0);
    #endif // STATIC_CHECK_UB_DECODER

    static_assert(cycle_test() == 2);
    static_assert(cycle_test2() == 3);
}

int main()
{
    //runtime_tests();
    //constexpr_tests();
    multiprocessor_tests();

    return 0;
}
