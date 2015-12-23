#ifndef LIBBALLISTAE_OPTIONS_HH
#define LIBBALLISTAE_OPTIONS_HH

#include <string>

#include <boost/program_options.hpp>

#include <libballistae/scene.hh>

namespace ballistae
{

options take_options(int argc, char **argv)
{
    namespace po = boost::program_options;

    options o;

    po::options_description spec("All options");
    spec.add_options()
        ("help", "Print this help message.")
        ("gridsize", po::value<size_t>(&(o.gridsize))->default_value(5), "Size of the supersampling grid.")

        ("img-rows", po::value<size_t>(&(o.img_rows))->default_value(864), "Rows in the output image.")
        ("img-cols", po::value<size_t>(&(o.img_cols))->default_value(1296), "Columns in the output image.")

        ("lambda-min", po::value<double>(&(o.lambda_min))->default_value(390), "Minimum wavelength.")
        ("lambda-max", po::value<double>(&(o.lambda_max))->default_value(835), "Maximum wavelength.")

        ("maxdepth", po::value<size_t>(&(o.maxdepth))->default_value(8), "Maximum ray depth in the scene.")

        ("asset_dir", po::value<std::string>(&(o.asset_dir))->default_value("./"), "Path where we should look for assets")
        ("output_file", po::value<std::string>(&(o.output_file))->default_value("output.pfm"), "Name of the output image.")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, spec), vm);
    po::notify(vm);

    return o;
}

}

#endif
