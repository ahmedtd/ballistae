#include <libballistae/contact.hh>

#include <armadillo>

namespace ballistae
{

contact::contact(
    const double &t_in,
    const arma::vec3 &point_in,
    const arma::vec3 &normal_in
)
    : t(t_in),
      point(point_in),
      normal(normal_in)
{
}

contact contact::infinity()
{
    return contact(
        std::numeric_limits<double>::infinity(),
        (arma::vec3()),
        (arma::vec3())
    );
}

bool operator<(const contact &a, const contact &b)
{
    return a.t < b.t;
}

bool operator==(const contact &a, const contact &b)
{
    return a.t == b.t;
}

bool operator!=(const contact &a, const contact &b)
{
    return !(a == b);
}

}
