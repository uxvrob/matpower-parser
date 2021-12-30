#pragma once

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <map>

using ColumnHeaders = std::vector<std::string>;
using Row = std::vector<double>;
using Mat = std::vector<Row>;

namespace matpower{

    struct mpc_matrix {
        std::string name;
        Mat matrix;
    };

    struct mpc_data {
        mpc_data(std::vector<std::string> h) : headers(h) {}
        ColumnHeaders headers;
        Mat data;
    };

    struct mpc_version {
        std::string name;
        std::string version;
    };

    struct mpc_baseMVA {
        std::string name;
        double baseMVA;
    };

    struct mpc_bus : mpc_data {
        mpc_bus() : mpc_data({"bus_i", "type", "Pd", "Qd", "Gs", "Bs", "area", "Vm", "Va", "baseKV", "zone", "Vmax", "Vmin"}) {}
    };

    struct mpc_gen : mpc_data {
        mpc_gen() : mpc_data({"bus", "Pg", "Qg", "Qmax", "Qmin", "Vg", "mBase", "status", "Pmax", "Pmin"}) {}
    };

    struct mpc_gencost : mpc_data {
        mpc_gencost() : mpc_data({"%", "2","startup" "shutdown","n", "c(n-1)", "c0"}) {}
    };

    struct mpc_branch : mpc_data {
        mpc_branch() : mpc_data({"fbus", "tbus", "r", "x", "b", "rateA", "rateB", "rateC", "ratio", "angle", "status", "angmin", "angmax"}) {}
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
    matpower::mpc_version,
    (std::string, name)
    (std::string, version)
)

BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_baseMVA,
    (std::string, name)
    (double, baseMVA)
)

BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_bus,
    (ColumnHeaders, headers)
    (Mat, data)
)

BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_gen,
    (ColumnHeaders, headers)
    (Mat, data)
)

BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_gencost,
    (ColumnHeaders, headers)
    (Mat, data)
)

BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_branch,
    (ColumnHeaders, headers)
    (Mat, data)
)

class MatpowerParser
{


public:
	std::vector<matpower::mpc_matrix> mpc_data;
    matpower::mpc_version mpc_version;
    matpower::mpc_baseMVA mpc_baseMVA;
    matpower::mpc_bus mpc_bus;
    matpower::mpc_gen mpc_gen;
    matpower::mpc_gencost mpc_gencost;
    matpower::mpc_branch mpc_branch;

    enum MpcValues { eVersion,
                            eBaseMVA,
                            eBus,
                            eGen,
                            eGenCost,
                            eBranch};

    std::map<std::string, MpcValues> s_mapMpcValues;

	MatpowerParser(std::string);
	void printData();

private:
	std::string case_storage;

    void init();
	bool open(std::string);
	bool parse(std::string);
    bool parse_mpc_data(std::vector<matpower::mpc_matrix>);
};