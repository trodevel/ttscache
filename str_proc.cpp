/*

String manipulation functions.

Copyright (C) 2014 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

// $Revision: 2563 $ $Date:: 2015-09-13 #$ $Author: serge $


#include "str_proc.h"               // self

#include <set>                      // std::set
#include <algorithm>                // std::replace_if
#include <iostream>                 // DEBUG: cout

#include "../utils/trim.h"                  // trim
#include "../utils/remove_extra_spaces.h"   // remove_extra_spaces

#include "namespace_lib.h"       // NAMESPACE_TTSCACHE_START

#define MODULENAME      "StrProc"

NAMESPACE_TTSCACHE_START

void split( std::vector<std::string> & res, const std::string & str )
{
    std::string temp;

    static const std::set< char > delim = { '.', ',', ';', '!', '?' };

    static auto not_found = delim.end();

    enum state_e
    {
        NORM,
        KEYW
    };

    const char BR1 = '<';
    const char BR2 = '>';

    state_e state = NORM;

    for( auto c : str )
    {
        switch( state )
        {
        case NORM:
            if( c == BR1 )
            {
                if( !temp.empty() )
                {
                    res.push_back( temp );
                    temp.clear();
                }
                temp.append( 1, c );

                state = KEYW;
            }
            else if( delim.find( c ) != not_found )
            {
                if( !temp.empty() )
                {
                    res.push_back( temp );
                    temp.clear();
                }
            }
            else
            {
                temp.append( 1, c );
            }
            break;

        case KEYW:
            if( c == BR2 )
            {
                if( !temp.empty() )
                {
                    temp.append( 1, c );

                    res.push_back( temp );
                    temp.clear();
                }

                state = NORM;
            }
            else
            {
                temp.append( 1, c );
            }
            break;

        default:
            break;
        }
    }

    if( !temp.empty() )
    {
        res.push_back( temp );
    }
}

void remove_special_symbols( std::string & s )
{
//    boost::replace_all( s, "~@#$%^&*()__+`-=[]{}\\:|<>/", " " );
    std::replace_if( s.begin(), s.end(),
                [] (const char& c) { return std::ispunct(c); }, ' ' );
}

void split_into_sentences( std::vector<std::string> & res, const std::string & str )
{
    split( res, str );

    for( auto & s : res )
    {
        remove_special_symbols( s );
        trim( s );
        ::remove_extra_spaces( s );
    }
}

void split_into_parts(
        std::vector<std::string>        & res,
        const std::string               & src,
        unsigned int                    max_length )
{
    std::string::size_type len = src.size();

    if( len < max_length )
    {
        res.push_back( src );
        return;
    }

    std::string::size_type start = 0;

    auto prev_space_pos   = std::string::npos;

    while( start < len )
    {
        if( prev_space_pos != std::string::npos )
        {
            std::string::size_type space_pos = src.find_first_of(' ', prev_space_pos + 1 );

//            std::cout << "A " << std::endl;
#ifdef XXX

            std::cout << "space_pos " << space_pos << std::endl;
            std::cout << "prev_space_pos " << prev_space_pos << std::endl;
            std::cout << "start " << start << std::endl;
#endif  // XXX

            if( space_pos == std::string::npos )
            {
//                std::cout << "AA " << std::endl;

                res.push_back( src.substr( start, prev_space_pos - start ) );
                start           = prev_space_pos + 1;
                prev_space_pos  = space_pos;
            }
            else
            {
                auto part_len = space_pos - start;

                if( part_len > max_length )
                {
//                    std::cout << "ABA " << std::endl;

                    res.push_back( src.substr( start, prev_space_pos - start ) );
                    start   = prev_space_pos + 1;

                    // recalculate part len
                    part_len = space_pos - start;

                    if( part_len > max_length )
                    {
                        prev_space_pos  = std::string::npos;
                    }
                    else
                    {
                        prev_space_pos  = space_pos;
                    }
                }
                else
                {
//                    std::cout << "ABB " << std::endl;
                    prev_space_pos  = space_pos;
                }
            }
        }
        else
        {
            std::string::size_type space_pos = src.find_first_of(' ', start );

//            std::cout << "B " << std::endl;
#ifdef XXX
            std::cout << "space_pos " << space_pos << std::endl;
            std::cout << "prev_space_pos " << prev_space_pos << std::endl;
            std::cout << "start " << start << std::endl;
#endif // XXX

            if( space_pos == std::string::npos )
            {
//                std::cout << "BA " << std::endl;

                // no spaces found, need to cut in the middle of the string
                res.push_back( src.substr( start, max_length ) );
                start   += max_length;
            }
            else
            {
                auto part_len = space_pos - start;

                if( part_len > max_length )
                {
//                    std::cout << "BBA " << std::endl;

                    std::string::size_type iter = part_len / max_length;

                    for( std::string::size_type i = 0; i < iter; ++i )
                    {
                        res.push_back( src.substr( start, max_length ) );
                        start   += max_length;
                    }
                }
                else
                {
//                    std::cout << "BBB " << std::endl;

                    prev_space_pos  = space_pos;
                }
            }
        }


    }
}

void split_into_parts(
        std::vector<std::string>        & res,
        const std::vector<std::string>  & src,
        unsigned int                    max_length )
{
    for( auto & s : src )
    {
        split_into_parts( res, s, max_length );
    }

}

void remove_empty_parts(
        std::vector<std::string>        & res,
        const std::vector<std::string>  & src )
{
    for( auto & s : src )
    {
        std::string s2 = s;
        trim( s2 );
        if( s2.empty() == false )
        {
            res.push_back( s2 );
        }
        else
        {
            //std::cout << "empty token" << std::endl;
        }
    }
}

NAMESPACE_TTSCACHE_END
