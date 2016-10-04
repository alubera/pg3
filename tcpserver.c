/* Andrew Lubera  | alubera
 * Alan Vuong     | avuong
 * 10-12-16
 * CSE30264
 * Programming Assignment 3 - Program acts as a server for a simple File Transfer
 *    Protocol. The FTP will use a TCP connection to perform the following operations:
 *      REQ (Request), UPL (Upload), LIS (List), MKD (Make Directory), RMD (Remove 
 *      Directory), CHD (Change Directory), and XIT (Exit)
 *    USER INPUTS:
 *      port number (41038, 41039)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#define MAX_BUFFER 4096
#define MAX_PENDING 0

int main(int argc, char* argv[]) {
  struct sockaddr_in sin;           // struct for address info
  int slen = sizeof(sin); 
  char buf[MAX_BUFFER];             // message received from client
  int server_port;                  // user input - port num
  int s, new_s;                     // socket and new socket
  int num_rec;                      // size of message received from client

  // handle all arguments
  if (argc == 2) {
    // port number
    server_port = atoi(argv[1]);
  } else {
    fprintf(stderr,"ERROR: invalid number of arguments\n");
    fprintf(stderr,"\tprogram should be called with port number\n");
    exit(1);
  }

  // build address data structure
  memset((char*)&sin,0,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(server_port);

  // create TCP socket
  s = socket(PF_INET,SOCK_STREAM,0);
  if (s < 0) {
    fprintf(stderr,"ERROR: unable to create socket\n");
    exit(1);
  }

  // set reuse option on socket
  int opt = 1;
  if ((setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))) < 0) {
    fprintf(stderr,"ERROR: unable to set reuse option on socket\n");
    exit(1);
  }

  // bind socket to address
  if ((bind(s,(struct sockaddr*)&sin,slen)) < 0) {
    fprintf(stderr,"ERROR: unable to bind socket\n");
    exit(1);
  }

  // open the TCP socket by listening
  if ((listen(s,MAX_PENDING)) < 0) {
    fprintf(stderr,"ERROR: unable to open socket\n");
    exit(1);
  }

  // wait to get a connection request from a client
  while (1) {
    if ((new_s = accept(s,(struct sockaddr *)&sin,&slen)) < 0) {
      fprintf(stderr,"ERROR: accept error\n");
      exit(1);
    }
    // continue to receive messages after client connects until XIT
    while (1) {
      memset((char*)&buf,0,sizeof(buf));
      if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
        fprintf(stderr,"ERROR: receive error\n");
        exit(1);
      }
      // determine operation sent by client
      if (!strcmp(buf,"REQ")) {

      } else if (!strcmp(buf,"UPL")) {

      } else if (!strcmp(buf,"DEL")) {

      } else if (!strcmp(buf,"LIS")) {

      } else if (!strcmp(buf,"MKD")) {

      } else if (!strcmp(buf,"RMD")) {

      } else if (!strcmp(buf,"CHD")) {

      } else if (!strcmp(buf,"XIT")) {

      }


      // send back to client
/*      if (sendto(s,buf,num_rec,0,(struct sockaddr*)&client_addr,sizeof(struct sockaddr)) == -1) {
        fprintf(stderr,"ERROR: server send error: %s\n",strerror(errno));
        exit(1);
      }
*/
    }

  }

  close(s);

  return 0;
}
