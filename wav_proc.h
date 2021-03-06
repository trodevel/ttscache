/*

Low-level WAV files processing.

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

// $Revision: 2940 $ $Date:: 2015-12-07 #$ $Author: serge $

#ifndef WAV_PROC_H
#define WAV_PROC_H

#include <string>           // std::string
#include <vector>           // std::vector

#include "namespace_lib.h"  // NAMESPACE_TTSCACHE_START


NAMESPACE_TTSCACHE_START

bool convert_mp3_to_wav( const std::string & inp, const std::string & outp );
bool convert_wav_to_mp3( const std::string & inp, const std::string & outp );
bool join_wav_files( const std::vector< std::string > & inp, const std::string & outp );

NAMESPACE_TTSCACHE_END

#endif  // WAV_PROC_H
