<?php
	header('Cache-Control: no-cache');
	header('Pragma: no-cache');
?>
<html>
<head>
<link rel="icon" href="data:;base64,iVBORw0KGgo=">
<style>

html, body, div, span {
	margin: 0;
	padding: 0;
	border: 0;
	font-size: 100%;
	font: inherit;
	vertical-align: baseline;
}

body {
	background-color: #E6E6E6;
	text-align: center;
	font-family: Helvetica, sans-serif;
}

.tele-container {
	margin: auto auto;
	text-align: left;
	width: 700px;
	background-color: #FFFFFF;
	min-height: 670px;
}
.tele-captcha {
  margin: 40px 0px 0px 0px;
  border-top: 1px solid #cecece;
  padding: 40px 30px 0px 30px;
  position: relative;
}


.tele-field {
	height: 50px;
	clear: both;
	width: 300px;
}

.tele-label {
	width: 120px;
	float: left;
	color: #555555;
	margin-top: 7px;
}

.tele-input {
	  border: 1px solid #555555;
	  border-radius: 20px;
	  margin-top: 0;
	  padding: 5px 5px 5px 10px;
	  width: 170px;
}

.tele-button {

	margin-top: 30px;
	-moz-box-shadow:inset 0px -4px 0px 0px #dd7d1d;
	-webkit-box-shadow:inset 0px -4px 0px 0px #dd7d1d;
	box-shadow:inset 0px -4px 0px 0px #dd7d1d;
	background-color:#f7921e;
	-webkit-border-top-left-radius:5px;
	-moz-border-radius-topleft:5px;
	border-top-left-radius:5px;
	-webkit-border-top-right-radius:5px;
	-moz-border-radius-topright:5px;
	border-top-right-radius:5px;
	-webkit-border-bottom-right-radius:5px;
	-moz-border-radius-bottomright:5px;
	border-bottom-right-radius:5px;
	-webkit-border-bottom-left-radius:5px;
	-moz-border-radius-bottomleft:5px;
	border-bottom-left-radius:5px;
	text-indent:0;
	border:1px solid #ffffff;
	display:inline-block;
	color:#ffffff;
	font-family:Arial;
	font-size:17px;
	font-weight:normal;
	font-style:normal;
	text-decoration:none;
	text-align:center;
	text-shadow:1px 1px 0px #dd7d1d;
	padding: 15px 35px;
	cursor: pointer;
}

</style>

	<title>Access Denied</title>
</head>
<div class="tele-container">
<img style="margin: 65px 20px 0px 20px;" src="data:image/png;base64,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"/>
<center>

<div class="tele-captcha">
<h3>Due to suspicious behaviour your access to this website was blocked.</h3>
<h4>Please resolve the following captcha to restore access</h4>
<?php
// generate captcha image - returns image handle
function captcha_image($secret, $img_width=200, $img_height=50) {
  // seed srand
  srand((double)microtime()*1000000);

  // create image
  $im = @imagecreate($img_width, $img_height) or die("Cannot Initialize new GD image stream");

  // define font
  $font = "/usr/fonts/ttf/Georgia.ttf";
  $font = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

  // create some colors
  $black = imagecolorallocate($im, 0, 0, 0);
  $white = imagecolorallocate($im, 255, 255, 255);
  $grey = imagecolorallocate($im, 128, 128, 128);

  // randomness, we need lots of randomness :)
  // background color -> 1=black, 2=white, 3=grey (more colors can be added)
  // lines -> black bg (1=white or 2=grey), white bg (1=black or 2=grey), grey bg (black only)
  $bgcolor = $black;
  $fontcolor = $white;
  $linecolor = $black;


  // line positioning and increment
  $x_start = rand(0, 10);
  $x_size = rand(5, 10);
  $y_start = rand(0, 10);
  $y_size = rand(5, 10);

  // fill with background color
  imagefill($im, 0, 0, $bgcolor);

  // initial x position
  $font_x = 10;

  // write text
  for ($i = 0; $i < strlen($secret); $i++) {
    // font size -> 20 to 35
    $font_size = rand(25, 35);
    // font angle -> -20 to +20
    $font_angle = rand(0, 20);
    if ($font_angle != 0) { if (rand(0, 1) == 0) { $font_angle = -$font_angle; } }
    // font y position -> if font_size <= 27 then 30 to 35, if font_size > 27 then 30 to 35
    if ($font_size <= 27) { $font_y = rand(25, 30); } else { $font_y = rand(30, 35); }
    // write the text
    imagettftext($im, $font_size, $font_angle, $font_x, $font_y+4, $fontcolor, $font, $secret{$i});
    // one more time to make it bolder
    imagettftext($im, $font_size, $font_angle, $font_x+1, $font_y+5, $fontcolor, $font, $secret{$i});
    // next font x position
    $font_x += ($font_size + 5);
  }

  // draw horizontal lines
  for ($y = $y_start; $y < $img_height; $y += $y_size) {
    imageline($im, 0, $y, $img_width, $y, $linecolor);
  }
  // draw vertical lines
  for ($x = $x_start; $x < $img_width; $x += $x_size) {
    imageline($im, $x, 0, $x, $img_height, $linecolor);
  }

  // return captcha image handle
  return $im;
}

function secret_key($length=5) {
  $salt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
  srand((double)microtime()*1000000);
  $i = 0;
  $skey = "";
  while ($i < $length) {
    $num = rand() % strlen($salt);
    $tmp = substr($salt, $num, 1);
    $skey .= $tmp;
    $i++;
  }
  return $skey;
}


// set headers
// generate secret
//$skey = secret_key();
$skey = $argv[1];
// create captcha and output to browser as PNG image
$im = captcha_image($skey);
ob_start();
imagepng($im);
$data = ob_get_clean();
imagedestroy($im);

$type = 'png';
$captcha_base64 = '<img src="data:image/' . $type . ';base64,' . base64_encode($data) . '">';

echo $captcha_base64;
//@imagepng($im);
//@
?>
</div>
<br><br>
<form method="POST">
<div class="tele-field">
	<label class="tele-label" for="login-username">Captcha:</label>
	<input type="text" class="tele-input" name="captcha">
</div>
<input type="submit" class="tele-button" id="login-button" value="Continue"></input>
</form>
</center>



</div>

