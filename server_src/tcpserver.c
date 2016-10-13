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
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <mhash.h>
#include <dirent.h>
#define MAX_BUFFER 512
#define MAX_PENDING 0

int my_ls(char* file_list) {
  DIR *dp;
  struct dirent *ep;
  int space = sizeof(file_list);
  
  dp = opendir("./");
  if (dp != NULL) {
    while (ep = readdir(dp)) {
      strncat(file_list,ep->d_name,space);
      if ((space = space-sizeof(ep->d_name)) < 2) {
        printf("too many file names to print\n");
        break;
      }
      strcat(file_list,"\n");
    }
    closedir(dp);
  } else {
    return -1;
  }
  return 0;
}

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
  int read;

  // wait to get a connection request from a client
  while (1) {
    if ((new_s = accept(s,(struct sockaddr *)&sin,&slen)) < 0) {
      fprintf(stderr,"ERROR: accept error\n");
      exit(1);
    }
    printf("New client\n");
    // continue to receive messages after client connects until XIT
    while (1) {
      memset((char*)&buf,0,sizeof(buf));
      if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
        fprintf(stderr,"ERROR: receive error\n");
        exit(1);
      }

      // determine operation sent by client
      if (!strcmp(buf,"REQ")) {
        /**********************************
         *
         *  REQUEST OPERATION
         *
         **********************************/
        printf("Client opeartion: REQ\n");

        // receive file name length
        if ((num_rec = recv(new_s,&fname_length,sizeof(short int),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        fname_length = ntohs(fname_length);
        printf("\tFilename length: %i\n",fname_length);

        // use length to set mem for fname
        fname = (char*)malloc(fname_length);
        memset(fname,0,fname_length);

        // receive file name string
        memset((char*)&buf,0,sizeof(buf));
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        strcpy(fname,buf);
        printf("\tFilename: %s\n",fname);

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
        int count = 0;
        memset((char*)&buf,0,sizeof(buf));
        while ((read = fread(buf,sizeof(char),MAX_BUFFER,fp)) > 0) {
          if ((num_sent = send(new_s,buf,read,0)) == -1) {
            fprintf(stderr,"ERROR: send error\n");
            exit(1);         
          }
          printf("%i\t%i\t%i\t%i\n",++count,read,strlen(buf),num_sent);
          fflush(stdout);
          mhash(td,buf,read);
          // wait for client to be ready for next message
          memset((char*)&buf,0,sizeof(buf));
        }

        // compute MD5 hash string and send
        mhash_deinit(td,hash);
        if ((num_sent = send(new_s,hash,sizeof(hash),0)) == -1) {
          fprintf(stderr,"ERROR: send error\n");
          exit(1);         
        }

        free(fname);
        fclose(fp);

      } else if (!strcmp(buf,"UPL")) {
        /**********************************
         *
         *  UPLOAD OPERATION
         *
         **********************************/
        memset((char*)&buf,0,sizeof(buf));
        // receive length of file name...then actual file name
        // receive file

      } else if (!strcmp(buf,"DEL")) {
        /**********************************
         *
         *  DELETE OPERATION
         *
         **********************************/
        memset((char*)&buf,0,sizeof(buf));
        // receive length of file name...then actual file name
        // syscall remove(filename)

      } else if (!strcmp(buf,"LIS")) {
        /**********************************
         *
         *  LIST OPERATION
         *
         **********************************/
        printf("Client opeartion: REQ\n");
        memset((char*)&buf,0,sizeof(buf));
        // call listing function
        if (my_ls(buf) < 0) { 
          fprintf(stderr,"ERROR: could not list directory\n");
          continue;
        }
        // send it back to the client

      } else if (!strcmp(buf,"MKD")) {
        /**********************************
         *
         *  MK DIR OPERATION
         *
         **********************************/
        memset((char*)&buf,0,sizeof(buf));
        printf("Client opeartion: MKD\n");
        // receive directory name length
        memset((char*)&buf,0,sizeof(buf));
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        fname_length = atoi(buf);
        printf("\tDirectory name length: %i\n",fname_length);
        // use length to set mem for fname
        fname = (char*)malloc(fname_length);
        memset(fname,0,fname_length);
        // receive directory name string
        memset((char*)&buf,0,sizeof(buf));
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        strcpy(fname,"./");
        strcat(fname,buf);
        printf("\tDirectory name: %s\n",fname);
        memset((char*)&buf,0,sizeof(buf));
        // use mkdir to make directory
        if (mkdir(fname,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) < 0) {
          // send back -2 if directory already exists
          if (errno == EEXIST) {
            printf("Directory already exists\n");
            sprintf(buf,"%i",-2);
          // send back -1 if theres another error
          } else {
            sprintf(buf,"%i",-1);
          }
        } else {
          // send back 1 if directory is created successfully
          sprintf(buf,"%i",1);
        }
        // send response message
        if ((num_sent = send(new_s,buf,strlen(buf),0)) == -1) {
          fprintf(stderr,"ERROR: send error\n");
          exit(1);         
        }
        free(fname);

      } else if (!strcmp(buf,"RMD")) {
         /**********************************
         *
         *  RM DIR OPERATION
         *
         **********************************/
        memset((char*)&buf,0,sizeof(buf));
        printf("Client opeartion: RMD\n");
        // receive directory name length
        memset((char*)&buf,0,sizeof(buf));
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        fname_length = atoi(buf);
        printf("\tDirectory name length: %i\n",fname_length);
        // use length to set mem for fname
        fname = (char*)malloc(fname_length);
        memset(fname,0,fname_length);
        // receive directory name string
        memset((char*)&buf,0,sizeof(buf));
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        strcpy(fname,"./");
        strcat(fname,buf);
        strcat(fname,"/");
        printf("\tDirectory name: %s\n",fname);
        memset((char*)&buf,0,sizeof(buf));
        // check to see if directory exists
        if (access(fname,F_OK) != 0) {
          if (errno == ENOENT || errno == ENOTDIR) {
            // name provided is not a directory
            sprintf(buf,"%i",-1);
          } else {
            // directory exists
            sprintf(buf,"%i",1);
          }
        } else {
          // some other error accessing directory
          sprintf(buf,"%i",-1);
        }
        // send confirm message
        if ((num_sent = send(new_s,buf,strlen(buf),0)) == -1) {
          fprintf(stderr,"ERROR: send error\n");
          exit(1);         
        }
        memset((char*)&buf,0,sizeof(buf));
        // receive confirmation that client wants to delete
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        /*if () {
          // user DOES NOT want to continue with delete
          continue;
        }*/
        // use rmdir to remove directory
        if (rmdir(fname) < 0) {
          // send back -2 if directory already exists
          if (errno == EEXIST) {
            printf("Directory already exists\n");
            sprintf(buf,"%i",-2);
          // send back -1 if theres another error
          } else {
            sprintf(buf,"%i",-1);
          }
        } else {
          // send back 1 if directory is created successfully
          sprintf(buf,"%i",1);
        }
        // send response message
        if ((num_sent = send(new_s,buf,strlen(buf),0)) == -1) {
          fprintf(stderr,"ERROR: send error\n");
          exit(1);         
        }
        free(fname);

      } else if (!strcmp(buf,"CHD")) {
        /**********************************
         *
         *  CH DIR OPERATION
         *
         **********************************/
       memset((char*)&buf,0,sizeof(buf));
        // expect to receive again for file name
        if ((num_rec = recv(new_s,buf,sizeof(buf)/sizeof(char),0)) == -1) {
          fprintf(stderr,"ERROR: receive error\n");
          exit(1);
        }
        // syscall chdir(path)

      } else if (!strcmp(buf,"XIT")) {
        // break out of client loop
        printf("client exited\n");
        break;
      }
    }
  }

  close(s);

  return 0;
}
