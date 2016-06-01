<?php

if(php_sapi_name() == 'cli') {
	echo "\n" . $heading . "\n";
	echo strip_tags($message) . "\n" . "\n";
	die;
}

$controller = trim(array_pop(explode('/', $_SERVER['REQUEST_URI'])));
if($controller == 'setup') {
	echo json_encode(array('heading' => $heading, 'message' => $message));
	die;
}

?>
<!DOCTYPE html>

<?php

if(!isset($heading)) {
	$heading = '';
}
if(!isset($message)) {
	$message = '';
}

?>

<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <title>Hybrid Telepath</title>
	
	<link rel="stylesheet" href="extjs/resources/css/ext-all-gray.css">
	<script src="extjs/ext42-all.js"></script>
	<script src="lib/jquery-1.9.1.min.js"></script>
	<script src="lib/yepnope.js"></script>
	
	<script>
		var telepath = { 
			basePath: "<?php echo dirname($_SERVER['PHP_SELF']) != '/' ? dirname($_SERVER['PHP_SELF']) : ''; ?>",
			controllerPath: "<?php echo $_SERVER['PHP_SELF']; ?>"
		};
	</script>
	
	<script src="js/telepath.trial.js"></script>
	
</head>
<body>

	<div id="container">
		<h1><?php echo $heading; ?></h1>
		<?php echo $message; ?>
	</div>
	
</body>
</html>
