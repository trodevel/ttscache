/*

Cache over Gtts.

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

// $Revision: 2545 $ $Date:: 2015-09-09 #$ $Author: serge $

#ifndef TTSCACHE_H
#define TTSCACHE_H

#include <string>                   // std::string
#include <mutex>                    // std::mutex

#include <map>                      // std::map
#include <vector>                   // std::vector
#include <stdint.h>                 // uint32_t

#include "../gspeak/i_text_to_speech.h"     // ITextToSpeech
#include "../lang_tools/language_enum.h"    // lang_e

#include "namespace_lib.h"  // NAMESPACE_TTSCACHE_START


NAMESPACE_TTSCACHE_START

struct Config
{
    std::string     word_base_path;
    std::string     data_path;
    std::string     temp_path;
    unsigned int    max_length;
};

class TtsCache
{
    friend class StrHelper;
//private:
public:

    struct WordLocale
    {

        lang_tools::lang_e  lang;
        std::string         word;

        bool operator<( const WordLocale & rh ) const
        {
            if( lang < rh.lang )
                return true;

            if( lang == rh.lang && word < rh.word )
                return true;

            return false;
        }
    };

    struct Token
    {
        //friend class boost::serialization::access;

        uint32_t              id;
        lang_tools::lang_e  lang;

        bool operator<( const Token & rh ) const
        {
            if( lang < rh.lang )
                return true;

            if( lang == rh.lang && id < rh.id )
                return true;

            return false;
        }
    };

    typedef std::map< WordLocale, uint32_t >      MapWordLocaleToInt;
    typedef std::map< Token, std::string >      MapTokenToString;

    typedef std::vector< std::string >          StrVect;
    typedef std::vector< Token >                TokenVect;

public:
    TtsCache();

    ~TtsCache();

    bool init( const Config & config, gspeak::ITextToSpeech * gtts );
    bool save_state();

    // interface of ITextToSpeech
    bool say( const std::string & text, const std::string & filename, lang_tools::lang_e lang = lang_tools::lang_e::EN );
    std::string get_error_msg() const;


private:

    bool is_inited__() const;

    bool save_state__();

    bool load_state__();

    bool parse_line_and_insert__( const std::string & s, uint32_t line_num );

    void set_error_msg__( const std::string & s );

    bool convert_words_to_tokens( const StrVect & inp, TokenVect & outp, lang_tools::lang_e lang );

    bool say_text( const TokenVect & inp, const std::string & wav_file );

    static uint32_t get_word_hash( const WordLocale & w );

    uint32_t get_word_id( const WordLocale & w );

    static void localize( WordLocale & w );

    static std::string get_locale_name( lang_tools::lang_e lang );

    bool add_new_word( const WordLocale & w, uint32_t id );

    bool generate_wav_file( const Token & t, std::string & wav_file );

    std::string get_filename_wav( const Token & t ) const;
    std::string get_filename_mp3( const Token & t ) const;

    bool say_word( const Token & t, const std::string & mp3_file, std::string & error );

    static lang_tools::lang_e   check_lang( const std::string & s );

public:


private:
    mutable std::mutex          mutex_;

    bool                        is_inited_;

    Config                      config_;

    MapWordLocaleToInt          word_to_id_;
    MapTokenToString            id_to_word_;

    gspeak::ITextToSpeech       * gtts_;

    std::string                 error_msg_;
};

NAMESPACE_TTSCACHE_END

#endif  // TTSCACHE_H
