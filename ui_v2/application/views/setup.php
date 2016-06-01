<html>
<head>

<script src="lib/jquery-1.9.1.min.js"></script>
<link rel="stylesheet" href="css/setup.css">
	
	<script>
	
	function formatDate(date, fmt) {
	
		function pad(value) {
			return (value.toString().length < 2) ? '0' + value : value;
		}
		return fmt.replace(/%([a-zA-Z])/g, function (_, fmtCode) {
			switch (fmtCode) {
			case 'Y':
				return date.getUTCFullYear();
			case 'M':
				return pad(date.getUTCMonth() + 1);
			case 'd':
				return pad(date.getUTCDate());
			case 'H':
				return pad(date.getUTCHours());
			case 'm':
				return pad(date.getUTCMinutes());
			case 's':
				return pad(date.getUTCSeconds());
			default:
				throw new Error('Unsupported format code: ' + fmtCode);
			}
		});
	
	};
	
	function bytesToSize(bytes) {
		
		var sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
		if (bytes == 0) return '0 Bytes';
		var i = parseInt(Math.floor(Math.log(bytes) / Math.log(1024)));
		return Math.round(bytes / Math.pow(1024, i), 2) + ' ' + sizes[i];
		
	};
	
	function log(text, noBR, noTIME) {
			$('#prog-log').append((noTIME ? '&nbsp;' : (formatDate(new Date(), '%H:%m:%s') + '&nbsp;::&nbsp;')) + text + (noBR ? '' : '<br>'));
			$('#prog-log').scrollTop($("#prog-log").prop('scrollHeight'));
	};
	
	var telepath = { 
		basePath: "<?php echo dirname($_SERVER['PHP_SELF']) != '/' ? dirname($_SERVER['PHP_SELF']) : ''; ?>",
		controllerPath: "<?php echo $_SERVER['PHP_SELF']; ?>",
		licenseValid: "<?php echo $licence_valid; ?>",
		status: 0,
		log: [],
	};
	
	$(function() { //shorthand document.ready function
		$('#configure_db').on('submit', function(e) { //use on if jQuery 1.7+
			
			$('.form-error').empty().hide();
		
			e.preventDefault();
			var data = $("#configure_db :input").serializeArray();
			
			$('#prog-bar-inner').css({ width: '0%' });
			telepath.tick = 0;
			
			telepath.timer = setInterval(function () {
				
				// Start polling for update prog
				$.getJSON(telepath.controllerPath + '/setup', { action: 'status', offset: telepath.log.length }, function(data) {
					
					telepath.log = data;
					
					// Print
					$.each(data, function(i, row) {
					
						try {
					
						row = JSON.parse(row);
						
						if(row.stop) {
							if(telepath.timer) {
								clearInterval(telepath.timer);
							}
						}
						
						if(row.index && row.total) {
							log('(' + row.index + '/' + row.total + ')&nbsp;' + row.msg);
							var percentage = Math.floor((row.index / row.total) * 100);
							console.log(percentage);
							$('#prog-bar-inner').css({ width: percentage + '%' });
						} else {
							log(row.msg);
						}
						
						
						
												
						} catch(e) {
						
						}
						
					});
					
				});
			
			}, 500);
			
			$.post(telepath.controllerPath + '/setup', data, function(data) {
				
				setTimeout(function () {
				
				if(telepath.timer) {
					clearInterval(telepath.timer);
				}
				
				}, 3000);
				
			}).fail(function(x,y) {
			
				setTimeout(function () {
				
				if(telepath.timer) {
					clearInterval(telepath.timer);
				}
				
				}, 3000);
			
				try {
				
					var obj = JSON.parse(x.responseText);
					$('.form-error').html(obj.message).show();
					
				} catch(e) {
					
					
					
				}
				
			});
		});
	});

</script>

<title>Hybrid Telepath</title>

</head>
<body>
	
	
	
	<img src="images/logo_big.png" alt="Hybrid Security">
	<br>
	
	<div id="container">
		<h1>Telepath Configuration</h1>
		<p>Please configure telepath database</p>
		
		<form id="configure_db" method="POST" action="index.php/setup">
			
			<div class="form-error">
			</div>
			
			<input type="hidden" name="action" value="update_db">
			
			<div class="form-element">
				<label for="db_host">Database Host</label>
				<input name="db_host" type="text" size="20" value="localhost" />
			</div>
			<div class="form-element">
				<label for="db_host">Database Port</label>
				<input name="db_port" type="text" size="20" value="3306" />
			</div>
			<div class="form-element">
				<label for="db_host">Database User</label>
				<input name="db_user" type="text" size="20" value="root" />
			</div>
			<div class="form-element">
				<label for="db_host">Database Pass</label>
				<input name="db_pass" type="text" size="20" value="" />
			</div>
			
			<div class="form-element">
				<input type="submit" value="Create / Update Database Structure">
			</div>
			
			<div class="clear"></div>
			
		</form>
		
		<div id="prog-bar"><div id="prog-bar-inner"></div></div>
		
		<div id="prog-log" style="color: green;"></div>
		

	</div>
</body>
</html>