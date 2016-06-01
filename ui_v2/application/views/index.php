<?php
?>
<!DOCTYPE html>

<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <title>Hybrid Telepath</title>
	
	<link rel="stylesheet" href="extjs/resources/css/ext-all-gray.css">
	<script src="extjs/ext42-all.js"></script>
	
	<script src="lib/jquery-1.11.1.min.js"></script>
	<script src="lib/yepnope.js"></script>
	
	<script src="lib/jquery-ui-1.10.3.custom.min.js"></script>
		
	<script>
		var telepath = { 
			basePath: "<?php echo dirname($_SERVER['PHP_SELF']) != '/' ? dirname($_SERVER['PHP_SELF']) : ''; ?>",
			controllerPath: "<?php echo $_SERVER['PHP_SELF']; ?>",
			licenseValid: "<?php echo $licence_valid; ?>"
		};
	</script>
	
	<?php
		
	$logged_in  = $this->ion_auth->logged_in();
	$is_admin   = $this->ion_auth->is_admin();
	$can_access = false;
	
	if($is_admin) {
		$can_access = true;
	} else {
		$this->acl->init_current_acl();
		$can_access = $this->acl->has_perm('Telepath', 'get');
	}
	
	if($licence_valid !== 'VALID') { $logged_in = false; $can_access = false; }

	if($logged_in && $can_access) {
		
	?>
	
	<script src="lib/jquery.jsPlumb-1.5.3-min.js"></script>
	<script src="lib/jquery.vmap.min.js"></script>
	<script src="lib/jquery.vmap.world.js"></script>
    <script src="lib/countdown/jquery.countdown.js"></script>
	    
	<script type="text/javascript" src="<?php echo $_SERVER['PHP_SELF']; ?>/resources"></script>
    <script type="text/javascript" src="app.js"></script>
	
	<link rel="stylesheet" href="lib/countdown/jquery.countdown.css" />
	<link rel="stylesheet" href="css/style.css">
	<link rel="stylesheet" href="css/apptree.css">
	<link rel="stylesheet" href="css/flags.css">
	<link rel="stylesheet" href="css/widgets.css">

	<script src="js/CountriesLookup.js"></script>
	
	<script src="js/telepath.handlers.js"></script>
	<script src="js/telepath.parameters.js"></script>
	<script src="js/telepath.ipconfig.js"></script>
	<script src="js/telepath.applications.js"></script>
	<script src="js/telepath.util.js"></script>
	<script src="js/telepath.apptree.js"></script>
	<script src="js/telepath.apptree.handlers.js"></script>
	
	<script src="js/telepath.dashboard.util.js"></script>
	<script src="js/telepath.dashboard.js"></script>
	<script src="js/telepath.realtime.js"></script>
		
    <script src="js/telepath.investigate.js"></script>
	<script src="js/telepath.sessionflow.js"></script>
    <script src="js/telepath.alerts.js"></script>
	
	<script src="js/teleSelect.js"></script>
	
	<script src="js/telepath.main.panels.js"></script>
	<script src="js/telepath.main.js"></script>

	<?php
		
	// Load Admin Script
	if($this->ion_auth->is_admin()) {
		echo '<script src="js/telepath.admin.js"></script>';
	}
		
	}
	else
	{
		
	if(!$logged_in) {
		
	
	?>
	
	<style>
		.password-reset { float: right; margin-top: 2px; }
		#login-msg p { padding: 0px; margin: 0px; }
	</style>
	
	<script src="js/telepath.license.js"></script>
	<script src="js/telepath.login.js"></script>
	<link rel="stylesheet" href="css/reset.css">
	<link rel="stylesheet" href="css/login.css">
	<script src="js/telepath.checkbox.js"></script>
	<script src="js/telepath.login.js"></script>	
	
	<?php
	
	}
	
	}
	
	?>
	
</head>
<body>

<?php if(!$logged_in) { ?>

<div class="tele-container">
	<img class="tele-logo" src="images/logo.jpg" alt="Hybrid Telepath" />
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
		
		<div class="tele-field">
			<label class="tele-label" for="login-remember">Remember Me:</label>
			<div id="login-remember"></div>
		</div>
		
		<center>
			<div class="tele-button" id="login-button">Login</div>
		</center>
		
	</div>
</div>
<?php } else { ?>
<div id="telepath-main">
<?php
	if($can_access) {
		echo '<div id="telepath-head"><div class="telepath-toolbar"></div></div>';
	} else {
		echo '<div style="padding: 20px"><img src="images/logo.png"><br>';
		echo '<div style="padding: 0px 70px">Please ask the administrator to configure permissions for this account.';
		echo '&nbsp;<a class="logout" href="#">Logout</a></div></div>';
?>
	<script>
		$(document).ready(function () { 
			$('.logout').click(function () { 
				$.getJSON(telepath.controllerPath + '/auth/logout', function(data) { 
					location.reload(true); 
				});
			});
		});
	</script>
<?php } } ?>
</div>
</body>
</html>
