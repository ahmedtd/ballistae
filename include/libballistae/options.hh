#ifndef LIBBALLISTAE_OPTIONS_HH
#define LIBBALLISTAE_OPTIONS_HH

#include <climits>
#include <cstdio>
#include <cstdlib>

#include <getopt.h>
#include <sysexits.h>

#include <libballistae/scene.hh>

namespace ballistae
{

size_t parse_size_t(const char *const text)
{
    char *end = NULL;
    size_t val = std::strtoull(text, &end, 10);
    if(end == text)
    {
        std::exit(1);
    }
    else if((ULLONG_MAX == val) && ERANGE == errno)
    {
        std::exit(1);
    }

    return val;
}

double parse_double(const char *const text)
{
    char *end = NULL;
    double val = std::strtod(text, &end);
    if(end == text)
    {
        std::exit(1);
    }
    else if(ERANGE == errno)
    {
        std::exit(1);
    }

    return val;
}

options take_options(int argc, char **argv)
{
    options o;

    o.gridsize = 5;
    o.img_rows = 512;
    o.img_cols = 512;
    o.lambda_min = 390;
    o.lambda_max = 835;
    o.maxdepth = 8;
    o.asset_dir = "./";
    o.output_file = "output.pfm";

    static option long_options [] = {
        {"help", no_argument, 0, 0},
        {"gridsize", required_argument, 0, 0},
        {"img-rows", required_argument, 0, 0},
        {"img-cols", required_argument, 0, 0},
        {"lambda-min", required_argument, 0, 0},
        {"lambda-max", required_argument, 0, 0},
        {"maxdepth",  required_argument, 0, 0},
        {"asset-dir", required_argument, 0, 0},
        {"output-file", required_argument, 0, 0},
        {0, 0, 0, 0},
    };

    while(true)
    {
        // Parse an option using getopt_long.  The empty string literal
        // indicates that we don't accept any short options, which significantly
        // simplifies our code here.
        int option_index;
        int c = getopt_long(
            argc,
            argv,
            "",
            long_options,
            &option_index
        );

        // Check if argument processing has finished.
        if(-1 == c)
            break;

        std::string optname = std::string(long_options[option_index].name);

        if("help" == optname)
        {
            std::printf("TODO: Print help\n");
            std::exit(EX_OK);
        }

        if("gridsize" == optname)
        {
            o.gridsize = parse_size_t(optarg);
        }
        else if("img-rows" == optname)
        {
            o.img_rows = parse_size_t(optarg);
        }
        else if("img-cols" == optname)
        {
            o.img_cols = parse_size_t(optarg);
        }
        else if("lambda-min" == optname)
        {
            o.lambda_min = parse_double(optarg);
        }
        else if("lambda-max" == optname)
        {
            o.lambda_max = parse_double(optarg);
        }
        else if("maxdepth" == optname)
        {
            o.maxdepth = parse_size_t(optarg);
        }
        else if("asset-dir" == optname)
        {
            o.asset_dir = std::string(optarg);
        }
        else if("output-file" == optname)
        {
            o.output_file = std::string(optarg);
        }
    }

    return o;
}

}

#endif
