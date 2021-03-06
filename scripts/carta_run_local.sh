#/bin/bash

export LD_LIBRARY_PATH=/usr/lib
export HOME=/users/$1
export CARTA_USER_PORT=$2

myname = $(whoami)

mkdir logs

echo "Home is set to $HOME : socket is $CARTA_USER_PORT"  > logs/carta_start_log.$$ 2>&1

/usr/local/CARTA/bin/carta_backend -threads 4 -root /carta_share -base /carta_share/users/$1 -exit_after 0 -init_exit_after 10 -port $CARTA_USER_PORT -use_mongodb 1 > logs/carta_log.$$ 2>&1
