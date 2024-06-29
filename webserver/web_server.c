#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include "config.h"
#include "helpers.h"

/*------------------------------------------------------------------------
 * Program:   http server
 *
 * Purpose:   allocate a socket and then repeatedly execute the following:
 *              (1) wait for the next connection from a client
 *              (2) read http request, reply to http request
 *              (3) close the connection
 *              (4) go back to step (1)
 *
 * Syntax:    http_server [ port ]
 *
 *               port  - protocol port number to use
 *
 * Note:      The port argument is optional.  If no port is specified,
 *            the server uses the port specified in config.h
 *
 *------------------------------------------------------------------------
 */

int main(int argc, char *argv[])
{
  /* structure to hold server's and client addresses, respectively */
  struct sockaddr_in server_address, client_address;

  int listen_socket = -1;
  int connection_socket = -1;
  int port = 0;

  /* id of child process to handle request */
  pid_t pid = 0;

  char response_buffer[MAX_HTTP_RESPONSE_SIZE] = "";
  int status_code = -1;
  char *status_phrase = "";

  /* 1) Create a socket */
  /* START CODE SNIPPET 1 */
  listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket == -1) {
    fprintf(stderr, "ERROR: could not create listening socket.\n");
    exit(EXIT_FAILURE);
  }
  /* END CODE SNIPPET 1 */

  /* Check command-line argument for port and extract
   * port number if one is specified. Otherwise, use default
   */
  if (argc > 1)
  {
    /* Convert from string to integer */
    port = atoi(argv[1]);
  }
  else
  {
    port = DEFAULT_PORT;
  }

  if (port <= 0)
  {
    /* Test for legal value */
    fprintf(stderr, "bad port number %d\n", port);
    exit(EXIT_FAILURE);
  }

  /* Clear the server address */
  memset(&server_address, 0, sizeof(server_address));

  /* 2) Set the values for the server address structure */
  /* START CODE SNIPPET 2 */
  server_address.sin_addr.s_addr = INADDR_ANY; 
  server_address.sin_family = AF_INET; 
  server_address.sin_port = htons(port); 
  /* END CODE SNIPPET 2 */

  /* 3) Bind the socket to the address information set in server_address */
  /* START CODE SNIPPET 3 */
  if (bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
    fprintf(stderr,"ERROR: failure to bind the socket to the address\n");
    exit(EXIT_FAILURE);
  }
  /* END CODE SNIPPET 3 */

  /* 4) Start listening for connections */
  /* START CODE SNIPPET 4 */
  int descp = listen(listen_socket, SOMAXCONN);
  if (descp < 0) {
    fprintf(stderr,"ERROR: Unable to listen on the socket\n");
    exit(EXIT_FAILURE);
  }
  /* END CODE SNIPPET 4 */

  /* Main server loop
   * Loop while the listen_socket is valid
   */
  while (listen_socket >= 0)
  {
    /* 5) Accept a connection */
    /* START CODE SNIPPET 5 */
    socklen_t client_len = sizeof(client_address);
    connection_socket = accept(listen_socket, (struct sockaddr *)&client_address, &client_len);
    if (connection_socket < 0) {
      fprintf(stderr,"ERROR: could not accept connection\n"); 
      continue; 
    }
    /* END CODE SNIPPET 5 */

    /* Fork a child process to handle this request */
    if ((pid = fork()) == 0)
    {
      /*----------START OF CHILD CODE----------------*/
      /* We are now in the child process */

      /* Close the listening socket
       * The child process does not need access to listen_socket 
       */
      if (close(listen_socket) < 0)
      {
        fprintf(stderr, "child couldn't close listen socket\n");
        exit(EXIT_FAILURE);
      }

      /* See httpreq.h for definition */
      struct http_request new_request;
      /* 6) call helper function to read the request
       * this will fill in the struct new_request for you
       * see helper.h and httpreq.h                      
       */
      /* START CODE SNIPPET 6 */
      bool http_request_valid = Parse_HTTP_Request(connection_socket, &new_request);
      if (!http_request_valid) { 
        fprintf(stderr, "ERROR: unable to parse the HTTP request\n");
        exit(EXIT_FAILURE);
      } 
      /* END CODE SNIPPET 6 */

      /* 7) Decide which status_code and reason phrase to return to client */
      /* START CODE SNIPPET 7 */
      if (!http_request_valid) {
        
          status_code = 400;
          status_phrase = "Bad Request";


      } else {
          // checkin if the method is get or head since they handled the same way for status code purposes
          if (strcmp(new_request.method, "GET") == 0 || strcmp(new_request.method, "HEAD") == 0) {
              // check if requested resource exists
              if (Is_Valid_Resource(new_request.URI)) {
                  status_code = 200; // its found
                  status_phrase = "OK";
              } else {
                  status_code = 404; // its not found
                  status_phrase = "Not Found";
              }
          } else {
              // if method is neither get nor head assume it is not implemented
              status_code = 501;
              status_phrase = "Not Implemented";
          }
      }
      /* END CODE SNIPPET 7 */

      /* 8) Set the reply message to the client
       * Copy the following line and fill in the ??
       * sprintf(response_buffer, "HTTP/1.0 %d %s\r\n", ??, ??);
       */
      /* START CODE SNIPPET 8 */
      sprintf(response_buffer, "HTTP/1.0 %d %s\r\n", status_code, status_phrase);
      /* END CODE SNIPPET 8 */

      printf("Sending response line: %s\n", response_buffer);

      /* 9) Send the reply message to the client
       * Copy the following line and fill in the ??
       * send(??, response_buffer, strlen(response_buffer), 0);
       */
      /* START CODE SNIPPET 9 */
      send(connection_socket, response_buffer, strlen(response_buffer), 0);
      /* END CODE SNIPPET 9 */

      bool is_ok_to_send_resource = false;
      /* 10) Send resource (if requested) under what condition will the
       * server send an entity body?
       */
      /* START CODE SNIPPET 10 */

      bool send_header_only = false;

      
      // only send resource if it a GET request and resource is valid
      if (strcmp(new_request.method, "GET") == 0) {
          if (Is_Valid_Resource(new_request.URI)) {
              is_ok_to_send_resource = true;
          }
      }
      // dealing wiht HEAD requests, do not send the resource, but send headers
      else if (strcmp(new_request.method, "HEAD") == 0) {
          if (Is_Valid_Resource(new_request.URI)) {
              send_header_only = true;
          }
      }
      /* END CODE SNIPPET 10 */

      if (is_ok_to_send_resource)
      {
        Send_Resource(connection_socket, new_request.URI);
      }
      // editing this for headsrs
      else if (send_header_only) {
        Dont_Send_Resource(connection_socket, new_request.URI);
      } else {
        /* 11) Do not send resource
         * End the HTTP headers
         * Copy the following line and fill in the ??
         * send(??, "\r\n\r\n", strlen("\r\n\r\n"), 0);
         */
        /* START CODE SNIPPET 11 */
        send(connection_socket, "\r\n\r\n", strlen("\r\n\r\n"), 0);
        /* END CODE SNIPPET 11 */
      }

      /* Child's work is done
       * Close remaining descriptors and exit 
       */
      if (connection_socket >= 0)
      {
        if (close(connection_socket) < 0)
        {
          fprintf(stderr, "closing connected socket failed\n");
          exit(EXIT_FAILURE);
        }
      }

      /* All done return to parent */
      exit(EXIT_SUCCESS);
    }
    /*----------END OF CHILD CODE----------------*/

    /* Back in parent process
     * Close parent's reference to connection socket,
     * then back to top of loop waiting for next request 
     */
    if (connection_socket >= 0)
    {
      if (close(connection_socket) < 0)
      {
        fprintf(stderr, "closing connected socket failed\n");
        exit(EXIT_FAILURE);
      }
    }

    /* if child exited, wait for resources to be released */
    waitpid(-1, NULL, WNOHANG);
  }
}
