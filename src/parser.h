#pragma once

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

using ColumnHeaders = std::vector<std::string>;
using Row = std::vector<double>;
using Mat = std::vector<Row>;

namespace matpower{

    struct mpc_matrix {
        std::string name;
        Mat matrix;
    };

    struct mpc_baseMVA {
        std::string name;
        double baseMVA;
    };

    struct mpc_version {
        std::string name;
        std::string version;
    };

    struct mpc_gen {
        ColumnHeaders headers; //{bus Pg  Qg  Qmax    Qmin    Vg  mBase   status  Pmax    Pmin};
        Mat data;
    };

    struct mpc_gencost {
        // [ %   2   startup shutdown    n   c(n-1)  ... c0]
        ColumnHeaders headers;
        Mat data;
    };
    
    struct mpc_branch {
        ColumnHeaders headers;
        Mat data;
    };

    /***********************************************************
     * The data structure consists of 
     * 
     * mpc.version
     * mpc.baseMVA
     * 
     * %% bus data
        mpc.bus = [bus_i   type    Pd  Qd  Gs  Bs  area    Vm  Va  baseKV  zone    Vmax    Vmin]
     * 
           
     * %% generator data
        mpc.gen = [bus Pg  Qg  Qmax    Qmin    Vg  mBase   status  Pmax    Pmin]

     * %% generator cost data
        mpc.gencost = [ %   2   startup shutdown    n   c(n-1)  ... c0]

     * %% branch data
        mpc.branch = [fbus    tbus    r   x   b   rateA   rateB   rateC   ratio   angle   status  angmin  angmax]
    

    *************************************************************/


} //matpower

BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_matrix,
    (std::string, name)
    (Mat, matrix)
)


BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_baseMVA,
    (std::string, name)
    (double, baseMVA)
)

BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_version,
    (std::string, name)
    (std::string, version)
)

class MatpowerParser
{


public:
	std::vector<matpower::mpc_matrix> mpc_data;

	MatpowerParser(std::string);
	void printData();

private:
	std::string case_storage;

	bool open(std::string);
	bool parse(std::string);
};