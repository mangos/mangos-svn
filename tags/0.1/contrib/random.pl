#!/usr/bin/perl
require DBI;

# Database Name
$database = "database_name";

# Database server
$dbserver = "database_server_ip";

# Database Username which can access database
$dbuser = "db_username";

# Database Password
$dbpass = "db_password";


&connectdb;

#1. baseattacktime,rangeattacktime filled with random values between 1000 and 2000
# done

#2. change all 0 mindmg,maxdmg from db according to creature level and also all values above 1000 formula based on unique1
# done

#3 default vendor items
#<unique1> if the creature is a vendor and doesnt have any, we need to make 
#it copy from another vendor of the same type


$range = 1000;
$minimum = 1000;

# baseattacktime
$check = $dbh->prepare("select entryid,baseattacktime from creaturetemplate where baseattacktime='0';");
$check->execute();

 while ($res = $check->fetchrow_hashref()) 
 {
	$random_num = int(rand($range)) + $minimum;
	print "creature is $res->{'entryid'}, baseattacktime is 0, should be $random_num\n";
	$update = $dbh->prepare("update creaturetemplate set baseattacktime=\"$random_num\" where entryid=\"$res->{'entryid'}\";");
	$update->execute();
 }


# rangeattacktime
$check1 = $dbh->prepare("select entryid,rangeattacktime from creaturetemplate where rangeattacktime='0';");
$check1->execute();

 while ($res = $check1->fetchrow_hashref()) 
 {
	$random_num = int(rand($range)) + $minimum;
	print "creature is $res->{'entryid'}, rangeattacktime is 0, should be $random_num\n";
	$update1 = $dbh->prepare("update creaturetemplate set rangeattacktime=\"$random_num\" where entryid=\"$res->{'entryid'}\";");
	$update1->execute();
 }

# mindmg
$check1 = $dbh->prepare("select entryid,level,mindmg from creaturetemplate where mindmg='0' or mindmg > '1000';");
$check1->execute();

 while ($res = $check1->fetchrow_hashref()) 
 {

	if ( ($res->{'mindmg'} > 1000 && $res->{'level'} < 48) || ($res->{'mindmg'} = 0))
	{
		if ( $res->{'level'} > 40 )
		{
		$newmindmg = $res->{'level'}-rand(5);
		$tt = sprintf("%.2f",$newmindmg);
		}
		elsif ( $res->{'level'} > 30 )
		{
		$newmindmg = $res->{'level'}-rand(10);
		$tt = sprintf("%.2f",$newmindmg);
		}
		elsif ($res->{'level'} > 20)
                {
                $newmindmg = $res->{'level'}-rand(15);
		$tt = sprintf("%.2f",$newmindmg);
                }
                elsif ($res->{'level'} > 10)
                {
                $newmindmg = $res->{'level'}-rand(9);
		$tt = sprintf("%.2f",$newmindmg);
                }
                elsif ($res->{'level'} > 5)
                {
                $newmindmg = $res->{'level'}-rand(4);
		$tt = sprintf("%.2f",$newmindmg);
                }
                else
                {
                $newmindmg = $res->{'level'}+rand(1)+1;
		$tt = sprintf("%.2f",$newmindmg);
                }
	}

       if ($res->{'level'} = 1)
	{
	$newmindmg = rand(1)+1;
	$tt = sprintf("%.2f",$newmindmg);
	}

	print "creature is $res->{'entryid'}, mindmg is $res->{'mindmg'}, should be $tt\n";
	$update1 = $dbh->prepare("update creaturetemplate set mindmg=\"$tt\" where entryid=\"$res->{'entryid'}\";");
	$update1->execute();
 }




# maxdmg
$check1 = $dbh->prepare("select entryid,level,maxdmg from creaturetemplate where maxdmg='0' or maxdmg > '1000';");
$check1->execute();

 while ($res = $check1->fetchrow_hashref()) 
 {

	if ( ($res->{'maxdmg'} > 1000 && $res->{'level'} < 48) || ($res->{'maxdmg'} = 0))
	{
		if ( $res->{'level'} > 40 )
		{
		$newmaxdmg = $res->{'level'}+rand(50);
		$tt = sprintf("%.2f",$newmaxdmg);
		}
		elsif ( $res->{'level'} > 20 )
		{
		$newmaxdmg = $res->{'level'}+rand(30);
		$tt = sprintf("%.2f",$newmaxdmg);
		}
		elsif ($res->{'level'} > 10)
                {
                $newmaxdmg = $res->{'level'}+rand(15);
		$tt = sprintf("%.2f",$newmaxdmg);
                }
                elsif ($res->{'level'} > 5)
                {
                $newmaxdmg = $res->{'level'}+rand(8);
		$tt = sprintf("%.2f",$newmaxdmg);
                }
                else
                {
                $newmaxdmg = $res->{'level'}+rand(4);
		$tt = sprintf("%.2f",$newmaxdmg);
                }
	}

       if ($res->{'level'} = 1)
	{
	$newmaxdmg = rand(2)+1;
	$tt = sprintf("%.2f",$newmaxdmg);
	}

	print "creature is $res->{'entryid'}, axdmg is $res->{'mindmg'}, should be $tt\n";
	$update1 = $dbh->prepare("update creaturetemplate set maxdmg=\"$tt\" where entryid=\"$res->{'entryid'}\";");
	$update1->execute();
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



