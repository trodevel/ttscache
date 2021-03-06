/*

Cache over TTS engines.

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

#include <boost/filesystem.hpp>     // boost::filesystem::exists
#include <boost/locale.hpp>         // boost::locale
#include <sstream>                  // std::ostringstream
#include <locale>                   // std::locale
#include <set>                      // std::set

#include <boost/functional/hash.hpp>    // boost::hash_combine

#include "../utils/dummy_logger.h"      // dummy_log
#include "../utils/mutex_helper.h"      // MUTEX_SCOPE_LOCK
#include "../lang_tools/str_helper.h"   // to_string()
#include "wav_proc.h"                   // convert_mp3_to_wav
#include "str_proc.h"                   // split_into_sentences

#include "namespace_lib.h"       // NAMESPACE_TTSCACHE_START

#define MODULENAME      "TtsCache"

NAMESPACE_TTSCACHE_START

bool file_exist( const std::string& name )
{
     return boost::filesystem::exists( name );
}

class StrHelper
{
public:

static std::string to_string( const TtsCache::WordLocale & w )
{
    std::ostringstream s;

    s << w.word << " " << lang_tools::to_string( w.lang );

    return s.str();
}

static std::string to_string( const TtsCache::Token & t )
{
    std::ostringstream s;

    s << t.id << " " << lang_tools::to_string( t.lang );

    return s.str();
}
};

TtsCache::TtsCache():
    is_inited_( false ),
    tts_( nullptr )
{
}

TtsCache::~TtsCache()
{
}

bool TtsCache::init( const Config & config, tts_connect::ITextToSpeech * gtts )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    if( is_inited__() == true )
    {
        dummy_log_error( MODULENAME, "already inited" );
        return false;
    }

    if( config.word_base_path.empty() )
    {
        dummy_log_error( MODULENAME, "word base path is empty" );
        return false;
    }

    if( config.data_path.empty() )
    {
        dummy_log_error( MODULENAME, "data path is empty" );
        return false;
    }

    if( config.temp_path.empty() )
    {
        dummy_log_error( MODULENAME, "temp path is empty" );
        return false;
    }

    config_ = config;

    if( false == load_state__() )
    {
        dummy_log_warn( MODULENAME, "cannot load state" );
    }

    is_inited_  = true;
    tts_       = gtts;

    return true;
}

bool TtsCache::save_state()
{
    MUTEX_SCOPE_LOCK( mutex_ );

    if( !is_inited__() )
    {
        dummy_log_error( MODULENAME, "not inited" );
        return false;
    }

    return save_state__();
}

bool TtsCache::is_inited__() const
{
    return is_inited_;
}

bool TtsCache::say( const std::string & text, const std::string & filename, lang_tools::lang_e lang )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    if( !is_inited__() )
    {
        dummy_log_error( MODULENAME, "not inited" );
        return false;
    }

    std::vector< std::string > sentences;

    split_into_sentences( sentences, text );

    std::vector< std::string > words_raw;

    split_into_parts( words_raw, sentences, config_.max_length );

    std::vector< std::string > words;

    remove_empty_parts( words, words_raw );

    if( words.empty() )
        return true;

    TokenVect ids;

    convert_words_to_tokens( words, ids, lang );

    say_text( ids, filename );

    return true;
}

std::string TtsCache::get_error_msg() const
{
    MUTEX_SCOPE_LOCK( mutex_ );

    return error_msg_;
}

void TtsCache::set_error_msg__( const std::string & s )
{
    error_msg_  = s;
}

bool TtsCache::say_text( const TokenVect & inp, const std::string & wav_file )
{
    TokenVect::const_iterator it_end    = inp.end();
    TokenVect::const_iterator it        = inp.begin();

    StrVect wav_files;

    for( ; it != it_end; ++it )
    {

        std::string file;

        const Token & t    = *it;

        bool b = generate_wav_file( t, file );

        if( b == false )
        {
            dummy_log_error( MODULENAME, "cannot generate wav file for token %s", StrHelper::to_string( t ).c_str() );
            return false;
        }

        wav_files.push_back( file );
    }

    join_wav_files( wav_files, wav_file );

    return true;
}

bool TtsCache::generate_wav_file( const Token & t, std::string & wav_file )
{
    std::string f_wav   = get_filename_wav( t );

    if( file_exist( f_wav ) )
    {
        // wav file already exists, nothing to do

        wav_file    = f_wav;
        return true;
    }

    std::string f_mp3   = get_filename_mp3( t );

    if( file_exist( f_mp3 ) )
    {
        // mp3 file exists, just convert it into wav

        bool b = convert_mp3_to_wav( f_mp3, f_wav );

        if( b == false )
        {
            dummy_log_error( MODULENAME, "cannot convert mp3 file to wav for token %s", StrHelper::to_string( t ).c_str() );
            return false;
        }

        wav_file    = f_wav;
        return true;
    }

    std::string error;

    // nothing exists, need to generate wav and then convert to mp3

    bool b1 = say_word( t, f_wav, error );

    if( b1 == false )
    {
        dummy_log_error( MODULENAME, "cannot generate wav file for token %s, %s",
                StrHelper::to_string( t ).c_str(), error.c_str() );
        return false;
    }

    bool b2 = convert_wav_to_mp3( f_wav, f_mp3 );

    if( b2 == false )
    {
        dummy_log_error( MODULENAME, "cannot convert wav file to mp3 for token %s", StrHelper::to_string( t ).c_str() );
        return false;
    }

    wav_file    = f_wav;

    return true;
}

std::string TtsCache::get_filename_wav( const Token & t ) const
{
    std::ostringstream s;

    s << config_.temp_path << "/" << t.id << ".wav";

    return s.str();
}

std::string TtsCache::get_filename_mp3( const Token & t ) const
{
    std::ostringstream s;

    s << config_.data_path << "/" << t.id << ".mp3";

    return s.str();
}


lang_tools::lang_e   TtsCache::check_lang( const std::string & s )
{
    using namespace lang_tools;

    // std::cout << "check_lang = " << s << std::endl; // DEBUG

    static std::map< std::string, lang_e > m =
    {
            { "<en>", lang_e::EN },
            { "<de>", lang_e::DE },
            { "<ru>", lang_e::RU },
    };

    auto it = m.find( s );

    if( it != m.end() )
        return it->second;

    return lang_e::UNDEF;
}

const std::string & TtsCache::get_locale_name( lang_tools::lang_e lang )
{
    using namespace lang_tools;

    static const std::map< lang_e, std::string > m =
    {
            { lang_e::EN, "en_GB.UTF-8" },
            { lang_e::DE, "de_DE.UTF-8" },
            { lang_e::RU, "ru_RU.UTF-8" },
    };

    auto it = m.find( lang );

    if( it != m.end() )
        return it->second;

    static const std::string def  = "en_GB.UTF-8";

    return def;
}

void TtsCache::localize( WordLocale & w )
{
    try
    {
        boost::locale::generator gen;
        std::locale loc = gen( "" );

        // Create system default locale
        std::locale::global( loc );

        w.word = boost::locale::to_lower( w.word );
    }
    catch( std::runtime_error &e )
    {

    }
}

bool TtsCache::convert_words_to_tokens( const StrVect & inp, TokenVect & outp, lang_tools::lang_e lang_def )
{
    lang_tools::lang_e lang = lang_def;

    for( auto s : inp )
    {
        lang_tools::lang_e l = check_lang( s );

        if( l != lang_tools::lang_e::UNDEF )
        {
            lang = l;
            continue;   // don't generate a word since it's a keyword
        }

        WordLocale w;

        w.lang  = lang;
        w.word  = s;

        localize( w );

        uint32_t                  id = get_word_id( w );

        if( id == 0 )
        {
            dummy_log_error( MODULENAME, "cannot get id for word '%s'", s.c_str() );
            return false;
        }

        Token t;

        t.id    = id;
        t.lang  = lang;

        outp.push_back( t );
    }

    return true;
}

uint32_t TtsCache::get_word_hash( const WordLocale & w )
{
    std::size_t seed = 0;

    boost::hash_combine( seed, boost::hash_value( lang_tools::to_string( w.lang ) ) );

    boost::hash_combine( seed, boost::hash_value( w.word ) );

    return seed;
}

uint32_t TtsCache::get_word_id( const WordLocale & w )
{
    auto res = word_to_id_.find( w );

    if( res == word_to_id_.end() )
    {
        uint32_t id  = get_word_hash( w );

        add_new_word( w, id );

        return id;
    }

    return res->second;
}

bool TtsCache::add_new_word( const WordLocale & w, uint32_t id )
{
    word_to_id_[ w ]   = id;

    Token t;

    t.id    = id;
    t.lang  = w.lang;

    id_to_word_.insert( MapTokenToString::value_type( t, w.word ) );

    return true;
}

bool TtsCache::say_word( const Token & t, const std::string & wav_file, std::string & error )
{
    const std::string & w = id_to_word_[ t ];

    return tts_->say( w, wav_file, t.lang, error );
}

NAMESPACE_TTSCACHE_END
