# carta-server
Components used to start CARTA from a web server. These include tools to maintain a user database needed by the CARTA backend when it is started in this way.

Still need to add cmake files, but have been compiling the cartadbctl program with:

g++ -g -I/usr/local/include/libbson-1.0 -I/usr/local/include/libmongoc-1.0/ cartadbctl.cc -o cartadbctl -lmongoc-1.0 -lbson-1.0 -ljsoncpp> err.txt 2>&1 
