These are instructions to set up a fully working carta-server version on an Ubuntu 18.04 server from scratch. It also involves building carta-backend and carta-frontend from source.
Note: Some modifications may still be needed for your system.

 # Part 0. Install all required packages:
 This step should install all required packages on the system to build the carta-backend, carta-frontend, and carta-server.
```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install -y bison build-essential byobu cmake curl default-jre emacs \
fftw3-dev flex gdb gcc gfortran git git-lfs htop libblas-dev libcfitsio-dev libfmt-dev \ 
libgtest-dev libhdf5-dev liblapack-dev libncurses-dev libprotobuf-dev libreadline-dev \
libssl-dev libstarlink-ast-dev libtbb-dev man protobuf-compiler python-pip python3-pip \
software-properties-common unzip vim wcslib-dev wget libgsl-dev libxml2-dev libxslt1-dev \
libjsoncpp-dev libmongoc-dev libbson-dev mongodb mongo-tools apache2 apache2-suexec-custom \
libapache2-mod-authnz-pam libcgi-session-perl libjson-perl libmongodb-perl php \
libapache2-mod-php php-mongodb php-curl libjson-c-dev libmongoc-dev libssl-dev
```
```
sudo systemctl unmask mongodb
sudo systemctl start mongodb
```
  

# Part 1: Prepare carta-backend

1. Install casacore data
```
sudo mkdir -p /usr/local/share/casacore && cd /usr/local/share/casacore
sudo git clone https://github.com/kernsuite-debian/casacore-data.git data
```

2. Install casacore/casacode Imageanalysis

Easy way: Use our cartavis PPA:
```
sudo add-apt-repository ppa:cartavis/carta-casacore
sudo apt-get update
apt-get install carta-casacore
```
 
4. Install uWS 

Easy way: Use our cartavis PPA
```
sudo add-apt-repository ppa:cartavis/uws
sudo apt-get update
sudo apt-get install uws
```
  
6. Install zfp 

Easy way: Use our cartavis PPA
```
sudo add-apt-repository ppa:cartavis/zfp
sudo apt-get update
sudo apt-get install zfp
```
  
8. Build carta-backend (server version)

Please note the **-DAuthServer=ON** flag. It is needed to enable server mode.
```
git clone https://github.com/CARTAvis/carta-backend.git
cd carta-backend
git submodule init
git submodule update
mkdir build && cd build
cmake .. -DCMAKE_CXX_FLAGS="-I/usr/local/include/casacode -I/usr/local/include/casacore" \
-DCMAKE_CXX_STANDARD_LIBRARIES="-L/usr/local/lib -limageanalysis" \
-DAuthServer=ON
```
The carta-backend should now be ready.

# Part 2: Prepare carta-frontend
1. Install Emscritpen (emsdk)
```
cd ~
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
git pull
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```
  
2. Clone the carta-frontend repository:
```
cd ~
git clone https://github.com/CARTAvis/carta-frontend.git
cd carta-frontend
```
3. Modify the hidden .env file to activate server mode:

Note: It is important to modify the hidden .env file in order to enable server mode on the frontend.
It will set **REACT_APP_AUTHENTICATION=true**
```
sed -i 's/false/true/g' .env
```
  
5. Build the carta-frontend:
```
git submodule init
git submodule update

npm install

cd wasm_libs
./build_libs.sh
cd ..

cd wasm_src
./build_wrappers.sh
cd ..

cd protobuf
./build_proto.sh
cd ..

npm run build
```
The carta-frontend should now be ready.

# Part 3: carta-server

1. Get the carta-server code from GitHub :
```
cd ~
git clone https://github.com/CARTAvis/carta-server.git
```
  
2. Build the **cartadbctl** program. It is used to add users to the Mongo database:
```
cd carta-server/src/dbcontrol
g++ -g -I/usr/include/libbson-1.0 -I/usr/include/libmongoc-1.0/ cartadbctl.cc -o cartadbctl -lmongoc-1.0 -lbson-1.0 -ljsoncpp -lcrypt
```
Two types of users can be added; 
1. Local users with user accounts on the server. 
2. Users from an institute-wide LDAP setup, if the LDAP system is already made accessible from the server. 

(To add local users to the ubuntu system use, **sudo adduser \<username>**. For example **sudo adduser ajm**).

If using local users, but you wish to mount a remote file system from elsewhere, make sure the local usernames have the same UID and GID as they do on the remote file system. 

Alternatively, it can immediately work with user accounts from an LDAP setup if it has that has been previously set up on your system.

To add users simply run 
` ./cartadbctl -u <username>`

You can see available options with **./cartadbctl --help**


# Part 4: Setting up Apache and bringing everything together

  
1. Copy the built carta-frontend from Part 2 into place.
 
Note: In this example we are simply using the the default Apache html location, but that is not necessary.
```
sudo cp -r ~/carta-frontend/build/* /var/www/html/
```
  
3. Put the carta_auth index.php in place:
```
mkdir /var/www/html/carta_auth
cp -r ~/carta-server/scripts/auth/index.php /var/www/html/carta_auth
```

4. Customise **/var/www/html/carta_auth/index.php** in order to set the correct server name/URL for your institute:

Note: In this example our system's DNS name is **`hilo.asiaa.sinica.edu.tw`**. Please adjust accordingly.
```
$suexecstring = "https://hilo.asiaa.sinica.edu.tw/~${username}/cgi-bin/carta.pl?socket=$socket";
```

6. Put the carta-backend executable from Part 1 and the **carta_run_local.sh** script (found in **carta-server/scripts**) in the place where carta-server looks (defined in the **carta-server/carta.pl** script):
```
sudo mkdir -p /usr/local/CARTA/bin
sudo cp ~/carta-backend/build/carta_backend /usr/local/CARTA/bin/
sudo cp ~/carta-server/scripts/carta_run_local.sh /usr/local/CARTA/bin/
```

7. Customise **carta_run_local.sh** for the server i.e. the "root" and "base" directories that will set where the CARTA filebrowser can access. 
In this example, we consider the user directories are located in **/asiaa/home/** 
Then you could set **-root /asiaa/home/$1 -base /asiaa/home/$1** . By setting "root" the same as "base", this would confine each user to their own home directory. This could be good for security. e.g.
>`/usr/local/CARTA/bin/carta_backend -threads 4 -root /asiaa/home/$1 -base /asiaa/home/$1 -exit_after 0 -init_exit_after 10 -port $CARTA_USER_PORT -use_mongodb 1 > logs/carta_log.$$ 2>&1`

8. Make sure the Mongo database system is running:
```
sudo service mongodb start
sudo service mongodb status
```
9. We want Apache to automatically redirect any http:// requests to https://
```
cd /etc/apache2/sites-enabled
cp ../sites-available/default-ssl.conf .
sudo vi default-ssl.conf
```
Add the following lines to `/etc/apache2/sites-enabled/default-ssl.conf` 
Note: In this example our server's DNS name is `hilo.asiaa.sinica.edu.tw` Please adjust accordingly for your server name: 
><VirtualHost *:80>
ServerName hilo.asiaa.sinica.edu.tw
Redirect / https://hilo.asiaa.sinica.edu.tw
<\/Virtualhost>

10. Add your institutes SSL certificates to **/etc/apache2/sites-available/default-ssl.conf**
>SSLCertificateFile <your.institutes.ssl.cert.crt>
SSLCertificateKeyFile <your.institutes.ssl.key.key>
SSLCACertificateFile <your.institutes.ssl.CA.crt.crt>

Note: Alternatively you could use ‘certbot’ to generate a self-signed certificate, but that requires port 80 and 443 to be opened in any potential firewalls.

Restart the Apache service:
```
sudo a2enmod ssl
sudo systemctl restart apache2
```
Check if it everything is working so far by visiting your servers DNS name in your web browser and check for a “Secure Connection” indicator.

10. Enable Apache access to user directories:

Note: carta-server will execute a Perl script in every user's home directory. They will need to have a directory **/public-html/cgi-bin** that is accessible by the Apache webserver. This is also where CARTA log files are stored. If security is a concern, doing the "local user" method mentioned in Part 3 could be preferable (In that way the server running carta-server would have its own set local users, but with the same UID and GID set the same as the user's institute-wide account, or the same as their institute-wide filesystem e.g. an NFS, Lustre, or Ceph system. 
```
sudo a2enmod suexec
sudo a2enmod userdir
sudo a2enmod cgid
```
Add the following to **/etc/apache2/mods-available/userdir.conf**

><Directory /home/*/public_html/cgi-bin>
Options ExecCGI
SetHandler cgi-script
<\/Directory>  
Restart Apache:
```
sudo systemctl restart apache2
```
  

11. Copy the **carta.pl** script into each user’s **public-html/cgi-bin** directory:
In this example, the user name is 'ajm' in '/asiaa/home' so please adjust accordingly:
```
mkdir -p /asiaa/home/ajm/public_html/cgi-bin
cp ~/carta-server/scripts/carta.pl /asiaa/home/ajm/public_html/cgi-bin/
mkdir -p /asiaa/home/ajm/public_html/cgi-bin/logs
```
 
The **user’s home directory** should have **711** permissions.
**public_html/cgi-bin/** should have **755** permissions.
**carta.pl** should have **755** permissions.
**carta.pl** owner and group should be the user.

A simple bash script could be created that a user runs the first time to automatically the required directories and permissions similar to **carta-server/scripts/addcartauser-idia** or the more complex **carta-server/scripts/addcartauser-iaa** that is designed to to be run by the user only the first time they wish to use carta-server. It will inform them what is going to happen and ask their permission to continue. If they agree it will set up the directory structure and permissions automatically.

12. Quick test: 

Now try to open **carta.pl** directly through your web browser to make sure the php is executed rather than appearing as a file to download. For example (using our server and user name **`https://hilo.asiaa.sinica.edu.tw/~ajm/cgi-bin/carta.pl`**)
If it is working correctly, the socket and token info for the user should appear.

14. Set up reverse-proxy capability and run the **`addsockets.pl`** script:

```
sudo a2enmod proxy_wstunnel
sudo a2enmod rewrite
sudo systemctl restart apache2
sudo ~/carta-server/scripts/addsockets.pl 3010 3100 > /etc/apache2/conf-enabled/proxy_wstunnel.conf
sudo systemctl restart apache2
```
  
15. Now carta-server should now be working:

When the user visits the your server’s URL, e.g. **`https://hilo.asiaa.sinica.edu.tw`** , they should be able to log in with their system username and password, a carta-backend process will spool up, connect on the appropriate free port, and mount their user directory.
