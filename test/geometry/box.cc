#include <iostream>
#include <string>

#include <libballistae/geometry/box.hh>

using namespace ballistae;

int main(int argc, char **argv)
{
    box test_box({span<double>{-1.0, 1.0}, span<double>{-1.0, 1.0}, span<double>{-1.0, 1.0}});

    ray_segment<double, 3> query{{{0, 0, 2}, {0.4, 0, -0.8}}, {0, std::numeric_limits<double>::infinity()}};
    auto result = test_box.ray_into(query);

    std::cout << result.p(0) << " " << result.p(1) << " " << result.p(2) << std::endl;
    std::cout << result.n(0) << " " << result.n(1) << " " << result.n(2) << std::endl;
}
