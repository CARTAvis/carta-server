# carta-server
Components used to start CARTA from a web server. These include tools to maintain a user database needed by the CARTA backend when it is started in this way.

Still need to add cmake files, but have been compiling the cartadbctl program with:

g++ -g -I/usr/include/libbson-1.0 -I/usr/include/libmongoc-1.0/ cartadbctl.cc -o cartadbctl -lmongoc-1.0 -lbson-1.0 -ljsoncpp> err.txt 2>&1

Paths in scripts currently assume that carta_run_local.sh and carta_backend are both placed in /usr/local/CARTA/bin .

On ubuntu you will need the following packages:
libmongoc-dev
libbson-dev
mongodb
mongo-tools
apache2-suexec-custom


Adding a system user to the database. This creates the security token needed to access the carta_backend server.
./cartadbctl -u simmonds
WARNING : This program is under construction and not all the options have been implemented yet
User = simmonds
 done ...