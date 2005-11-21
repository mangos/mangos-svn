/* ProgressBar.cpp
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ProgressBar.hpp"

barGoLink::~barGoLink()
{
  printf( "\n" );
}

barGoLink::barGoLink( int row_count )
{
  rec_no    = 0;
  rec_pos   = 0;
  indic_len = 50; // size of the progress bar (50 chars)
  num_rec   = row_count;
  empty     = " ";
#ifdef _WIN32 // UQ1: This *MAY* work in *nix too?? Dunno...
  full      = "\xDB";
#else //!_WIN32
  full      = "*";
#endif //_WIN32
#ifdef _WIN32
  printf( "\xB3" );
#else //!_WIN32
  printf( "[" );
#endif //_WIN32
  for ( int i = 0; i < indic_len; i++ ) printf( empty );
#ifdef _WIN32
  printf( "\xB3 0%%\r\xB3" );
#else //!_WIN32
  printf( "] 0%%\r[" );
#endif //_WIN32
}

void barGoLink::step( void )
{
  int i, n;//, t;

  if ( num_rec == 0 ) return;  
  rec_no++;
  n = rec_no * indic_len / num_rec;
  if ( n != rec_pos )
  {
#ifdef _WIN32
    printf( "\r\xB3" );
#else //!_WIN32
    printf( "\r[" );
#endif //_WIN32
    for ( i = 0; i < n; i++ ) printf( full );
    for ( ; i < indic_len; i++ ) printf( empty );
	float percent = (((float)n/(float)indic_len)*100);
#ifdef _WIN32
    printf( "\xB3 %i%%  \r\xB3", (int)percent);
#else //!_WIN32
	printf( "] %i%%  \r[", (int)percent);
#endif //_WIN32
    //for ( i = 0; i < 3000000; i++ ) t = 100*200; // generate a short delay -- UQ1: Removed.. No point waiting longer just for display...
    rec_pos = n;
  }
}
