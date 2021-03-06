/*

Cache over TTS engines. Serialization.

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

// $Revision: 2947 $ $Date:: 2015-12-07 #$ $Author: serge $


#include "ttscache.h"               // self

#include <sstream>                  // std::ostringstream
#include <fstream>                  // std::ofstream
#include <limits>                   // std::numeric_limits

#include "../lang_tools/parser.h"       // to_lang()
#include "../lang_tools/str_helper.h"   // to_string()
#include "../utils/dummy_logger.h"      // dummy_log
#include "../utils/trim.h"              // trim

#define MODULENAME      "TtsCache"

NAMESPACE_TTSCACHE_START

std::ofstream & operator <<( std::ofstream & os, const TtsCache::Token & t )
{
    os << t.id << " " << lang_tools::to_string( t.lang );

    return os;
}

std::ofstream & operator <<( std::ofstream & os, const TtsCache::MapTokenToString::value_type & v )
{
    os << v.first << " " << v.second << std::endl;

    return os;
}

bool TtsCache::save_state__()
{
    std::ofstream ofs( config_.word_base_path );

    ofs << "VER_2_0" << "\n";

    for( const auto & e : id_to_word_ )
    {
        ofs << e;
    }

    dummy_log_debug( MODULENAME, "saved words = %u", id_to_word_.size() );

    return true;
}

bool TtsCache::load_state_v2__( std::ifstream & is, uint32_t line_num )
{
    std::string line;

    while( is.good() )
    {
        std::getline( is, line );

        line_num++;

        bool b = parse_line_and_insert__( line, line_num );

        if( b == false )
            dummy_log_warn( MODULENAME, "cannot parse record '%s' at line %u", line.c_str(), line_num );
    }

    dummy_log_debug( MODULENAME, "loaded words = %u", id_to_word_.size() );

    return true;
}

bool TtsCache::load_state__()
{
    std::ifstream in_file( config_.word_base_path.c_str() );

    if( in_file.is_open() == false )
    {
        dummy_log_error( MODULENAME, "cannot open file %s", config_.word_base_path.c_str() );

        return false;
    }

    if( in_file.good() )
    {
        std::string line;

        std::getline( in_file, line );

        if( line == "VER_2_0" )
        {
            load_state_v2__( in_file, 1 );
        }
        else
        {
            dummy_log_error( MODULENAME, "unsupported version %s", line.c_str() );

            return false;
        }
    }
    else
    {
        dummy_log_error( MODULENAME, "cannot read from file %s", config_.word_base_path.c_str() );

        return false;
    }

    in_file.close();

    return true;
}

bool TtsCache::parse_line_and_insert__( const std::string & si, uint32_t line_num )
{
    std::string s( si );

    trim( s );

    if( s.empty() )
        return true;

    std::istringstream iss( s );

    WordLocale w;
    uint32_t id;

    std::string lang_str;

    iss >> id;
    iss >> lang_str;
    std::getline( iss, w.word );

    trim( w.word );

    if( w.word.empty() )
        return false;

    w.lang  = lang_tools::to_lang( lang_str );

    return add_new_word( w, id );
}

NAMESPACE_TTSCACHE_END
