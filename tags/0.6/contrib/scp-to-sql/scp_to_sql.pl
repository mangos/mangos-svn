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


use Parsescp;
use Parsemap;
use Preprocessor;

my %type_table = {};

sub usage
{
    printf "scp_to_sql usage: \r\n";
    printf "                 map=map_files\r\n";
    printf "                 file=scp_file\r\n";
    printf "                 id=id_tag\r\n";
    printf "                 table=table_name\r\n";
    printf "                 outfile=output_file_name\r\n";
    printf "                 --apply-preprocessor [default=false]\r\n";
    exit;
};

sub from_string
{
    my ($col, $value) = @_;
    my $type_is = $type_table{$col};

    if( $type_is eq "hex" )
    {
	return (hex $value);
    }
    elsif( $type_is eq "int" )
    {
	return (int $value);
    }
    elsif( $type_is eq "string" )
    {
	return ("'" . $value . "'");
    }
    elsif( $type_is eq "float" )
    {
	return unpack("i", pack("f", $value));
    }
    elsif( $type_is eq "double" )
    {
	return unpack("i", pack("d", $value));
    }
    elsif( $type_is eq "array_float" )
    {
	my @vals = split ' ', $value;
	my $size = scalar(@vals);
	my $result = "";
	$result = unpack("i", pack("f", $vals[0]));
	for(my $i=1; $i < $size; ++$i)
	{
	    $result = $result . " " . unpack("i", pack("f", $vals[$i]));
	}
	
	return $result;
    }
    elsif( $type_is =~ /^evaluate\((.*)\)$/ )
    {
	my $tmp_val = eval($1);
	if( $tmp_val eq "" )
	{
	    return $value;
	}
	return $tmp_val;
    }
    else
    {
	# default as is.
	return $value;
    }
}

sub concatValues
{
    my ($concat_arr, $def_values_arr, $obj_map, $cols, $col_value, $result_col_name) = @_;
    # concat_arr: array of concat columns
    # def_values_arr: arr of default values correspond to the concate columns
    # obj_map: map{key,value} where key is the column name, value is column value
    # cols, col_value: resulting column ands and column values respectively.
    # result_col_name: all concat_arr columns will map to result_col_name
    my $num = scalar(@{$concat_arr});
    my $num2 = scalar(@{$def_values_arr});
    my $result = "'";
    for(my $i=0; $i < $num; ++$i)
    {	
	my $val = $obj_map->{ $concat_arr->[$i] };
	my $default_value = $def_values_arr->[$i];
	if( $val eq "" )
	{
	    if( $default_value eq "!*" )
	    {
		$default_value = "";  # how do you default it???
	    }
	    elsif( $default_value ne "" && $default_value ne "*" )
	    {
		$val = $default_value;
	    }
	    else
	    {
		return 0; # for contatenation, its impossible to ignore columns
	    }
	}
	elsif( $default_value eq "!*" )
	{
	    return 0; # filtered out.
	}

	# concat it
	$result = $result . from_string($concat_arr->[$i], $val) . " ";
    }

    if( !($result_col_name =~ /^\!/) )
    {
	push  @{$cols}, $result_col_name;
	push @{$col_value}, $result . '\'';
    }

    return 1;
}

sub dump_values
{
    my ($handle, $columns, $col_values, $table_name) = @_;

    if( scalar(@{$columns}) > 0 )
    {
	my $col_str = join ',', @{$columns};
	my $val_str = join ',', @{$col_values};
	printf $handle "INSERT INTO $table_name($col_str) VALUES($val_str)\;\r\n";
    }
};

sub insert_single_column
{
    my ($columns_arr, $col_values_arr, $col_name, $val, $orig_col) = @_;
    my $default_value = "";
    if( $col_name =~ /^([^\(]+)[ \t]*\(([^\)]*)[ \t]*\)/ )
    {
	$col_name = $1;
	$default_value = $2;
    }
    
    if( $val eq "" )
    {
	# column is missing so check default
	if( $default_value eq "*" )
	{
	    return 0; # filter out item
	}
	elsif( $default_value ne "" && $default_value ne "!*" )
	{
	    if( !( $col_name =~ /^\!/ ) )
	    {
		push  @{$columns_arr}, $col_name;
		push @{$col_values_arr}, from_string($orig_col, $default_value);
	    }
	}
	else
	{
	    # ignores to column completly
	}
    }
    elsif( $default_value eq "!*" )
    {
	return 0; # X->Y(!*) means if x value is not null.. then filter it out
    }
    else
    {
	if( !($col_name =~ /^\!/) )
	{
	    push @{$columns_arr}, $col_name;
	    push @{$col_values_arr}, from_string($orig_col, $val);
	}
    }    

    return 1;
}

sub insert_column
{
    my ($columns_arr, $col_values_arr, $col, $val, $orig_col) = @_;

    my @col_arr = split ',', $col;
    my @val_arr = split ' ', $val;
    my $num_col = scalar(@col_arr);
    my $num_val = scalar(@val_arr);

    if( $num_col == 1 )
    {
	return insert_single_column($columns_arr, $col_values_arr, $col, $val, $orig_col);
    }

    my $size = 0;
    my $remaining = 0;
    if( $num_val <= $num_col )
    {
	$remaining = $num_col; # end index not actual number
	$size = $num_val;
    }
    else
    {
	$size = $num_col;
	# note, remaining still zero cuz we ignored the unmapped values
    }

    for(my $i=0; $i < $size; ++$i)
    {
	if( insert_single_column($columns_arr, $col_values_arr, $col_arr[$i], $val_arr[$i], $orig_col) == 0 )
	{
	    return 0;
	}
    }
    
    for(my $i=$size; $i < $remaining; ++$i)
    {
	if( insert_single_column($columns_arr, $col_values_arr, $col_arr[$i], "", $orig_col) == 0 )
	{
	    return 0;
	}
    }

    return 1;
}

sub insert_into_table
{
    my ($handle, $obj_map, $lookup_map, $table_name) = @_;
    if( ref($obj_map) ne 'HASH' ) { die("Expect a hash reference for object map not $obj_map\n"); }
    if( ref($lookup_map) ne 'HASH' ) { die("Expect a hash reference for object map not $lookup_map\n"); }

    my @columns;
    my @col_values;

    # Handle Concat first
    my $has_unique = 0;
    for my $key ( keys %{$lookup_map} )
    {
	# (1) Handles concat
	my $mapped_cols = $lookup_map->{$key};
	if( $mapped_cols eq "" || $mapped_cols eq "__Referencer__" || $key eq "__array_type__" )
	{
	    # continue
	}
	elsif( $key =~ /(^[^ \t\[]+)\[([^\]]*)\]/ )
	{
	    # handle array types
	    # for arrays, its ok not to be in
	    my $col = $1;
	    my $arr_index = $2;

	    if( $arr_index ne "" )
	    {
		if( $arr_index =~ /UNIQUE\(([^\)]+)\)/ )
		{
		    my @unique_items = split ',', $1;
		    my @unique_columns;
		    my @unique_values;
		    for my $unique_k (@unique_items)
		    {
			if( insert_column(\@unique_columns, \@unique_values, $lookup_map->{$unique_k}, $obj_map->{$unique_k}, $unique_k) == 0 )
			{
			    return; # missing values
			}
		    }

		    my $count = 0;
		    my $obj_val = "";
		    do
		    {
			# temporary stores the maps if the unique items
			my @tmp_cols = @unique_columns;
			my @tmp_vals = @unique_values;
			my $array_item = $col . "[" . $count . "]";
			$obj_val = $obj_map->{$array_item};
			if( $obj_val ne "" )
			{
			    if( insert_column(\@tmp_cols, \@tmp_vals, $mapped_cols, $obj_val, $col) == 0 )
			    {
				return; # missing columns;
			    }
			    dump_values($handle, \@tmp_cols, \@tmp_vals, $table_name);
			}
			++$count;
		    }while($obj_val ne "");

		    $has_unique = 1;
		}
		else
		{
		    my $obj_val = $obj_map->{$key};
		    if( insert_column(\@columns, \@col_values, $mapped_cols, $obj_val, $col) == 0 )
		    {
			return; # missing values
		    }
		}
	    }
	    else
	    {
		my $count = 0;
		my $obj_val = "";
		do
		{
		    my $array_item = $col . "[" . $count . "]";
		    $obj_val = $obj_map->{$array_item};
		    if( $obj_val ne "" )
		    {
			if( insert_column(\@columns, \@col_values, $mapped_cols, $obj_val, $col) == 0 )
			{
			    return; # missing columns;
			}
		    }
		    ++$count;		    
		}while($obj_val ne "");
	    }
	}
	elsif( $key =~ /^ConCat\(([^\)]+)\)/ )
	{
	    my $multi_cols = $1;
	    if( $lookup_map->{$key} =~ /^([^\(]+)[ \t]*\(([^\)]+)[ \t]*\)/ )
	    {
		my $concat_tocol = $1;
		my $def_values = $2;
		my @concat_arr = split ',', $multi_cols;
		my @def_values_arr = split ',', $def_values;
		my $num_col = scalar(@concat_arr);
		my $num_def_val = scalar(@def_values_arr);
		die("Invalid map $key to $lookup_map->{$key} ... mapping columns and default values does not match [numcol=$num_col, numdef=$num_def_val]\r\n") if( $num_col != $num_def_val ); 
		if( concatValues(\@concat_arr, \@def_values_arr, $obj_map, \@columns, \@col_values, $concat_tocol) == 0 )
		{
		    return; # something's wrong... ignore object
		}
	    }
	    else 
	    {
		die("Invalid ConCat map encountered from $key to $lookup_map->{$key}. Format should be X()->Y()\r\n");
	    } 
	}
	else
	{
	    if( insert_column(\@columns, \@col_values, $mapped_cols, $obj_map->{$key}, $key) == 0 )
	    {
		return;
	    }
	}

    }

    if( $has_unique == 0 )
    {
	dump_values($handle, \@columns, \@col_values, $table_name);
    }

}

sub generate_sql
{
    my ($handle, $obj_map, $lookup_map, $table_name) = @_;
    if( ref($obj_map) ne 'HASH' ) { die("Expect a hash reference for object map not $obj_map\n"); }
    if( ref($lookup_map) ne 'HASH' ) { die("Expect a hash reference for object map not $lookup_map\n"); }

    for my $i ( keys %{$obj_map} )
    {
	if( $i ne "" )
	{
	    insert_into_table($handle, $obj_map->{$i}, $lookup_map, $table_name);
	}
    }

}

MAIN: {    
    
    my $map_name;
    my $file_name = "";
    my %info_table;

    foreach( @ARGV )
    {
	if( $_ =~ /^file=(.*)/ )
	{
	    $file_name = $1;
	}
	elsif( $_ =~ /^map=(.*)/ )
	{
	    $map_name = $1;
	}
	elsif( $_ =~ /^--help/ )
	{
	    usage();
	}
	elsif( $_ =~ /^id=(.*)/ )
	{
	    $info_table{"id"} = $1;
	}
	elsif( $_ =~ /^table=(.*)/ )
	{
	    $info_table{"table"} = $1;
	}
	elsif( $_ =~ /outfile=(.*)/ )
	{
	    $info_table{"outfile"} = $1;
	}
	elsif( $_ =~ /--apply-preprocessor$/ )
	{
	    $info_table{"apply-preprocessor"} = "true";
	}
	else
	{
	    printf "Invalid option $_. Use --help to display options.\r\n";
	    exit;
	}
    }

    my %object_map;
    my %lookup_map;
    
    # (1) parse map file...
    die("Missing map file(s).  Type --help for usage options.\r\n") if( $map_name eq "" );
    printf "Parse map file [$map_name] begin.....\r\n";
    Parsemap::parse($map_name, \%lookup_map, \%type_table, \%info_table);
    printf ".................................DONE\r\n";

    # validation first...
    my $table_name = $info_table{"table"};
    die("Missing table name options.  Type --help for usage options or specify in your map file.\r\n") if( $table_name eq "" );    

    if( $file_name eq "" )
    {
	$file_name = $info_table{"fileName"};
    }

    # parse source file
    printf "Parse source file [$file_name] begin....\r\n";
    Parsescp::parse($file_name, \%object_map, \%lookup_map, $info_table{"id"} );
    printf ".................................DONE\r\n";
    
    if( $info_table{"apply-preprocessor"} ne "" )
    {
	printf "Applying preprocessor...................\r\n";
	Preprocessor::apply(\%object_map, \%lookup_map, $info_table{"apply-preprocessor"});
	printf ".................................DONE\r\n";
    }
    elsif( $info_table{"apply-zone"} ne "" )
    {
	printf "Applying processing zones................\r\n";
	Preprocessor::apply_zone(\%object_map);
	printf ".................................DONE\r\n";
    }

    # validate output file
    my $sql_file = $info_table{"outfile"};
    if( $sql_file eq "" )
    {
	$sql_file = "table.sql";
    }
    elsif( !($sql_file =~ /(.*)\.sql$/) )
    {
	$sql_file = $sql_file . ".sql";
    }
    
    
    printf "Opening file $sql_file to write.\r\n";
    die("Failed to open file $sql_file to write..") if( !(open(HANDLE, ">$sql_file")) );
    
    printf "Apply map file......................\r\n";
    generate_sql(*HANDLE, \%object_map, \%lookup_map, $table_name);
    printf ".................................DONE\r\n";

}
