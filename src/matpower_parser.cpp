#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include<iostream>
#include<fstream>
#include<string>

using It  = boost::spirit::istream_iterator;
using Row = std::vector<double>;
using Mat = std::vector<Row>;


namespace matpower{

    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    namespace fusion = boost::fusion;

    struct mpc_matrix{
        std::string name;
        Mat matrix;
    };


} //matpower

BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_matrix,
    (std::string, name)
    (Mat, matrix)
)

namespace matpower {

    /*
    template <typename Iterator>
    struct skipper : qi::grammar<Iterator>
    {
        skipper() : skipper::base_type(skipper_)
        {
            using ascii::char_;
            using qi::eol;
            using qi::eoi;
            using ascii::space;

            // Skip single line comments -- assume no block statements
            single_line_comment_ = '%' >> *(char_ - eol) >> (eol|eoi);
            skipper_ = space | single_line_comment_;

        }
        qi::rule<Iterator> single_line_comment_, skipper_;
        
    };
    */

    template <typename Iterator>
    struct matpower_grammar : qi::grammar<Iterator, mpc_matrix(), ascii::space_type>
    {
        matpower_grammar() : matpower_grammar::base_type(mpc_)
        {

            using qi::lit;
            using qi::double_;
            using qi::lexeme;
            using ascii::char_;
            using ascii::space;


            mpc_var_name_ %= lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];
            matrix_ %=  lexeme[double_ % *space % double_ >>';'];

            mpc_ %= 
                lit("mpc")
                >> '.'
                >> mpc_var_name_
                >> '='
                >> '['
                >> *matrix_
                >> ']'
                >> ';'
                ;

        }
        qi::rule<Iterator, std::string(), ascii::space_type> mpc_var_name_;
        qi::rule<Iterator, Mat(), ascii::space_type> matrix_;
        qi::rule<Iterator, matpower::mpc_matrix(), ascii::space_type> mpc_;

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

    matpower::mpc_matrix mpc_data;
    matpower::matpower_grammar<StringIt> mpg;

    
    bool r = phrase_parse(iter, end, mpg, space, mpc_data);

    if (r && iter == end)
    {
        std::cout << "Parse successful!" << std::endl;
        
        std::cout << "Parsed variable name: '" << boost::fusion::at_c<0>(mpc_data) << "' [";

        for(auto& row : boost::fusion::at_c<1>(mpc_data))
            std::copy(row.begin(), row.end(), std::ostream_iterator<double>(std::cout<<"\n\t",", "));
        std::cout << "\n]\n";
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
