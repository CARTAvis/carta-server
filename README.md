# carta-server
Components used to start CARTA from a web server. These include tools to maintain a user database needed by the CARTA backend when it is started in this way.

Still need to add `cmake` files, but have been compiling the `cartadbctl` program with:
```
g++ -g -I/usr/include/libbson-1.0 -I/usr/include/libmongoc-1.0/ cartadbctl.cc -o cartadbctl -lmongoc-1.0 -lbson-1.0 -ljsoncpp> err.txt 2>&1
```

Paths in scripts currently assume that `carta_run_local.sh` and `carta_backend` are both placed in `/usr/local/CARTA/bin`.

On ubuntu you will need the following packages:
```
libmongoc-dev
libbson-dev
mongodb
mongo-tools
apache2
apache2-suexec-custom
```

Adding a system user to the database. This creates the security token needed to access the `carta_backend` server.
```
./cartadbctl -u simmonds
WARNING : This program is under construction and not all the options have been implemented yet
User = simmonds
 done ...
```

Setting up apache:
```
apt install libapache2-mod-authnz-pam libcgi-session-perl libjson-perl libmongodb-perl

cd /etc/apache2/sites-enabled
```

remove the default site
```
cp ../sites-available/default-ssl.conf
```

Then add these lines to the top of the conf file before the other `VirtualHost` clause. You will need to set the correct hostname.
```
<VirtualHost *:80>
          ServerName carta-dev.idia.ac.za
          Redirect / https://carta-dev.idia.ac.za
          </Virtualhost>
```

And replace the certificate files with site specific ones.
```
    SSLCertificateFile   /etc/apache2/certs/idia-wldcrd.pem
    SSLCertificateKeyFile /etc/apache2/certs/idia-wldcrd.key
```

```
cd /etc/apache2/mods-enabled
ln -s  ../mods-available/proxy_wstunnel.load .
ln -s ../mods-available/ssl.* .
ln -s ../mods-available/socache_shmcb.load .
ln -s ../mods-available/proxy.* .
ln -s ../mods-available/userdir.* .
ln -s ../mods-available/suexec.load .
ln -s ../mods-available/cgi.load .
ln -s ../mods-available/session.load .
ln -s ../mods-available/session_c
ln -s ../mods-available/session_cookie.load
ln -s ../mods-available/session_crypto.load  
ln -s ../mods-available/proxy_html.* .
ln -s ../mods-available/proxy_http2.* .
ln -s ../mods-available/proxy_http.* .
ln -s ../mods-available/rewrite.load .
```

Edit userdir.conf to add this:
```
        <Directory /users/*/public_html/cgi-bin>
                Require method GET POST OPTIONS
                Options ExecCGI
                SetHandler cgi-script
        </Directory>
```

If you are using apache as WS reverse proxy:
```
cd /etc/apache2/conf-enabled
~ubuntu/Build/carta-server/scripts/addsockets.pl 3010 3110 > proxy_wstunnel.conf
```

You can use nginx if you want but you do need a range of ports setup.

Users have to have a `carta.pl` script in their `cgi-bin` dir for `suexec` to work. The script `addcartauser.pl` is provided to copy the script. You may need to modify this script to work with your local home directory structures.

```
cd /var/www/html
cp -r /home/ubuntu/Build/carta-frontend/build carta
cp -r /home/ubuntu/Build/carta-server/auth .
```
