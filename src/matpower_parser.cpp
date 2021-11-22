#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <map>

using It  = boost::spirit::istream_iterator;
using ColumnHeaders = std::vector<std::string>;
using Row = std::vector<double>;
using Mat = std::vector<Row>;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace spirit = boost::spirit;
namespace fusion = boost::fusion;

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


/*
BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_data,
    (matpower::mpc_baseMVA, baseMVA)
)
*/

namespace matpower {

    
    template <typename Iterator>
    struct skip_grammar : qi::grammar<Iterator>
    {
        skip_grammar() : skip_grammar::base_type(skipper_)
        {
            using ascii::char_;
            using ascii::space;
            using qi::eol;
            using qi::eoi;

            // Skip single line comments -- assume no block statements
            single_line_comment_ = "%" >> *(char_ - eol) >> (eol|eoi);
            skipper_ = space | single_line_comment_;

        }
        qi::rule<Iterator> single_line_comment_, skipper_;
        
    };
    

    template <typename Iterator, typename Skipper = skip_grammar<Iterator>>
    struct matpower_grammar : qi::grammar<Iterator, std::vector<mpc_matrix>(), Skipper>
    {
        matpower_grammar() : matpower_grammar::base_type(mpc_data_)
        {

            using qi::lit;
            using qi::double_;
            using qi::lexeme;
            using spirit::repeat;
            using spirit::inf;
            using ascii::char_;
            using ascii::space;

            mpc_var_name_ %= lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];
            matrix_ %=  lexeme[double_ % *space % double_ >>';'];

            mpc_start_ %=                 
                lit("mpc")
                >> '.'
                >> mpc_var_name_
                >> '='
                ;
/*
            mpc_baseMVA_ %=
                mpc_start_
                >> double_
                ;

*/
            mpc_matrix_ %= 
                mpc_start_
                >> '['
                >> *matrix_
                >> ']'
                >> ';'
                ;

            mpc_data_ %= repeat[mpc_matrix_];
            //mpc_data_ %= repeat[mpc_baseMVA_ / mpc_matrix_];

        }
        qi::rule<Iterator, std::string(), Skipper> mpc_start_;
        qi::rule<Iterator, std::string(), Skipper> mpc_var_name_;
        qi::rule<Iterator, double, Skipper> mpc_baseMVA_;
        qi::rule<Iterator, Mat(), Skipper> matrix_;
        qi::rule<Iterator, mpc_matrix(), Skipper> mpc_matrix_;
        qi::rule<Iterator, std::vector<mpc_matrix>(), Skipper> mpc_data_;

    };

}


int main(int argc, char **argv)
{

    ////////////////////////////////////////////////////////////
    // Process Case File
    ///////////////////////////////////////////////////////////

    // Process input parameters
    char const* case_filename;
    std::string default_case_filename = "../data/matrix_only.m";

    if(argc > 1) case_filename = argv[1];
    else
    {
        case_filename = default_case_filename.c_str();
        std::cout << "Using default case file: " << case_filename << std::endl;
    }
    std::cout << "Parsing file: " << case_filename << std::endl;

    // Open case_file
    std::ifstream fin(case_filename, std::ios::in);

    if(!fin)
    {
        std::cerr << "Error: Could not open input file: "
            << case_filename << std::endl;
        return 1;
    }

    // Copy case_file into string storage & close file
    std::string case_storage;
    fin.unsetf(std::ios::skipws); // Don't skip white spaces
    std::copy(
        std::istream_iterator<char> (fin),
        std::istream_iterator<char>(),
        std::back_inserter(case_storage)
    );
    fin.close();

    /////////////////////////////////////////////////////////
    // Run Parser
    /////////////////////////////////////////////////////////

    typedef std::string::const_iterator StringIt;
    using boost::spirit::ascii::space;

    StringIt iter = case_storage.begin();
    StringIt end = case_storage.end();

    std::vector<matpower::mpc_matrix> mpc_data;
    matpower::matpower_grammar<StringIt> mpg;
    matpower::skip_grammar<StringIt> skipper;

    bool r = phrase_parse(iter, end, mpg, skipper, mpc_data);

    if (r && iter == end)
    {
        std::cout << "Parse successful!" << std::endl;
        
        for(auto& v : mpc_data)
        {
            std::cout << "Parsed variable name: '" << boost::fusion::at_c<0>(v) << "' [";

            for(auto& row : boost::fusion::at_c<1>(v))
                std::copy(row.begin(), row.end(), std::ostream_iterator<double>(std::cout<<"\n\t",", "));
            std::cout << "\n]\n";
        }
    } else {
        std::cout << "Parse failed\n";
    }
    

    if (iter!=end)
        std::cout << "Remaining input: '" << std::string(iter,end) << "'\n";


}

// mpc.bus =
//bus_i bus_type    Pd  Qd  Gs  Bs  area    Vm  Va  baseKV  zone    Vmax    Vmin

// mpc.gen = 
//  bus Pg  Qg  Qmax    Qmin    Vg  mBase   status  Pmax    Pmin

// mpc.gencost
//  model   startup shutdown    ncost   c(n-1)  ... c0


// mpc.branch
//  fbus    tbus    r   x   b   rateA   rateB   rateC   ratio   angle   status  angmin  angmax
