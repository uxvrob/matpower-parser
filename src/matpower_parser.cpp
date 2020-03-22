//https://stackoverflow.com/questions/27050956/writing-a-parser-for-a-matrix-like-input-with-boost-spirit
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

typedef std::vector<double> Row;
typedef std::vector<Row> Mat;

namespace matpower{

    namespace fusion = boost::fusion;
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    //typedef std::pair<std::string, Mat> mpc_matrix;

/*    enum bus_type
   {
       PQ=1,
       PV=2,
       REF=3,
       ISOLATED=4
   }; */
/*
    struct bus_data
    {
        int bus_id;
        int  bt;
        double Pd;
        double Qd;
        double Gs;
        double BS;
        double area;
        double Vm;
        double Va;
        double baseKV;
        double zone;
        double Vmax;
        double Vmin;

    };
*/
    struct mpc_matrix{
        std::string name;
        Mat matrix;
    };

    struct mpc_data
    {
        std::string case_name;
        std::string version;
        double baseMVA;
        std::vector<mpc_matrix> data; 

    };

} //matpower

BOOST_FUSION_ADAPT_STRUCT(
    matpower::mpc_matrix,
    (std::string, name)
    (Mat, matrix)
)

BOOST_FUSION_ADAPT_STRUCT(
	matpower::mpc_data,
    (std::string, case_name)
    (std::string, version)
    (double, baseMVA)
    (std::vector<matpower::mpc_matrix>, data)
)

/*
BOOST_FUSION_ADAPT_STRUCT(
        matpower::bus_data,
        (int, bus_id)
        (int, bt)
        (double, Pd)
        (double, Qd)
        (double, Gs)
        (double, BS)
        (double, area)
        (double, Vm)
        (double, Va)
        (double, baseKV)
        (double, zone)
        (double, Vmax)
        (double, Vmin)
)
*/
namespace matpower{

    ///////////////////////////////////////////////////////////////////////
    // Matpower Data Printer
    ///////////////////////////////////////////////////////////////////////
    int const tabsize = 4;

    void tab(int indent)
    {
        for (int i = 0; i < indent; ++i)
            std::cout << ' ';
    }
    void tabnewline(int indent)
    {
        std::cout << std::endl;
        tab(indent);
    }
    struct mpc_printer
    {
        mpc_printer(int indent=0)
            : indent(indent)
            {
            }

        void print_matrix(Mat data)
        {

        }

        void print_mpc_matrix(mpc_matrix var_data)
        {
                
        }

        void operator()(mpc_data const& mpc) const
        {
            tab(indent);
            std::cout << "case_name: " << boost::fusion::at_c<0>(mpc) << std::endl;
            tab(indent);
            std::cout << "version: " << boost::fusion::at_c<1>(mpc) << std::endl;
            tab(indent);
            std::cout << "base_mva: " << boost::fusion::at_c<2>(mpc) << std::endl;
            for(auto& node : boost::fusion::at_c<3>(mpc))
            {                
                tab(indent);
                std::cout << "variable: " << node.name << std::endl;
                tab(indent);
                std::cout << "data: " << std::endl;
                tab(indent);
                std::cout << '{' << std::endl;
                
                for (auto& row : node.matrix)
                    std::copy(row.begin(), row.end(), 
                            std::ostream_iterator<double>(std::cout<<"\n\t", ", ")
                        );
                tab(indent);
                std::cout << '}' << std::endl;

            }
            
        }
        int indent;
    };

    ///////////////////////////////////////////////////////////////////////
    // Matpower M Grammar Definition
    ///////////////////////////////////////////////////////////////////////

    template <typename Iterator>
    struct matpower_grammar 
        : qi::grammar<Iterator, mpc_data(), ascii::space_type>
    {
        matpower_grammar() 
            : matpower_grammar::base_type(mpc_)
        {
            //using qi::int_;
            
            using qi::lit;
            using qi::double_;
            using qi::string;
	        using qi::lexeme;
            using qi::repeat;
            using qi::eoi;
            using qi::eol;
            using qi::skip;
            using namespace qi::labels;
            using ascii::char_;
            using ascii::space;

            
            // Skip single line comments -- assume no block statements
            single_line_comment_ = "%" >> *(char_ - eol) >> (eol|eoi);
            skipper_ = space | single_line_comment_;

	        mpc_var_name_ = lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];
            matrix_ = lexeme['[' >> double_ % ' ' % ';' >> ']'];
            //var_char_ = lexeme['\'' >> char_ >> '\''];

            mpc_case_name_ = 
                lit("function mpc")
                >> '='
                >> mpc_var_name_
                >> ';'
                ;
            
            mpc_matrix_var_ =
                lit("mpc")
                >> '.'
                >> mpc_var_name_
                >> '='
                >> matrix_ 
                >> ';'
                ;

            mpc_ = 
                //skip(skipper_)
                mpc_case_name_                               //mpc case_name
                //>> skip(skipper_)
                >> lexeme['\'' >> char_ >> '\'' >> ';']                //mpc.version
                //>> skip(skipper_)
                >> lit("mpc.baseMVA") >> '=' >> double_ >> ';'  //mpc.baseMVA
                //>> skip(skipper_)
                >> *mpc_matrix_var_;

        }
        qi::rule<Iterator> single_line_comment_, skipper_;
	    qi::rule<Iterator, std::string(), ascii::space_type> mpc_var_name_;
        //qi::rule<Iterator, char(), ascii::space_type> var_char_;
        //qi::rule<Iterator, double(), ascii::space_type> var_double_;
        qi::rule<Iterator, Mat(), ascii::space_type> matrix_;
        qi::rule<Iterator, mpc_matrix(), ascii::space_type> mpc_matrix_var_;
        qi::rule<Iterator, std::string(), ascii::space_type> mpc_case_name_;
        qi::rule<Iterator, mpc_data(), ascii::space_type> mpc_;
    };

} //matpower


int main(int argc, char **argv)
{

    // Process input parameters
    char const* case_filename;
    std::string default_case_filename = "../data/pglib_opf_case14_ieee.m";

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

    // Create matpower grammar and data structure
    typedef std::string::const_iterator StringIt;
    typedef matpower::matpower_grammar<StringIt> MatpowerGrammar;
    MatpowerGrammar grammar;
    matpower::mpc_data mpc_data;

    // Create parser from matpower grammar
    using boost::spirit::ascii::space;
    StringIt iter = case_storage.begin();
    StringIt end = case_storage.end();
    bool r = phrase_parse(iter, end, grammar, space, mpc_data);

    
    if(r && iter == end){
        std::cout << "-------------------------\n";
        std::cout << "Parsing succeeded\n";
        std::cout << "-------------------------\n";
        matpower::mpc_printer printer;
        printer(mpc_data);
        return 0;
    }
    else
    {
        StringIt some = iter+30;
        std::string context(iter, (some>end)?end:some);
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        std::cout << "stopped at: \": " << context << "...\"\n";
        std::cout << "-------------------------\n";
        return 1;
    }

}

// mpc.bus =
//bus_i	bus_type	Pd	Qd	Gs	Bs	area	Vm	Va	baseKV	zone	Vmax	Vmin

// mpc.gen = 
//	bus	Pg	Qg	Qmax	Qmin	Vg	mBase	status	Pmax	Pmin

// mpc.gencost
// 	model	startup	shutdown	ncost	c(n-1)	...	c0


// mpc.branch
//	fbus	tbus	r	x	b	rateA	rateB	rateC	ratio	angle	status	angmin	angmax
