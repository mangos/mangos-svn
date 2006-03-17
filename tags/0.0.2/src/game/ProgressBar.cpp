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
  full      = "*";
  printf( "[" );
  for ( int i = 0; i < indic_len; i++ ) printf( empty );
  printf( "] 100%\r[" );
}

void barGoLink::step( void )
{
  int i, n, t;

  if ( num_rec == 0 ) return;  
  rec_no++;
  n = rec_no * indic_len / num_rec;
  if ( n != rec_pos )
  {
    printf( "\r[" );
    for ( i = 0; i < n; i++ ) printf( full );
    for ( ; i < indic_len; i++ ) printf( empty );
    printf( "\r" );
    for ( i = 0; i < 3000000; i++ ) t = 100*200; // generate a short delay
    rec_pos = n;
  }
}
