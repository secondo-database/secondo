#ifndef SECONDO_TERNARY_FUNCTION_HPP
#define SECONDO_TERNARY_FUNCTION_HPP

template
<
    typename ARG1,
    typename ARG2,
    typename ARG3,
    typename RESULT
>
struct ternary_function
{
    typedef ARG1 first_argument_type;
    typedef ARG2 second_argument_type;
    typedef ARG3 third_argument_type;
    typedef RESULT result_type;
};

#endif // SECONDO_TERNARY_FUNCTION_HPP
