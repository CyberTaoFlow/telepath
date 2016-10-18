echo "---------------------------------------------------"
echo "DOWNLOADING AND COMPILING NGINX (Openresty edition)"
echo "---------------------------------------------------"

rm -rf /tmp/openresty
mkdir /tmp/openresty
cd /tmp/openresty

apt-get install --force-yes -y libreadline-dev libncurses5-dev libpcre3-dev libssl-dev perl make php5-fpm
yum install readline-devel pcre-devel openssl-devel php php-mysql php-fpm -y

useradd www-data

sed -i 's/^listen = .*/listen = \/var\/run\/php5-fpm\.sock/' /etc/php-fpm.d/www.conf
sed -i 's/^user = .*/user = www-data/' /etc/php-fpm.d/www.conf
sed -i 's/^group = .*/group = www-data/' /etc/php-fpm.d/www.conf

systemctl start php-fpm
systemctl enable php-fpm.service

wget http://openresty.org/download/ngx_openresty-1.7.7.2.tar.gz
tar -xf ngx_openresty-1.7.7.2.tar.gz
cd ngx_openresty-1.7.7.2/

./configure --prefix=/opt/telepath/openresty --with-http_sub_module

make
make install

sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout /opt/telepath/ui/ssl.key -out /opt/telepath/ui/ssl.crt -subj "/O=HybridSec/CN=hybridsec.com"

chmod -R 777 /opt/telepath/openresty/nginx/certs/
mv /opt/telepath/openresty/nginx/lua/telepath.conf.default /opt/telepath/openresty/nginx/conf/nginx.conf
chmod -R 777 /opt/telepath/openresty/nginx/conf/nginx.conf
mv /opt/telepath/openresty/nginx/html/index.html  /opt/telepath/openresty/nginx/html/index_old.html

service apache2 stop
service httpd stop
chkconfig httpd off

# Allow firewall access to http, https and telepath ui port
firewall-cmd --zone=public --add-port=80/tcp --permanent
firewall-cmd --zone=public --add-port=8080/tcp --permanent
firewall-cmd --zone=public --add-port=443/tcp --permanent
firewall-cmd --reload

iptables -A INPUT -p tcp --dport 80 -j ACCEPT
iptables -A INPUT -p tcp --dport 8080 -j ACCEPT
iptables -A INPUT -p tcp --dport 443 -j ACCEPT

ln -sr /opt/telepath/ui/html/* /opt/telepath/openresty/nginx/html

/opt/telepath/openresty/nginx/sbin/nginx

echo "DONE!"

