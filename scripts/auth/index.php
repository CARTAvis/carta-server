<?php
	$user = $_SERVER['PHP_AUTH_USER'];
	header( "Location: https://carta.idia.ac.za/~${user}/cgi-bin/carta.pl" ) ;
?>