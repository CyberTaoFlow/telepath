<?php
defined('BASEPATH') OR exit('No direct script access allowed');

$is_admin = $this->ion_auth->is_admin();
$perms = array();

if (!$is_admin) {
    $this->acl->init_current_acl();
    $tmp = $this->acl->perms;

    if (!empty($tmp)) {
        foreach ($tmp as $k => $v) {
            $perms[str_replace('::', '_', $k)] = true;
        }
    }

}

/*
$perms = array();
$master_list = $this->acl->get_master_list();
foreach($master_list as $cat => $items) {
	foreach($items as $item => $desc) {
		$perms[$cat . '_' . $item] = true;
	}
}
*/
/*
$can_access = false;

if($is_admin) {
	$can_access = true;
} else {
	$this->acl->init_current_acl();
	$can_access = $this->acl->has_perm('Telepath', 'get');
}

if($licence_valid !== 'VALID') { $logged_in = false; $can_access = false; }

if(!$logged_in) {
	echo '123';
	die;
}

if($logged_in && $can_access) {
	echo '1010101';
}
*/

?>
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <title>Telepath 3.0</title>
    <link rel='shortcut icon' href='/favicon.ico'/>
    <link rel='icon' href='/favicon.ico'/>

    <?php

 //   if ($_SERVER['HTTP_HOST'] == 'localhost') {
        $css_files = array('reset.css', 'ui-lightness/jquery-ui-1.10.4.custom.min.css', 'telepath.css', 'listitem.css', 'infoblock.css', "icons.css", "flags.css", "overlay.css", "jquery.contextmenu.css", "slider.css", "tipsy.css");
        // add css files
       // $this->minify->css($css_files);

        // bool argument for rebuild css (false means skip rebuilding).
       // $this->minify->deploy_css(true);

        foreach ($css_files as $src) {
            echo '<link rel="stylesheet" href="css/' . $src . '">';
        }
//    } else {
//        echo '<link rel="stylesheet" href="css/styles.min.css">';
//    }

    ?>


    <script>
        var telepath = {
            basePath: "<?php echo dirname($_SERVER['PHP_SELF']) != '/' ? dirname($_SERVER['PHP_SELF']) : ''; ?>",
            controllerPath: "<?php echo $_SERVER['PHP_SELF']; ?>",
            licenseValid: "VALID",
            access: <?php echo json_encode(array('admin' => $is_admin, 'perm' => $perms)) ?>
        };
    </script>

    <?php
 //   if ($_SERVER['HTTP_HOST'] == 'localhost') {

        $js_files = array(
            "lib/jquery-1.11.0.min.js",
            "lib/jquery-ui-1.10.4.custom.min.js",
            "lib/yepnope.1.5.4-min.js",
            "lib/jquery.contextmenu.min.js",
            "lib/jquery.fileupload.min.js",
            "lib/jquery.iframe-transport.min.js",
            "lib/jquery.flot.min.js",
            "lib/jquery.flot.resize.min.js",
            "lib/jquery.flot.pie.min.js",
            "lib/jquery.flot.selection.min.js",
            "lib/jquery.flot.time.min.js",
            "lib/jquery.flot.axislabels.min.js",
            "lib/bootstrap-slider.min.js",
            "lib/jquery.flot.symbol.min.js",
            "lib/jquery.flot.tooltip.min.js",
            "lib/jquery.tipsy.min.js"
        );

        array_push($js_files, "telepath.js", "telepath.header.js", "telepath.utils.js", "telepath.ds.js", "telepath.infoblock.js", "telepath.popup.js", "telepath.countries.js", "telepath.dropdown.js", "telepath.radios.js", "telepath.button.js", "telepath.checkbox.js", "telepath.search.js", "telepath.listitem.js", "telepath.listitem.generic.js", "telepath.toggle.js", "telepath.daterange.js", "telepath.graph.js", "telepath.vectormap.js", "telepath.anomalyscore.js", "telepath.anomalyscore.js", "telepath.anomalyscore.js", "telepath.notifications.js", "telepath.notifications.js", "telepath.overlay.js", "telepath.pagination.js", "telepath.config.js", "telepath.dashboard.js", "telepath.case.js", "telepath.cases.js", "telepath.alert.js", "telepath.alerts.js", "telepath.suspects.js", "telepath.reports.js");

        // add js files
      //  $this->minify->js($js_files);

        // rebuild js (false means skip rebuilding).
     //   $this->minify->deploy_js(false);

        foreach ($js_files as $src) {
            echo '<script src="js/' . $src . '"></script>';
        }
//    } else {
//        echo '<script src="js/scripts.min.js"></script>';
//    }
    ?>


</head>

<body>

</body>
</html>
