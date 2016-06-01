<img src="images/logo.png" alt="HybridSec Telepath" /><br><br>
<?php
	
	// TODO:: connect to codeigniter auth, also allow to work without DB (somehow)
	echo 'Please edit the file to allow updates';
	return;
	
	function download($VER) {
		
		$url  = 'https://hybridsec.com/updates/ui_update_' . intval($VER) . '.tar';
		$data = _simple_curl($url);
		$file = realpath('.') . '/files/ui_update_' . intval($VER) . '.tar';
		
		file_put_contents($file, $data); 
		
		echo 'Done, received ' . strlen($data) . ' bytes from ' . $url . ' updating, please wait ';
			
	}
	
	function do_update($VER, $UI) {
		
		download($VER);
		
		$shell = 'sudo /opt/telepath/bin/telepath update ' . $VER . ' ' . $UI;
		echo shell_exec ( $shell );
		
		?>
		<span id="seconds">10</span> seconds, the page will reload automatically.
		
		<script>
		
			var val = 10;
			
			function tick() {
				
				val = val - 1;
				if(val > -1) {
					setTimeout(tick, 1000);
					document.getElementById("seconds").innerHTML = val;
				} else {
					document.location.href = '/telepath/';
				}
			
			}
			
            setTimeout(tick, 1000);

		</script>
		<?php
		
		//exec($shell);
		die;
	
	}
	
	if(isset($_REQUEST['REV']) && isset($_REQUEST['REV'])) {
	
		$target_REV = $_REQUEST['REV'];
		$target_UI  = $_REQUEST['UI'];
		
		if(isset($_REQUEST['confirm']) && $_REQUEST['confirm'] == 'yes') {
			
			do_update($target_REV, $target_UI);
			die;
		
		} else {
				
			echo 'Are you sure you want to update to UI version ' . $target_UI . ' revision ' . $target_REV . ' ?<br>';
			echo "<br><a style='color: green; font-size: 18px;' href='updater.php?REV=$target_REV&UI=$target_UI&confirm=yes'>Update!</a><br><br>";
			echo '<a style="color: red; font-size: 14px;" href="updater.php">Cancel</a><br>';
			die;
			
		}
	}
	
	$versions = _simple_curl('https://hybridsec.com/updates/ui_updates.php');
	if($versions != '') {
		$versions = json_decode($versions, true);
		if(!$versions || empty($versions)) {
			echo 'Malformed updates version list';
			die;
		} else {
			echo 'Avaliable versions:';		
		}
	} else {
		
		echo 'Count not receive UI updates version list'; 
		die;
	
	}
	
	function _simple_curl($url, $proxy = false) {
		
		// is cURL installed yet?
		if (!function_exists('curl_init')) {
			die('Sorry cURL is not installed!');
		}
	 
		$ch = curl_init();
		
		if($proxy) {
			curl_setopt($ch, CURLOPT_HTTPPROXYTUNNEL, 0);
			curl_setopt($ch, CURLOPT_PROXY, $proxy);
		}
		
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_TIMEOUT, 30);
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false); 
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
		
		$output = curl_exec($ch);
		
		if(curl_errno($ch)) {
			echo 'error:' . curl_error($ch) . "\n";
			return '';
		}
		
		curl_close($ch);
		return $output;
	
	}
	
	echo '<ul>';
	
	foreach($versions as $version) {
		
		$VER = explode('_', $version);
        $VER = substr($VER[2], 0, -4);
		
		echo "<li><a href='updater.php?REV=$VER&UI=2'>Install UI V2 revision " . $VER . '</a></li>';
		echo "<li><a href='updater.php?REV=$VER&UI=3'>Install UI V3 revision " . $VER . '</a></li>';
		
	}
	echo '</ul>';

?>

