#!/usr/bin/perl

if ($#ARGV != 1) {
    print("Usage : $0 first_socket_number last_socket_number\n");
    exit(1);
}

print("RewriteEngine On\n");
print("ProxyRequests Off\n");
print("ProxyPreserveHost On\n");

for( my $i = $ARGV[0]; $i <= $ARGV[1] ; $i++ ) {
    print("ProxyPass \"/socket\" \"ws://localhost:${i}/\"\n");
    print("ProxyPassReverse \"/socket\" \"ws://localhost:${i}/\"\n");
}
