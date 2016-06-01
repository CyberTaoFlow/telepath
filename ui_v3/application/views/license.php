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
	<!-- <script src="js/telepath.login.js"></script> -->
	<script src="js/telepath.license.js"></script>

</head>

<body>
	<div class="tele-container-license">
		<img class="tele-logo" src="img/logo_big.png" alt="Hybrid Telepath" />
		<div class="tele-license">
		
			<h1>License Management</h1>
			
			<div class="tele-error" id="license-error"></div>
			<center>
			<div class="tele-field">
				<label class="tele-label-license" for="license-number">Please provide a license number:</label>
				<input type="text" class="tele-input-license" id="license-number" name="license-number" placeholder="XXXXXXXX-XXXX-XXXX-TPTH-XXXXXXXXXXXX">
			</div>
			<center>
			
			<br>
			
			<center>
				<div class="tele-button" id="validate-button">Validate</div>
			</center>
			
		</div>
	</div>
</body>
</html>