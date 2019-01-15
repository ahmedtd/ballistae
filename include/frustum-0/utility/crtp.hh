
#ifndef FRUSTUM_0_CRTP_HH
#define FRUSTUM_0_CRTP_HH

namespace frustum
{

namespace crtp
{

template<class D, class B>
D& cast(B& p)
{
    return static_cast<D&>(p);
}

template<class D, class B>
D const& cast(B const& p)
{
    return static_cast<D const&>(p);
}

template<class D, class B>
D volatile& cast(B volatile& p)
{
    return static_cast<D volatile&>(p);
}

template<class D, class B>
D const volatile& cast(B const volatile& p)
{
    return static_cast<D const volatile&>(p);
}

}

}

#endif
