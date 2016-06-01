user www-data;
worker_processes 8;
worker_rlimit_nofile 200000;

events {
    worker_connections  4096;
    multi_accept        on;
    use                 epoll;
}

http {

<?php

	foreach($apps as $app) {
		
		echo '	upstream application_' . md5($app['host']) . '_upstream {' . "\n";
		
		foreach($app['app_ips'] as $ip) {
			echo '		server ' . $ip . ':' . (intval($app['ssl_flag']) == 1 ? $app['ssl_server_port'] : '80') . ';' . "\n";
		}
		
		echo '	}' . "\n";
		
	}

?>

	sendfile        on;
	keepalive_timeout  15;
	gzip  on;
	init_by_lua_file /opt/telepath/openresty/nginx/lua/init.lua;
	
	# Management port
	server {
	
		include    mime.types;
		autoindex on;
		index index.php;
		listen 8080 ssl;
		ssl_certificate     /opt/telepath/ui/ssl.crt;
                ssl_certificate_key /opt/telepath/ui/ssl.key;

		
		
		location / {
			try_files $uri $uri/ /index.php;
		}
	 
		location ~ \.php($|/) {
			fastcgi_pass unix:/var/run/php5-fpm.sock;
			fastcgi_split_path_info ^(.+\.php)(.*)$;
			include fastcgi_params;
			fastcgi_param  SCRIPT_FILENAME  $document_root$fastcgi_script_name;
		}
		
	}

	# Reverse proxy(ies)

<?php
	
	foreach($apps as $app) {
		
		echo '	server {' . "\n\n";
		
		echo '		server_name ' . $app['host'] . ';'  . "\n";
		
		if(intval($app['ssl_flag']) == 1) {
		
			echo '		listen ' . $app['ssl_server_port'] . ' ssl;' . "\n";
			echo '		ssl_certificate ' . $certs_dir . 'application_' . md5($app['host']) . '_certificate.crt'  . ";\n";
			echo '		ssl_certificate_key ' . $certs_dir . 'application_' . md5($app['host']) . '_private_key.key'  . ";\n";

		} else {
			echo '		listen 80;';
		}
		
?>		

		location / {

			proxy_read_timeout		90;
			proxy_redirect			off;
			proxy_set_header		Host            $http_host;
			proxy_set_header		X-Real-IP       $remote_addr;
			proxy_set_header		X-Forwared-For  $proxy_add_x_forwarded_for;
			proxy_set_header		Accept-Encoding '';
			
<?php
	echo '			proxy_pass		http' . (intval($app['ssl_flag']) == 1 ? 's' : '') . '://application_' . md5($app['host']) . '_upstream;';
?>


			lua_need_request_body on;
			
			header_filter_by_lua_file /opt/telepath/openresty/nginx/lua/header_filter.lua;
			body_filter_by_lua_file   /opt/telepath/openresty/nginx/lua/body_filter.lua;
			access_by_lua_file        /opt/telepath/openresty/nginx/lua/access.lua;
			log_by_lua_file           /opt/telepath/openresty/nginx/lua/log.lua;
			
		}

		error_page   500 502 503 504  /50x.html;
		location = /50x.html {
			root   html;
		}
		
		location ~ /\.ht {
			deny  all;
		}
		
	}

<?php
}
?>
	
}

