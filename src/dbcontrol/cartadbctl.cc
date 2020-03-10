#include <string.h>
#include <time.h>

#define _XOPEN_SOURCE_
#include <unistd.h>
#include <mongoc.h>
#include <iostream>
#include <string>



#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>

char *randstring(int length) {
  static int mySeed = 25011984;
  const char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
  size_t stringLen = strlen(string);
  char *randomString = NULL;

  srand(time(NULL) * length + ++mySeed);

  if (length < 1) {
    length = 1;
  }

  randomString = (char *)malloc(sizeof(char) * (length +1));

  if (randomString) {
    short key = 0;

    for (int n = 0;n < length;n++) {
      key = rand() % stringLen;
      randomString[n] = string[key];
    }

    randomString[length] = '\0';

    return randomString;
  }
  else {
    printf("No memory");
    exit(1);
  }
}


char * shared_token = "skU0p!2XH2!zCszyzzatRLQggk0tv0EkHVHuU6aYR!hQ6G?oDMedhsXnCGdv4eF8";

int
main (int argc, char *argv[])
{
  const char *uri_string = "mongodb://localhost:27017";
  mongoc_uri_t *uri;
  mongoc_client_t *client;
  mongoc_database_t *database;
  mongoc_collection_t *collection;
  mongoc_cursor_t *cursor;
  bson_t *command, reply, *insert, userconf, *query;
  const bson_t *doc;
  bson_error_t error;
  char *str;
  std::string tmp_socket;
  bool retval;
  int lowsock, highsock;
  int user_socket;
  int num_threads;
  char * sock_str = NULL;
  char * remove_user = NULL;
  char * uvalue = NULL;
  char * pvalue = NULL;
  char * bvalue = NULL;
  char * tvalue = NULL;
  char * mvalue = NULL;
  char * encrypted = NULL;
  char num_threads_str[10];
  char tmp_char_string[10];
  int c;
  Json::Value json_config;
  Json::Reader reader;
  
  lowsock = 3010;
  highsock = 3400;
  num_threads = 4;

  fprintf(stderr,"WARNING : This program is under construction and not all the options have been implemented yet\n");

  while ((c = getopt(argc, argv, "u:p:b:t:m:n:r:h")) > 0) {
    switch(c) {
    case 'u': {
      uvalue = optarg;
      break;
    }
    case 'p': {
      pvalue = optarg;
      break;
    }
    case 'b': {
      lowsock = atoi(optarg);
      break;
    }
    case 't': {
      highsock = atoi(optarg);
      break;
    }
    case 'm': {
      uri_string = optarg;
      break;
    }
    case 'n': {
      num_threads = atoi(optarg);
      break;
    }
    case 'r': {
      remove_user = optarg;
    }
    case 'h': {
      fprintf(stderr,
	      "Usage :\n"
	      "\t-u username\tThe name of the user to add to the CARTA database\n"
	      "\t-p password\tSet the password of an external user. Don't call\n"
	      "\t\t\tthis for a system user as their password will have\n"
	      "\t\t\talready been set at the system level\n"
	      "\t-b sockno\tThe lowest value in the range of sockets to assign\n"
	      "\t-t sockno\tThe highst value in the ragne of sockets to assigne\n"
	      "\t-n num_threads\tThe default number of threads for this user\n"
	      "\t-r username\tRemove this user from the database\n"
	      "\t-m url\t\tThe url of the mongoDB instance you want to use\n"
	      "\t-h\t\tPrint this usage message\n");
      break;
    }
    default: {
      fprintf(stderr,"Unknown option : %c : use -h to get usage information.\n", c );
    }
    }
  }

  if(uvalue) {
    fprintf(stderr,"User = %s\n", uvalue);
  } else {
    fprintf(stderr,"Username must be provided. Use -h to see usage options.\n");
    exit(1);
  }
  
  mongoc_init ();
  
  uri = mongoc_uri_new_with_error (uri_string, &error);
  if (!uri) {
    fprintf (stderr,"failed to parse URI: %s\n"
	     "error message:       %s\n",
	     uri_string, error.message);
    return EXIT_FAILURE;
  }

  client = mongoc_client_new_from_uri (uri);
  if (!client) {
    return EXIT_FAILURE;
  }

  mongoc_client_set_appname (client, "cartadbctl");

  database = mongoc_client_get_database (client, "CARTA");
  collection = mongoc_client_get_collection (client, "CARTA", "userconf");

  command = BCON_NEW ("ping", BCON_INT32 (1));
  retval = mongoc_client_command_simple (client, "admin", command, NULL, &reply, &error);
  if (!retval) {
    fprintf (stderr, "%s\n", error.message);
    return EXIT_FAILURE;
  }

  str = bson_as_json (&reply, NULL);
  //  printf ("%s\n", str);

  query = bson_new ();
  BSON_APPEND_UTF8 (query, "username", uvalue);
  cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);
  
  if (mongoc_cursor_next (cursor, &doc)) {
    str = bson_as_canonical_extended_json (doc, NULL);
    printf ("%s\n", str);
     
    fprintf(stderr,"User \"%s\"is already in the database.\n"
	    "Remove them and add them back if needed.\n", uvalue );
    exit(1);
  }
  bson_destroy (query);
  
  query = bson_new ();
  opts = BCON_NEW ("sort", "{", "socket", BCON_INT32 (1), "}");
  cursor = mongoc_collection_find_with_opts (collection, query, opts, NULL);
  
  user_socket = lowsock;

  while (mongoc_cursor_next (cursor, &doc)) {
    str = bson_as_canonical_extended_json (doc, NULL);
    reader.parse(str, json_config);
    bson_free (str);    
    tmp_socket = json_config["socket"].asString();
    strcpy(tmp_char_string,tmp_socket.c_str());
    if (user_socket == atoi(tmp_char_string)) {
      ++user_socket;
    } else {
      break;
    }
  }
  bson_destroy (query);
  
  sock_str = (char*)malloc(120);

  sprintf(sock_str,"%d", user_socket);
  sprintf(num_threads_str,"%d", num_threads);
    
  insert = bson_new();

  bson_init(&userconf);
  BSON_APPEND_UTF8 (&userconf,"username", uvalue);
  BSON_APPEND_UTF8 (&userconf,"socket", sock_str);
  if (pvalue) {
    str = shared_token;
    BSON_APPEND_UTF8 (&userconf,"usertype", "external");
    encrypted = crypt(pvalue, "x6");
    BSON_APPEND_UTF8 (&userconf,"password", encrypted);
  } else {
    str = randstring(64);
    BSON_APPEND_UTF8 (&userconf,"usertype", "system");
  }
  BSON_APPEND_UTF8 (&userconf,"token", str);
  BSON_APPEND_UTF8 (&userconf,"num_threads", num_threads_str);
    
  if (!mongoc_collection_insert_one (collection, &userconf, NULL, NULL, &error)) {
    fprintf (stderr, "%s\n", error.message);
  }
  
  bson_destroy (insert);
  bson_destroy (&reply);
  bson_destroy (command);
  //  bson_free (str);

  mongoc_collection_destroy (collection);
  mongoc_database_destroy (database);
  mongoc_uri_destroy (uri);
  mongoc_client_destroy (client);
  mongoc_cleanup ();

  fprintf(stdout," done ...\n");
  return EXIT_SUCCESS;
}
