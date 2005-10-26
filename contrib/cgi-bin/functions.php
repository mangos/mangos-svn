<?php
//MaNGOS Registration Script by kylef

 include_once("includes/config.php");

	function server_status() {
	$STATUS = @fsockopen($REALMLIST, 3724, $errno, $errstr, 1);
	
		if (!$STATUS) {
		echo  '<img src="images/off.png">';
		} else {
		echo  '<img src="images/on.png">';

		fclose($STATUS);
		}
	}

	function players_online() {
	// Performing SQL query
	$query = 'SELECT `name` FROM `characters` WHERE `online` =1';
	$result = mysql_query($query) or die('Query failed: ' . mysql_error());
	$numofrows = mysql_num_rows($result);
		   


echo "<TABLE BORDER=\"1\">\n";
echo "<TR bgcolor=\"lightblue\"><TD>Name</TD><TD>Zone</TD><TD>Level</TD></TR>\n";
for($i = 0; $i < $numofrows; $i++) {
    $row = mysql_fetch_array($result); //get a row from our result set
    if($i % 2) { //this means if there is a remainder
        echo "<TR bgcolor=\"#80C688\">\n";
    } else { //if there isn't a remainder we will do the else
        echo "<TR bgcolor=\"#00CC66\">\n";
    }
    echo "<TD>".$row['name']."</TD><TD>".$row['phonenumber']."</TD><TD>".$row['age']."</TD>\n";
    echo "</TR>\n";
}
//now let's close the table and be done with it
echo "</TABLE>\n";


		// Free resultset
		mysql_free_result($result);

		// Closing connection
		mysql_close($link);
		}


?>