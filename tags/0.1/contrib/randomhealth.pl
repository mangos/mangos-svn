#!/usr/bin/perl
require DBI;

# Database Name
$database = "database";

# Database server
$dbserver = "db_ip";

# Database Username which can access database
$dbuser = "db_user";

# Database Password
$dbpass = "db_pass";


&connectdb;

#1. maxhealth

# maxhealth
$check1 = $dbh->prepare("select entryid,level,maxhealth from creaturetemplate where maxhealth='0';");
$check1->execute();

 while ($res = $check1->fetchrow_hashref()) 
 {

		if ( $res->{'level'} >= 64 )
		{
		$end = $res->{'level'} * 50;
		$start = $res->{'level'} * 80;
		$newhealth = int(rand($end)) + $start;
		print "Creature $res->{'entryid'} new health is $newhealth\n";
	        $update = $dbh->prepare("update creaturetemplate set maxhealth=\"$newhealth\" where entryid=\"$res->{'entryid'}\";");
		$update->execute();
                }
		elsif ( $res->{'level'} > 48 )
		{
		$end = $res->{'level'} * 40;
		$start = $res->{'level'} * 70;
		$newhealth = int(rand($end)) + $start;
		print "Creature $res->{'entryid'} new health is $newhealth\n";
	        $update = $dbh->prepare("update creaturetemplate set maxhealth=\"$newhealth\" where entryid=\"$res->{'entryid'}\";");
		$update->execute();
                }
		elsif ( $res->{'level'} > 32 )
		{
		$end = $res->{'level'} * 30;
		$start = $res->{'level'} * 60;
		$newhealth = int(rand($end)) + $start;
		print "Creature $res->{'entryid'} new health is $newhealth\n";
	        $update = $dbh->prepare("update creaturetemplate set maxhealth=\"$newhealth\" where entryid=\"$res->{'entryid'}\";");
		$update->execute();
                }
		elsif ( $res->{'level'} > 24 )
		{
		$end = $res->{'level'} * 30;
		$start = $res->{'level'} * 60;
		$newhealth = int(rand($end)) + $start;
		print "Creature $res->{'entryid'} new health is $newhealth\n";
	        $update = $dbh->prepare("update creaturetemplate set maxhealth=\"$newhealth\" where entryid=\"$res->{'entryid'}\";");
		$update->execute();
                }
		elsif ( $res->{'level'} > 16 )
		{
		$end = $res->{'level'} * 40;
		$start = $res->{'level'} * 60;
		$newhealth = int(rand($end)) + $start;
		print "Creature $res->{'entryid'} new health is $newhealth\n";
	        $update = $dbh->prepare("update creaturetemplate set maxhealth=\"$newhealth\" where entryid=\"$res->{'entryid'}\";");
		$update->execute();
                }
		else 
		{
		$end = $res->{'level'} * 70;
		$start = $res->{'level'} * 150;
		$newhealth = int(rand($end)) + $start;
		print "Creature $res->{'entryid'} new health is $newhealth\n";
	        $update = $dbh->prepare("update creaturetemplate set maxhealth=\"$newhealth\" where entryid=\"$res->{'entryid'}\";");
		$update->execute();
                }

 }



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



