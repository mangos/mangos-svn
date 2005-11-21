<?php
//MaNGOS Registration Script by kylef

 include("config.php");


$reg_password  	  = $_POST['password']; 
	$reg_user         = $_POST['login'];
	$reg_email        = $_POST['email'];
/* Strip some slashes in case the user entered
any escaped characters. */

	$reg_password = stripslashes($reg_password);
	$reg_user = stripslashes($reg_user);
	$reg_email = stripslashes($reg_email);

/* Encrypt the password */

	$reg_password2 = md5($reg_password);
/* Do some error checking on the form posted fields */

if((!$reg_password) || (!$reg_user) || (!$reg_email)){
    echo 'You did not submit the following required information! <br />';
    if(!$reg_user){
        echo "Username is a required field. Please enter it below.<br />";
    }
    if(!$reg_password){
        echo "Password is a required field. Please enter it below.<br />";
    }
    if(!$reg_email){
        echo "Email Address is a required field. Please enter it below.<br />";
    }
    include 'register.php'; // Show the form again!
    /* End the error checking and if everything is ok, we'll move on to
     creating the user account */
    exit(); // if the error checking has failed, we'll exit the script!
}  
    
/* Let's do some checking and ensure that the user's email address or username
does not exist in the database */

$sql_email_check = mysql_query("SELECT email FROM activation, accounts
            WHERE email='$reg_email'");
$sql_login_check = mysql_query("SELECT login FROM activation
            WHERE login='$reg_user'");

$email_check = mysql_num_rows($sql_email_check);
$login_check = mysql_num_rows($sql_login_check);

if(($email_check > 0) || ($login_check > 0)){
    echo "Please fix the following errors: <br />";
    if($email_check > 0){
        echo "<strong>Your email address has already been used by another member
        in our database. Please submit a different Email address!<br />";
        unset($email);
    }
    if($login_check > 0){
        echo "The username you have selected has already been used by another member
         in our database. Please choose a different Username!<br />";
        unset($login);
    }
    include 'register.php'; // Show the form again!
    exit();  // exit the script so that we do not create this account!
}

/* Everything has passed both error checks that we have done.
It's time to create the account! */


// Enter info into the Database.
$sql = mysql_query("INSERT INTO activation (login, password, email)
        VALUES('$reg_user', '$reg_password2', '$reg_email')")
        or die (mysql_error());

if(!$sql){
    echo 'There has been an error creating your account. Please contact the webmaster.';
} else {
    $acct = mysql_insert_id();
    // Let's mail the user!
    $subject = "Your Membership at $SITE_NAME!";
    $message = "Dear $reg_login,
    Thank you for registering at our website, $SITE!
    
    You are two steps away from logging in and accessing your account.
    
    To activate your membership,
    please click here: $SITE/activate.php?acct=$acct&code=$reg_password2
    
    Once you activate your account, you will be able to login
    with the following information:
    Username: $reg_user
    Password: $reg_password
    
    Thanks!
    The Webmaster
    
    This is an automated response, please do not reply!";
    
    mail($reg_email, $subject, $message,
        "From: MyDomain Webmaster<admin@mydomain.com>\n
        X-Mailer: PHP/" . phpversion());
    echo 'Your account information has been mailed to your email address!
    Please check it and follow the directions!';
}

?>