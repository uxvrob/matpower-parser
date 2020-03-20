#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <iostream>
#include <string>
#include <complex>
#include <fstream>

using Row = std::vector<double>;
using Mat = std::vector<Row>;

namespace matpower{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

//    enum bus_type
//    {
//        PQ=1,
//        PV=2,
//        REF=3,
//        ISOLATED=4
//    };

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

    struct matpower_data
    {
	std::string name;
	Mat matrix;
    };
} //matpower

BOOST_FUSION_ADAPT_STRUCT(
	matpower::matpower_data,
	(std::string, name)
	(Mat, matrix)
)

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

namespace matpower{


    template <typename Iterator>
    struct matpower_data_parser : qi::grammar<Iterator, matpower_data(), ascii::space_type>
    {
        matpower_data_parser() : matpower_data_parser::base_type(start)
        {
            using qi::int_;
            using qi::lit;
            using qi::double_;
	    using qi::lexeme;
            using ascii::char_;

	    varname_ %= lexeme[ascii::char_("a-zA-Z_") >> *ascii::char_("a-zA-Z0-9_")];
            start %=
		    lit("mpc")
		    >> "."
		    >> varname_
		    >> '='
		    >> '['
		    >> (double_ % ' ' % ';')
		    >> ']'
		    >> ';'
		    ;


        }
	qi::rule<Iterator, std::string(), ascii::space_type> varname_;
        qi::rule<Iterator, matpower_data(), ascii::space_type> start;
    };

} //matpower


int main()
{
    
    using boost::spirit::ascii::space;
    using It = boost::spirit::istream_iterator;

    typedef std::string::const_iterator iterator_type;
    typedef matpower::matpower_data_parser<iterator_type> matpower_data_parser;

    // matpower data grammar
    matpower_data_parser mdg;
    matpower::matpower_data md;

    std::string case_filename;
    case_filename = "../data/pglib_opf_case14_ieee.m";
    
    std::ifstream case_file(case_filename, std::ios::in);
    if(case_file.is_open()){

        std::cout << "=====\t\tStarting matpower parser\t\t====\n\n";
       
    	case_file.unsetf( std::ios::skipws ); 
	It begin(case_file);
	It end;	

	bool r = phrase_parse(begin, end, mdg, space, md);
	if(r){
		std::cout << "Parse: variable named: '" << boost::fusion::as_vector(md.name) << "' [";
		for (auto& row : boost::fusion::as_vector(md.matrix))
			std::copy(row.begin(), row.end(), 
				std::ostream_iterator<double>(std::cout<<"\n\t", ", "));
		std::cout << "\n]\n";
	}
	

        std::cout << "=== END ===\n\n";
    }
    case_file.close();

    return 0;
}

// mpc.bus =
//bus_i	bus_type	Pd	Qd	Gs	Bs	area	Vm	Va	baseKV	zone	Vmax	Vmin

// mpc.gen = 
//	bus	Pg	Qg	Qmax	Qmin	Vg	mBase	status	Pmax	Pmin

// mpc.gencost
// 	model	startup	shutdown	ncost	c(n-1)	...	c0


// mpc.branch
//	fbus	tbus	r	x	b	rateA	rateB	rateC	ratio	angle	status	angmin	angmax
