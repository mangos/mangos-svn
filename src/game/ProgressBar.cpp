// Simple Console Progress Bar - function definitions
// sani@gorna.net

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
