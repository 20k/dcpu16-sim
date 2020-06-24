#ifndef EXECUTION_CONTEXT_HPP_INCLUDED
#define EXECUTION_CONTEXT_HPP_INCLUDED

#include <vector>
#include <dcpu16-asm/stack_vector.hpp>

template<typename T>
struct execution_context
{
    stack_vector<T, 64> cpus;
};

#endif // EXECUTION_CONTEXT_HPP_INCLUDED
