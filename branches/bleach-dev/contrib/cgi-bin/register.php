<?
//MaNGOS Registration Script by kylef

include("includes/header.php");

	
?>
<table border="0">
  <tr>
    <td><?php include("includes/column_left.php"); ?></td>
	<td width="500" align="center"><form action='includes/register.php' method='post' class='register'>
	<br />
        Email:
        <input name='email' type ='text' class='fields' value="<?php echo $mail; ?>" />
        <br />
        <br />
        Username: 
        <input name='login' type ='text' class='fields' value="<?php echo $login; ?>" />
        <br />
        <br />
        Password:
        <input name='password' type="password" value="<?php echo $password; ?>" />
        <br />
        <br />
        <input name="submit" type='submit' value='Submit' />
        <input name="reset" type='reset' value='Reset' />
      </form></td>
  </tr>
</table>
