<?php
setcookie("TestCookie1","Test Value"); /* expires upon session termination */
setcookie("TestCookie2","Another Value",time()+3600);  /* expire in 1 hour */
setcookie( "cookie[one]", "cookieone" ); /* demonstrates arrays within arrays */
setcookie( "cookie[two]", "cookietwo" );
setcookie( "cookie[three]", "cookiethree" ); 
?>

<html>
    <head>
        <title>Example PHP Script</title>
    </head>
    <body>

<h3>Simple Echo</h3>
<?php echo "Hi, I'm a PHP script!"; ?>

<h3>Server Variables</h3>
<b>SCRIPT_NAME:</b> <?php echo $SCRIPT_NAME; ?><br>
<b>QUERY_STRING:</b> <?php echo $QUERY_STRING; ?><br>
<b>SERVER_SOFTWARE:</b> <?php echo $SERVER_SOFTWARE; ?><br>

<h3>Cookies</h3>
<table border=1>
<?php
if ( isset( $HTTP_COOKIE_VARS ) ) {
   while( list( $name, $value ) = each( $HTTP_COOKIE_VARS ) ) {
	  echo "<tr><td>$name</td><td>$value</td></tr>\n";
   }
}
?>
</table>


<p>
For more information on programming with PHP, check out the official PHP page at
<a href="http://www.php.net">www.php.net</a>.
</p>

     </body>
</html>    
