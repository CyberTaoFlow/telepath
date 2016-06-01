<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>
<!DOCTYPE html>
<html lang="en">

<head>

	<meta charset="utf-8">
	<title>Telepath 3.0</title>
	
	<link rel="stylesheet" href="css/reset.css">
	<link rel="stylesheet" href="css/login.css">
		
	<script src="js/lib/jquery-1.11.0.min.js"></script>
	<script src="js/lib/jquery-ui-1.10.4.custom.min.js"></script>
	
	<script>
		var telepath = { 
			basePath: "<?php echo dirname($_SERVER['PHP_SELF']) != '/' ? dirname($_SERVER['PHP_SELF']) : ''; ?>",
			controllerPath: "<?php echo $_SERVER['PHP_SELF']; ?>",
		};
	</script>
	
	<script src="js/telepath.checkbox.js"></script>
	<script src="js/telepath.login.js"></script>

</head>

<body>
	<div class="tele-container">
		<img class="tele-logo" src="img/logo_big.png" alt="Hybrid Telepath" />
		<div class="tele-reset">
			
			<h1>Reset Password</h1>
			<div class="tele-error" id="reset-error"></div>
			<span>Please enter your email and we will send you a password reset link.</span>
			<br><br>
			<div class="tele-field">
				<label class="tele-label" for="login-email">Email:</label>
				<input type="text" class="tele-input" id="login-email" name="login-email">
			</div>
			
			<center>
				<div class="tele-button" id="reset-button">Reset</div>
			</center>
			
			<a href="#" id="login-reset-cancel">Cancel</a>
			
		</div>
		<div class="tele-login">
		
			<h1>Login</h1>
			
			<div class="tele-error" id="login-error"></div>
			
			<div class="tele-field">
				<label class="tele-label" for="login-username">Username:</label>
				<input type="text" class="tele-input" id="login-username" name="login-username">
			</div>
			
			<div class="tele-field">
				<label class="tele-label" for="login-password">Password:</label>
				<input type="password" class="tele-input" id="login-password" name="login-password">
				<div id="login-reset">Reset Password</div>
			</div>
			
			<br>
			
<!--
			<div class="tele-field">
				<label class="tele-label" for="login-remember">Remember Me:</label>
				<div id="login-remember"></div>
			</div>
-->
			
			<center>
				<div class="tele-button" id="login-button">Login</div>
			</center>
			
		</div>
	</div>
</body>
</html>
