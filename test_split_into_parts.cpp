#include <iostream>         // cout

#include "str_proc.h"       // TtsCache

int main()
{
    std::string input;

    std::cout << "enter text: ";
    std::getline( std::cin, input );

    std::cout << "input text size " << input.size() << std::endl;

    std::vector<std::string>        res;
    std::vector<std::string>        src;

    src.push_back( input );

    ttscache::split_into_parts( res, src, 100 );

    std::cout << "got " << res.size() << " records" << std::endl;

    for( auto & s : res )
    {
        std::cout << s.size() << ": " << s << std::endl;
    }

    return 0;
}
