#ifndef FRUSTUM_0_ELEMENTWISE_ARITHMETIC_HH
#define FRUSTUM_0_ELEMENTWISE_ARITHMETIC_HH

namespace frustum
{

template<class R, class Op, class Arg0, class... ArgTail>
void generic_elementwise_op(
    R &r,
    Op op,
    const Arg0 &arg0,
    const ArgTail&... argtail
)
{
    for(size_t i = 0; i < r.n_entries(); ++i)
        r[i] = op(arg0[i], argtail[i]...);
}

}

#endif
