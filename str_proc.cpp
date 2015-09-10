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

// $Revision: 2548 $ $Date:: 2015-09-10 #$ $Author: serge $


#include "str_proc.h"               // self

#include <set>                      // std::set
#include <boost/algorithm/string/replace.hpp>   // replace_all

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
    boost::replace_all( s, "~@#$%^&*()__+`-=[]{}\\:|<>/", " " );
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
        unsigned int                    max_lenghth )
{
    std::string::size_type len = src.size();

    if( len < max_lenghth )
    {
        res.push_back( src );
        return;
    }

    std::string::size_type i = 0;

    std::string::size_type prev_space_pos = std::string::npos;

    while( i < len )
    {
        std::string::size_type space_pos = src.find_first_of(' ', i );

        if( space_pos == std::string::npos )
        {
            if( prev_space_pos == std::string::npos )
            {
                // no spaces found, need to cut in the middle of the string
                res.push_back( src.substr( i, max_lenghth ));
                i += max_lenghth;
            }
            else
            {

            }
        }
        else
        {

        }
    }
}

void split_into_parts(
        std::vector<std::string>        & res,
        const std::vector<std::string>  & src,
        unsigned int                    max_lenghth )
{
    for( auto & s : src )
    {
        split_into_parts( res, s, max_lenghth );
    }

}


NAMESPACE_TTSCACHE_END
