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
#include <mhash.h>
#define MAX_BUFFER 4096
#define MAX_PENDING 0

int main(int argc, char* argv[]) {
  struct sockaddr_in sin;           // struct for address info
  int slen = sizeof(sin); 
  char buf[MAX_BUFFER];             // message received from client
  int server_port;                  // user input - port num
  int s, new_s;                     // socket and new socket
  int num_rec, num_sent;            // size of messages sent and received
  MHASH td;                         // for computing MD5 hash
  char hash[16];                    // for sending MD5 hash as 16-byte string

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

  // variables used in client server communication
  short int fname_length;
  char* fname;
  FILE* fp;
  int file_size;

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
        // receive file name length
        memset((char*)&buf,0,sizeof(buf));
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        fname_length = atoi(buf);
        // use length to set mem for fname
        fname = (char*)malloc(fname_length);
        // receive file name string
        memset((char*)&buf,0,sizeof(buf));
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        strcpy(fname,buf);
        // find file and send size
        if ((fp = fopen(fname,"r")) != NULL) {
          // use fseek to get file size
          fseek(fp,0,SEEK_END);
          file_size = ftell(fp);
          fseek(fp,0,SEEK_SET);
        } else {
          // file does not exist, send error response
          file_size = -1;
        }
        memset((char*)&buf,0,sizeof(buf));
        sprintf(buf,"%i",file_size);
        if ((num_sent = send(new_s,buf,strlen(buf),0)) == -1) {
          fprintf(stderr,"ERROR: send error\n");
          exit(1);         
        }
        // if file does not exist, skip sending step
        if (file_size == -1) continue;
        // init mhash
        if ((td = mhash_init(MHASH_MD5)) == MHASH_FAILED) {
          fprintf(stderr,"ERROR: unable to initialize MHASH\n");
          exit(1);
        }
        // read file into buffer, 4096 chars at a time
        // send each buffer as well as adding to MD5 hash
        memset((char*)&buf,0,sizeof(buf));
        while (fread(buf,sizeof(char),MAX_BUFFER,fp)) {
          if ((num_sent = send(new_s,buf,strlen(buf),0)) == -1) {
            fprintf(stderr,"ERROR: send error\n");
            exit(1);         
          }
          mhash(td,buf,MAX_BUFFER);
          memset((char*)&buf,0,sizeof(buf));
        }
        // compute MD5 hash string and send
        mhash_deinit(td,hash);
        if ((num_sent = send(new_s,hash,strlen(hash),0)) == -1) {
          fprintf(stderr,"ERROR: send error\n");
          exit(1);         
        }
        free(fname);
        fclose(fp);

      } else if (!strcmp(buf,"UPL")) {
        memset((char*)&buf,0,sizeof(buf));
        // receive length of file name...then actual file name
        // receive file

      } else if (!strcmp(buf,"DEL")) {
        memset((char*)&buf,0,sizeof(buf));
        // receive length of file name...then actual file name
        // syscall remove(filename)

      } else if (!strcmp(buf,"LIS")) {
        memset((char*)&buf,0,sizeof(buf));
        // call system ls call
        // send it back to the client

      } else if (!strcmp(buf,"MKD")) {
        memset((char*)&buf,0,sizeof(buf));
        // receive length of file name...then actual file name
        // syscall mkdir(filename)

      } else if (!strcmp(buf,"RMD")) {
        memset((char*)&buf,0,sizeof(buf));
        // receive length of dir name...then actual dir name
        // syscall rmdir(dirname)

      } else if (!strcmp(buf,"CHD")) {
        memset((char*)&buf,0,sizeof(buf));
        // expect to receive again for file name
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        // syscall chdir(path)

      } else if (!strcmp(buf,"XIT")) {
        // break out of 
        break;
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
