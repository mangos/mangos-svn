eval 'exec perl -S $0 ${1+"$@"}' # -*- Perl -*-
  if $running_under_some_shell;
#!perl 

use strict;

BEGIN {
    if( defined( $ENV{"PATH"} ) )
    {
	push(@INC, split ':', $ENV{"PATH"});
	push(@INC, ".");
    }
}


sub usage
{
    printf "convert_all usage: \r\n";
    printf "\t mapdir=map_directory [maps]\r\n";
    exit;
};


MAIN: {    
    
    my $map_dir = "maps";

    foreach( @ARGV )
    {
	if( $_ =~ /^mapdir=(.*)/ )
	{
	    $map_dir = $1;
	}
	elsif( $_ =~ /^--help/ )
	{
	    usage();
	}
	else
	{
	    printf "Invalid option $_. Use --help to display options.\r\n";
	    exit;
	}
    }

    my @map_files = glob("$map_dir/*.map");
    for my $map_file( @map_files )
    {
	printf "========================================\r\n";
	my @command = ("scp_to_sql.pl", "map=" . $map_file);
	system(@command);
    }

}
