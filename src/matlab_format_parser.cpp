#include <boost/spirit/include/qi.hpp>

using It  = boost::spirit::istream_iterator;
using Row = std::vector<double>;
using Mat = std::vector<Row>;

int main() {
    typedef std::string::const_iterator StringIt;

    std::string test_var = "MATRIX.{variable_name}=[1,2,3;4,5,6]";

    StringIt iter = test_var.begin();
    StringIt end = test_var.end();

    Mat matrix;
    std::string name;

    {
        using namespace boost::spirit::qi;
        rule<StringIt, std::string()> varname_ = char_("a-zA-Z_") >> *char_("a-zA-Z0-9_");

        if (phrase_parse(iter, end, 
                lit("MATRIX") >> '.' >> '{' >> varname_ >> '}' >> '=' 
                    >> '[' >> (int_ % ',' % ';') >> ']',
                space, name, matrix))
        {
            std::cout << "Parsed: variabled named '" << name << "' [";

            for(auto& row : matrix)
                std::copy(row.begin(), row.end(), std::ostream_iterator<double>(std::cout<<"\n\t",", "));
            std::cout << "\n]\n";
        } else {
            std::cout << "Parse failed\n";
        }
    }

    if (iter!=end)
        std::cout << "Remaining input: '" << std::string(iter,end) << "'\n";
}