#!/usr/bin/perl
require DBI;

# Database Name
$database = "mangos";

# Database server
$dbserver = "xxx.xxx.xxx.xxx";

# Database Username which can access finance database
$dbuser = "mangosuser";

# Database Password
$dbpass = "mangospass";


&connectdb;

$i = 0;
$check = $dbh->prepare("select vendorGuid as vendor,itemGuid as item from vendors group by vendorGuid,itemGuid having count(vendorGuid)>1;");

$check->execute();

 while ($res = $check->fetchrow_hashref()) 
 {
#	print "vendorGuid - $res->{'vendor'}, itemGuid - $res->{'item'}\n";
	$delete = $dbh->prepare("delete from vendors where vendorGuid=\"$res->{'vendor'}\" and itemGuid=\"$res->{'item'}\" limit 1;");
	$delete->execute();
	$i++;
 }

$delete->finish();	
$check->finish();
	print "$i\n";

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



