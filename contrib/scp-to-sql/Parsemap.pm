
package Parsemap;

our @ISA = qw(Exporter);
our @EXPORT_OK = qw(parse);
our $VERSION = 1.0;

use strict;

sub string_converter
{
    my ($value) = @_;
    return ("'" . $value . "'");
}

sub hex_converter
{
    my ($value) = @_;
    return (hex $value);
}

sub parse
{
    my ($file_name, $lookup_map, $type_table, $info_table) = @_;
    if( ref($lookup_map) ne 'HASH' ) { die("Expect a hash reference for object map not $lookup_map\n"); }
    
    if( !(open(HANDLE, "<$file_name")) )
    {
	printf "Failed to open map file $file_name.\r\n";
    }
    

    while( <HANDLE> )
    {
	if( $_ =~ /^\#/ )
	{
	    # //ignore comments...
	}
	elsif( $_ =~ /^\/\/*/ )
	{
	    # //ignore comments...
	}
	elsif( $_ =~ /^[ \t]*([^ \t].*)\-\>[ \t]*(.*)/ )
	{
	    my $from = $1;
	    my $to = $2;
	    $from =~ s/^[ \t]*//g;
	    $to =~ s/^[ \t]*//g;
	    $from =~ s/[ \t]*$//g;
	    $to =~ s/[ \t]*$//g;
	    
	    if( $from =~ /(^[^ \t\[]+)\[([^\]]*)\]/ )
	    {
		my $from_arr = $1;
		my $array_type_val = $lookup_map->{"__array_type__"};
		if( $array_type_val eq "" )
		{
		    $array_type_val = $from_arr;
		}
		else
		{
		    $array_type_val = $array_type_val . "," . $from_arr;
		}
		
		$lookup_map->{"__array_type__"} = $array_type_val;
	    }
	    
	    $lookup_map->{$from} = $to;
	}
	elsif( $_ =~ /^TypeTable\{([^\}]+)\}/ )
	{
	    my @types = split ',', $1;
	    for my $i(@types)
	    {
		if( $i =~ /([^=]+)=(.*)/ )
		{
		    $type_table->{$1} = $2;		    
		}
	    }
	}
	elsif( $_ =~ /^InfoTable\{([^\}]+)\}/ )
	{
	    my @infos = split ',', $1;
	    for my $i(@infos)
	    {
		if( $i =~ /([^=]+)=(.*)/ )
		{
		    $info_table->{$1} = $2;		    
		}
	    }
	}
    }
    
    close(HANDLE);
}

1;
