#include "parser.h"

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

using ColumnHeaders = std::vector<std::string>;
using Row = std::vector<double>;
using Mat = std::vector<Row>;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace spirit = boost::spirit;
namespace fusion = boost::fusion;

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

MatpowerParser::MatpowerParser(std::string filename)
{

	if(this->open(filename))
	{
		this->parse(this->case_storage);
	}


}

bool MatpowerParser::open(std::string filename)
{

	std::ifstream fin(filename, std::ios::in);

	if(!fin)
    {
        std::cerr << "Error: Could not open input file: "
            <<  filename << std::endl;
        return false;
    }

    // Copy case_file into string storage & close file
    fin.unsetf(std::ios::skipws); // Don't skip white spaces
    std::copy(
        std::istream_iterator<char> (fin),
        std::istream_iterator<char>(),
        std::back_inserter(this->case_storage)
    );
    fin.close();

    return true;

}

bool MatpowerParser::parse(std::string data)
{

	if(this->case_storage == "") return false;

    typedef std::string::const_iterator StringIt;
    using boost::spirit::ascii::space;

    StringIt iter = case_storage.begin();
    StringIt end = case_storage.end();

    matpower::matpower_grammar<StringIt> mpg;
    matpower::skip_grammar<StringIt> skipper;

    bool r = phrase_parse(iter, end, mpg, skipper, this->mpc_data);

    if (r && iter == end)
    {
        std::cout << "Parse successful!" << std::endl;
        return true;
        
    } 
    else {

    	std::cout << "Parse unsuccessful" << std::endl;
        return false;
    }
}

void MatpowerParser::printData()
{
        for(auto& v : this->mpc_data)
        {
            std::cout << "Parsed variable name: '" << boost::fusion::at_c<0>(v) << "' [";

            for(auto& row : boost::fusion::at_c<1>(v))
                std::copy(row.begin(), row.end(), std::ostream_iterator<double>(std::cout<<"\n\t",", "));
            std::cout << "\n]\n";
        }

}

