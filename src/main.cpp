#include "parser.h"

#include <iostream>
#include <string>

int main(int argc, char** argv)
{
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

    MatpowerParser * p = new  MatpowerParser(case_filename);

    p->printData();

    delete p;

    return 1;

}