#include <cstdio>
#include <iostream>         // cout

#include "../tts_connect/tts_connect.h" // TtsConnect
#include "ttscache.h"           // TtsCache

#include "../utils/dummy_logger.h"          // dummy_log_set_log_level

int main()
{
    printf( "Hello, world\n" );

    dummy_logger::set_log_level( log_levels_log4j::DEBUG );

    std::string input;

    std::cout << "enter text: ";
    std::getline( std::cin, input );

    ttscache::Config config;

    config.word_base_path   = "voc/words.txt";
    config.data_path        = "voc";
    config.temp_path        = "voc_temp";
    config.max_length       = 100;

    tts_connect::ITextToSpeech * tts = tts_connect::TtsConnect::get().get_engine( "gspeak" );

    ttscache::TtsCache g;

    bool b = g.init( config, tts );

    if( b == false )
    {
        std::cout << "ERROR: cannot initialize TtsCache" << std::endl;

        return 0;
    }

    g.say( input, "text.wav" );

    g.save_state();

    return 0;
}
