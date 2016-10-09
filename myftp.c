//Alan Vuong and Andrew Lubera
//netid: avuong and alubera
//TCP Client that will connect to our ftp server
//prompt the user to enter an operation
//send that operation to the server and carry out 
//the operation, displaying the result back
//to the user

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <mhash.h>

int main(int argc, const char* argv[]){

    //check for valid input
    //ex.) ./myftp Server_name [port]
    if(argc != 3){
        printf("Error: Usage ./myftp <hostname> <portnumber>\n");    
        exit(1);
    }

    //initialize variables
    struct hostent *hp;
    struct sockaddr_in sin; //struct to hold info about sockets
    const char* host_name = argv[1];
    int port_num = atoi(argv[2]);
    int s; //socket return code    
    char buf[4096];
    struct sockaddr_in client_addr;      
    struct timeval begin, end;
     
    //creating the socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Error creating the socket");
        exit(1);
    }

    //translate host name into peer's IP address
    hp = gethostbyname(host_name);
    if (!hp){
                fprintf(stderr, "simplex-talk unknown host: %s\n", host_name);
                exit(1);
        }        

    //set addess and port number
    memset((char*)&sin, 0, sizeof(sin)); //make sure to clear mem beforehand
    sin.sin_family = AF_INET;
    bcopy((char *)hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(port_num);

    //create connection
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0){
        printf("Error with connecting");
        exit(1);
    }

    while(1){
        //Prompt user for operation
        printf("Hello, please enter an operation: REQ, UPL, DEL, LIS, MKD, RMD, CHD, XIT");

        char operation_buf[3]; //operation buf to send to server
        bzero(operation_buf, 3);
        fgets(operation_buf, 3, stdin); //put request into buffer

        int send_val;

        //send the buffer to the server
        if ((send_val = send(s, operation_buf, strlen(operation_buf),0)) < 0){
            printf("Error with write function");
            exit(1);
        }

        short int name_length;
        char * name_length_str;
        char * file_name;

        if (!strcmp(operation_buf, "REQ")){ 
            printf("What is the file name you want to request?");
            fgets(file_name, 4096, stdin);
            name_length = strlen(file_name);
            snprintf(name_length_str, 4096, "%d", name_length); //convert int to str
            
            //send the file name length
            if ((send_val = send(s, name_length_str, strlen(name_length_str),0)) < 0){
                printf("Error writing file length to the server");      
                exit(1);
            }

            //send the file name
            if ((send_val = send(s, file_name, name_length,0)) < 0){
                printf("Error writing file name to the server");
                exit(1);
            }

            int rec_bytes; //number of bytes received
            int file_size;
            socklen_t addr_len;
            addr_len = sizeof(client_addr);

            //recieve the 32-bit size, decode
            if((rec_bytes = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &client_addr, &addr_len)) < 0){
                printf("Error receiving!\n");
                exit(1);
            }         

            if(rec_bytes == -1){
                printf("File does not exist on the server\n");
                continue;
            } else{ //set the file size equal to the buf we just received
                file_size = atoi(buf);
            }
            
            //open new file pointer and keep writing to the file
            FILE *f;
            f = fopen(file_name, "a");
            if (f == NULL){
                printf("Error opening the file\n");
            }

            //read "file size" bytes from the server
            //loop while reading the file and exit after last buffer
            //also compute the hash 

            char md5_hash_c[16];
            MHASH td;
            td = mhash_init(MHASH_MD5);
            if (td == MHASH_FAILED){
                printf("MHASH failed\n");
                exit(1);
            }
            int total_rec = 0;

            gettimeofday(&begin,NULL);
            while(1){
                bzero(operation_buf, sizeof(buf));
                if((rec_bytes = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &client_addr, &addr_len)) < 0){
                    printf("Error receiving file\n");
                    exit(1);
                }
                   
                total_rec = total_rec + rec_bytes;
                
                mhash_deinit(td, md5_hash_c); 
                //append new text to the file
                fprintf(f, "%s", buf);

                if(total_rec == file_size){
                    break;
                }
            }
      
            gettimeofday(&end,NULL);
            unsigned int time = end.tv_usec - begin.tv_usec;
            float throughput;
            throughput = file_size/(time*10^-6);

            //receive the has of the file and save
            char md5_hash_s[16]; 
            if((rec_bytes = recvfrom(s, md5_hash_s, sizeof(md5_hash_s), 0, (struct sockaddr *) &client_addr, &addr_len)) < 0){
                printf("Error receiving hash\n");
                exit(1);
            }
          
            //compare hashes
            if (!strcmp(md5_hash_s, md5_hash_c)){
                printf("File transfer successfuli\n");
                printf("The throughput was %i bytes per sec", throughput);
                continue;
            } 
          
            printf("File transfer unsuccessful\n"); 
            continue;
        
        } else if (!strcmp(operation_buf,"UPL")) {


        } else if (!strcmp(operation_buf,"DEL")) {


        } else if (!strcmp(operation_buf,"LIS")){

        
        } else if (!strcmp(operation_buf,"MKD")) {


        } else if (!strcmp(operation_buf,"RMD")) {


        } else if (!strcmp(operation_buf,"CHD")) {


        } else if (!strcmp(operation_buf,"XIT")) {
            close(s);
            printf("Session has been closed, have a nice day.");
        }
    }

    return 0;
}
