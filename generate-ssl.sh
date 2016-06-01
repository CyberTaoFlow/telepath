#!/usr/bin/env bash
if [ -f /etc/apache2/apache2.conf ]; then
	# Specify where we will install
	SSL_DIR="/etc/apache2/ssl/telepath";

	# Set the wildcarded domain
	# we want to use
	DOMAIN="localhost"

	# A blank passphrase
	PASSPHRASE=""

	# Set our CSR variables
	SUBJ="
C=US
ST=NY
O=NY
localityName=Telepath
commonName=$DOMAIN
organizationalUnitName=Hybrid Security
emailAddress=info@hybridsec.com
"

	# Create our SSL directory
	# in case it doesn't exist
	sudo mkdir -p "$SSL_DIR"

	# Generate our Private Key, CSR and Certificate
	sudo openssl genrsa -out "$SSL_DIR/telepath.key" 2048;
	sudo openssl req -new -subj "$(echo -n "$SUBJ" | tr "\n" "/")" -key "$SSL_DIR/telepath.key" -out "$SSL_DIR/telepath.csr" -passin pass:$PASSPHRASE;
	sudo openssl x509 -req -days 365 -in "$SSL_DIR/telepath.csr" -signkey "$SSL_DIR/telepath.key" -out "$SSL_DIR/telepath.crt";

	# Adding Virtual Host block to /etc/apache2/apache2.conf
	block="\n<VirtualHost *:443>\n\tServerName localhost\n\tDocumentRoot /var/www\n\tSSLEngine on\n\tSSLCertificateFile $SSL_DIR/telepath.crt\n\tSSLCertificateKeyFile $SSL_DIR/telepath.key\n\t# And some extras, copied from Apache\'s default SSL conf virtualhost\n\t<FilesMatch \"\.(cgi|shtml|phtml|php)$\">\n\t\tSSLOptions +StdEnvVars\n\t</FilesMatch>\n\tBrowserMatch \"MSIE [2-6]\" nokeepalive ssl-unclean-shutdown downgrade-1.0 force-response-1.0\n\t# MSIE 7 and newer should be able to use keepalive\n\tBrowserMatch \"MSIE [17-9]\" ssl-unclean-shutdown\n</VirtualHost>"
	if ! grep -q "<VirtualHost " /etc/apache2/apache2.conf ; then
		echo -e $block >> /etc/apache2/apache2.conf
		# Enabling mode ssl
		sudo a2enmod ssl

		# Restart apache2
		sudo service apache2 restart
	else
		echo ">>>> SSL already configured in apache2.conf";
	fi
fi
if [ -f /etc/httpd/conf/httpd.conf ]; then
        # Specify where we will install
        SSL_DIR="/etc/httpd/ssl/telepath";

        # Set the wildcarded domain
        # we want to use
        DOMAIN="localhost"

        # A blank passphrase
        PASSPHRASE=""

        # Set our CSR variables
        SUBJ="
C=US
ST=NY
O=NY
localityName=Telepath
commonName=$DOMAIN
organizationalUnitName=Hybrid Security
emailAddress=info@hybridsec.com
"

        # Create our SSL directory
        # in case it doesn't exist
        sudo mkdir -p "$SSL_DIR"

        # Generate our Private Key, CSR and Certificate
        sudo openssl genrsa -out "$SSL_DIR/telepath.key" 2048
        sudo openssl req -new -subj "$(echo -n "$SUBJ" | tr "\n" "/")" -key "$SSL_DIR/telepath.key" -out "$SSL_DIR/telepath.csr" -passin pass:$PASSPHRASE
        sudo openssl x509 -req -days 365 -in "$SSL_DIR/telepath.csr" -signkey "$SSL_DIR/telepath.key" -out "$SSL_DIR/telepath.crt"

        # Adding Virtual Host block to /etc/apache2/apache2.conf
        block="\n<VirtualHost *:443>\n\tServerName localhost\n\tDocumentRoot /var/www\n\tSSLEngine on\n\tSSLCertificateFile $SSL_DIR/telepath.crt\n\tSSLCertificateKeyFile $SSL_DIR/telepath.key\n\t# And some extras, copied from Apache\'s default SSL conf virtualhost\n\t<FilesMatch \"\.(cgi|shtml|phtml|php)$\">\n\t\tSSLOptions +StdEnvVars\n\t</FilesMatch>\n\tBrowserMatch \"MSIE [2-6]\" nokeepalive ssl-unclean-shutdown downgrade-1.0 force-response-1.0\n\t# MSIE 7 and newer should be able to use keepalive\n\tBrowserMatch \"MSIE [17-9]\" ssl-unclean-shutdown\n</VirtualHost>"
        if ! grep -q "<VirtualHost " /etc/httpd/conf/httpd.conf ; then
                echo -e $block >> /etc/httpd/conf/httpd.conf
                # Enabling mode ssl
                #sudo a2enmod ssl
		sudo yum install -y mod_ssl

                # Restart apache2
                sudo service httpd restart
        else
                echo ">>>> SSL already configured in httpd.conf";
        fi
fi

echo ">>> DONE <<<"
