<?php
//MaNGOS Registration Script by kylef
// SITE INFO
$SITE		= ""; 			// Site address
$SITE_NAME	= "MaNGOS Test Server";	// Site Name
$realmlist	= "mangos.mine.nu";		// Realmlist IP or DNS


// DATABASE SETTINGS
$HOST		= "localhost"; 			// Host to database (usualy localhost)
$DB			= "mangos"; 				// The database for your MaNGOS
$USER		= ""; 				// Username for your MaNGOS database
$PASSWORD	= ""; 				// And password for your MaNGOS database

	connecttodb($HOST,$DB,$USER,$PASSWORD);
	function connecttodb($HOST,$DB,$USER,$PASSWORD)
	{
	global $link;
	$link=mysql_connect ("$HOST","$USER","$PASSWORD");
	if(!$link){die("Could not connect to MySQL");}
	mysql_select_db("$DB",$link) or die ("could not open db".mysql_error());
	}
?>
