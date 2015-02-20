#ifndef LIBBALLISTAE_CONTACT_HH
#define LIBBALLISTAE_CONTACT_HH

#include <armadillo>

namespace ballistae
{

struct contact
{
    double t;
    arma::vec3 point;
    arma::vec3 normal;

    contact() = default;
    contact(
        const double &t_in,
        const arma::vec3 &point_in,
        const arma::vec3 &normal_in
    );

    static contact infinity() __attribute__((const));
};


bool operator<(const contact &a, const contact &b);
bool operator==(const contact &a, const contact &b);
bool operator!=(const contact &a, const contact &b);

}

#endif
