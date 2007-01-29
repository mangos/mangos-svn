#!/usr/bin/perl
require DBI;

# Database Name
$database = "database_name";

# Database server
$dbserver = "database_server_ip";

# Database Username which can access database
$dbuser = "db_user";

# Database Password
$dbpass = "db_pass";


&connectdb;

#3 default vendor items
#<unique1> if the creature is a vendor and doesnt have any, we need to make 
#it copy from another vendor of the same type


# vendors compare data
$check = $dbh->prepare("select entryid,subname from creaturetemplate where subname like '%merchant%';");
$check->execute();

 while ($res = $check->fetchrow_hashref()) 
 {
	$compare = $dbh->prepare("select vendorGuid from vendors where vendorGuid=\"$res->{'entryid'}\";");
	$compare->execute();
	$res1 = $compare->fetchrow_hashref();

		if(!$res1)
		{
		print "Vendor: $res->{'entryid'} is missing, subname is $res->{'subname'}\n";
		}

 }


$compare->finish();
&disconnectdb;

# Sub ConnectDB
sub connectdb {
                # MySQL
                $data_source = "DBI:mysql:$database:$dbserver";
                $dbh = DBI->connect($data_source, $dbuser, $dbpass, { 
RaiseError
 => 0, AutoCommit => 1 })
                || &error('mysql_error');

                }

# Sub Disconnect function
sub disconnectdb {
                $dbh->disconnect;
                  }



