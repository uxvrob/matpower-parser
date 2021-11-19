#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include<iostream>
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
            //using qi::eol;
            //using qi::eoi;
            using qi::double_;
            using qi::lexeme;
            using ascii::char_;
            using ascii::space;


            mpc_var_name_ %= lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];
            matrix_ %=  double_ % space % ';';

            mpc_ %= 
                lit("mpc")
                >> '.'
                >> mpc_var_name_
                >> '='
                >> '['
                //>> (space|eol|eoi)
                >> matrix_
                //>> (space|eol|eoi)
                >> ']'
                >> ';'
                ;

        }
        qi::rule<Iterator, std::string(), ascii::space_type> mpc_var_name_;
        qi::rule<Iterator, Mat(), ascii::space_type> matrix_;
        qi::rule<Iterator, matpower::mpc_matrix(), ascii::space_type> mpc_;

    };

}

int main() {
    typedef std::string::const_iterator StringIt;
    using boost::spirit::ascii::space;

    //std::string test_var = "mpc.gencosts=[1 2 3;4 5 6];";
    //std::string test_var = "mpc.gencosts=[1,2,3;4,5,6];";
    //std::string test_var = "mpc.gencosts=[1.0 2.0 3.343;4.454 5.4 6.43];";

    std::string test_var = R"""(mpc.bus = [
    1    3   0.0     0.0     0.0     0.0     1      1.00000     0.00000  1.0     1      1.06000     0.94000;
    2    2   21.7    12.7    0.0     0.0     1      1.00000     0.00000  1.0     1      1.06000     0.94000;
    ];)""";

    StringIt iter = test_var.begin();
    StringIt end = test_var.end();
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