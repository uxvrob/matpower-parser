#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

using It  = boost::spirit::istream_iterator;
using Row = std::vector<double>;
using Mat = std::vector<Row>;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;


struct mpc
{
    std::string name;
    Mat matrix;
};

BOOST_FUSION_ADAPT_STRUCT(
    mpc,
    (std::string, name)
    (Mat, matrix)
)

template<typename Iterator>
struct matpower_grammar
    : qi::grammar<Iterator, mpc(), ascii::space_type>
{
    matpower_grammar()
        : matpower_grammar::base_type(start_)
        {
            using qi::lexeme;
            using qi::double_;
            using qi::lit;
            using ascii::char_;

            varname_ %= lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];
            matrix_ %= lexeme[double_ % ' ' % ';'];
            start_ %=
                lit("mpc") 
                >> '.' 
                >> varname_
                >> '=' 
                >> '[' 
                >> matrix_
                >> ']'
                >> ';'
                ;
        }
        qi::rule<Iterator, Mat(), ascii::space_type> matrix_; 
        qi::rule<Iterator, std::string(), ascii::space_type> varname_;
        qi::rule<Iterator, mpc(), ascii::space_type> start_;
};



int main() {
    typedef std::string::const_iterator StringIt;
    using boost::spirit::ascii::space;

    //std::string test_var = "mpc.gencosts=[1 2 3;4 5 6];";
    //std::string test_var = "mpc.gencosts=[1,2,3;4,5,6];";
    std::string test_var = "mpc.gencosts=[1.0 2.0 3.343;4.454 5.4 6.43];";

    StringIt iter = test_var.begin();
    StringIt end = test_var.end();

    mpc mpc_data;

    matpower_grammar<StringIt> mpg;

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