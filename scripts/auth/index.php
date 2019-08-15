<?php

  // Takes raw data from the request
  $json_in = file_get_contents('php://input');

  // Converts it into a PHP object
  $data = json_decode($json_in);

  $username = $data->username;
  $password = $data->password;

  $myobj = new \stdClass;
  $mongo = new MongoDB\Driver\Manager();
  $filter = ['username' => $username];
  $options = [];
  $query = new \MongoDB\Driver\Query($filter, $options);
  $rows = $mongo->executeQuery('CARTA.userconf', $query);

  foreach ($rows as $document) {
    $document = json_decode(json_encode($document),true);
    $myobj->username = $document['username'];
    $myobj->token = $document['token'];
    $socket_string =  "socket" . $document['socket'];
    $myobj->socket = $socket_string;
  }
//  echo json_encode($myobj);

  $username = $myobj->username;

  $socket = $myobj->socket;
  $auth_string = $username.":".$password;
  $suexecstring = "https://carta-dev.idia.ac.za/~${username}/cgi-bin/carta.pl?socket=$socket";      
   
  $ch = curl_init();

  // set URL and other appropriate options
  curl_setopt($ch, CURLOPT_URL, $suexecstring);
  curl_setopt($ch, CURLOPT_HEADER, 0);
  curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
  curl_setopt($ch, CURLOPT_USERPWD, $auth_string);
  curl_setopt($ch, CURLOPT_NOBODY, 0);

  // grab URL and pass it to the browser
  curl_exec($ch);
  
  // forward the response code
  $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
  http_response_code($http_code);

  // close cURL resource, and free up system resources
  curl_close($ch);
?>
