<?
//MaNGOS Registration Script by kylef
/* Account activation script */

// Get database connection
include ("includes/config.php");

// Create variables from URL.

$acct = $_REQUEST['acct'];
$code = $_REQUEST['code'];

$sql = mysql_query("UPDATE activation SET activated='1' WHERE acct='$acct' AND password='$code'");

$sql_doublecheck = mysql_query("SELECT * FROM activation WHERE acct='$acct' AND password='$code' AND activated='1'");
$doublecheck = mysql_num_rows($sql_doublecheck);

if($doublecheck == 0){
    echo "<strong><font color=red>Your account could not be activated!</font></strong>";
} elseif ($doublecheck > 0) {
	$sql_add_account = mysql_query("INSERT INTO accounts (login, password, email)
	SELECT login, password, email FROM activation");
	$sql_del_activated = mysql_query("DELETE FROM activation WHERE activated ='1'")
	or die (mysql_error());
    /*echo "<strong>Your account has been activated!</strong> You may login below!<br />";
    include ; ffeature not added yet */
}

?> 