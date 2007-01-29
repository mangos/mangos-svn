
package Parsescp;

our @ISA = qw(Exporter);
our @EXPORT_OK = qw(parse);
our $VERSION = 1.0;

use strict;

sub is_an_array_type
{
    my ($array_types, $id) = @_;
    my $size = scalar(@{$array_types});
    for(my $i=0; $i < $size; ++$i)
    {
	if( $id eq $array_types->[$i] )
	{
	    return 1;
	}
    }

    return 0;
}

sub parse
{
    my ($file_name, $obj_map, $lookup_map, $id_tag) = @_;
    if( ref($obj_map) ne 'HASH' ) { die("Expect a hash reference for object map not $obj_map\n"); }
    if( ref($lookup_map) ne 'HASH' ) { die("Expect a hash reference for object map not $lookup_map\n"); }

    if( !(open(HANDLE, "<$file_name")) )
    {
	printf "Failed to open file $file_name.\r\n";
    }

    # build array maps
    my %array_type_count;    
    my $current_id = 0;
    my @array_types = split ',', $lookup_map->{"__array_type__"};
    my $num_array_types = scalar(@array_types);

    while( <HANDLE> )
    {
	if( $_ =~ /^\[([^ ]+)[ \t]*([0-9]*)\]/ )
	{
	    # resetting the array types
	    for(my $i=0; $i < $num_array_types; ++$i)
	    {
		$array_type_count{ $array_types[$i] } = 0;
	    }

	    my $tag = $1;
	    my $tag_value = $2;
	    $obj_map->{""} = {}; # clears the map
	    if( $id_tag eq "" || $id_tag eq $tag)
	    {
		$current_id = $tag_value;
		$obj_map->{$current_id}{$tag} = $tag_value;
	    }
	    else
	    {
		$current_id = ""; 
	    }
	}
	elsif( $_ =~ /^\/\/*/ )
	{
	    # //ignore comments...
	}
	elsif( $_ =~ /^[ \t]*([\w]*)[ \t]*=[ \t]*([^\n\r]*)[\s]*/ )
	{
	    my $key = $1;
	    my $val = $2;

	    if( $id_tag eq $key )
	    {
		my $prev_id = $current_id;
		$current_id = $val;
		$obj_map->{$current_id} = $obj_map->{$prev_id};
		$obj_map->{$prev_id} = {}; #clears the map
	    }

	    $val =~ s/[\r\n]//g;
	    $val =~ s/\'/\\\'/g;
	    $val =~ s/\"/\\\"/g;
	    $val =~ s/\$/\\\$/g;
	    $val =~ s/\%/\\\%/g;

	    if( $val =~ /^([^\/]+)[ t]*\/\/*/ )
	    {
		$val = $1;
	    }

	    if( is_an_array_type(\@array_types, $key) == 1 )
	    {
		my $arr_idx = $array_type_count{$key};
		$array_type_count{$key} += 1;
		$key = $key . "[" . $arr_idx . "]";
	    }

	    $obj_map->{$current_id}{$key} = $val;
	}
    }

    $obj_map->{""} = {}; # clears the map
    close(HANDLE);
}



1;
