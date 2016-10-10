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
        printf("Hello, please enter an operation: REQ, UPL, DEL, LIS, MKD, RMD, CHD, XIT:\n");
        fflush(stdout);

        char operation_buf[4]; //operation buf to send to server
        bzero(operation_buf, sizeof(operation_buf));
        //fgets(operation_buf, 4, stdin); //put request into buffer
        scanf("%s",operation_buf);

        int send_val;

        //send the buffer to the server
        if ((send_val = send(s, operation_buf, strlen(operation_buf),0)) < 0){
            printf("Error with write function");
            exit(1);
        }

        short int name_length;
        char name_length_str[4096];
        char file_name[4096];

        if (!strcmp(operation_buf, "REQ")){ 
            printf("What is the file name you want to request?\n");
            scanf("%s", file_name);
            //fgets(file_name, 4096, stdin);
            name_length = strlen(file_name);
            snprintf(name_length_str, 4096, "%d", name_length); //convert int to str
            printf("%s %s", name_length_str, file_name);
            fflush(stdout); 
            //send the file name length
            if ((send_val = send(s, name_length_str, strlen(name_length_str) ,0)) < 0){
                printf("Error writing file length to the server\n");      
                exit(1);
            }
            
            //send the file name
            if ((send_val = send(s, file_name, strlen(file_name) ,0)) < 0){
                printf("Error writing file name to the server\n");
                exit(1);
            }

            int rec_bytes; //number of bytes received
            int file_size;
            socklen_t addr_len;
            addr_len = sizeof(client_addr);

            //recieve the 32-bit size, decode
            if((rec_bytes = recv(s, buf, sizeof(buf), 0)) < 0){
                printf("Error receiving!\n");
                exit(1);
            }         

            printf("received: %s\n", buf);

            if(!strcmp(buf,"-1")){
                printf("File does not exist on the server\n");
                continue;
            } else{ //set the file size equal to the buf we just received
                file_size = atoi(buf);
            }
           
            //open new file pointer and keep writing to the file
            FILE *f;
            f = fopen(file_name, "w+");
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
                fflush(stdout);
                bzero(operation_buf, sizeof(buf));
                if((rec_bytes = recv(s, buf, sizeof(buf), 0)) < 0){
                    printf("Error receiving file\n");
                    exit(1);
                }
                if ((send_val = send(s, "READY", 5, 0)) < 0){
                    printf("Error sending ready string\n");
                }
               
                printf("recieved2: %i\n", rec_bytes); 
                fflush(stdout);
                total_rec = total_rec + rec_bytes;
               
                mhash(td,buf,sizeof(buf)); 
                //append new text to the file
                fprintf(f, "%s", buf);

                if(total_rec >= file_size){
                    break;
                }
            }

            mhash_deinit(td, md5_hash_c); 
            gettimeofday(&end,NULL);
            unsigned int time = end.tv_usec - begin.tv_usec;
            float throughput;
            throughput = file_size/(time*10^-6);

            //receive the hash of the file and save
            char md5_hash_s[16]; 
            if((rec_bytes = recv(s, md5_hash_s, sizeof(md5_hash_s), 0)) < 0){
                printf("Error receiving hash\n");
                exit(1);
            }
            fclose(f); 
            
            printf("Client hash: %s, Server hash: %s", md5_hash_c, md5_hash_s);
            
            //compare hashes
            if (!strcmp(md5_hash_s, md5_hash_c)){
                printf("File transfer successful\n");
                printf("The throughput was %i bytes per sec", throughput);
                continue;
            } 
          
            printf("File transfer unsuccessful\n"); 
            continue;
        
        } else if (!strcmp(operation_buf,"UPL")) {


        } else if (!strcmp(operation_buf,"DEL")) {


        } else if (!strcmp(operation_buf,"LIS")){

          int rec_bytes; //number of bytes received
          bzero(buf, sizeof(buf));
          //receive size from directory and loop to read directory
          if((rec_bytes = recv(s, buf, sizeof(buf), 0)) < 0){
            printf("Error receiving!\n");
            exit(1);
          } 

          //loop to show directory listing

        } else if (!strcmp(operation_buf,"MKD")) {

            char directory_name[4096];
            short int dir_length;
            char dir_length_str[4096];

            printf("What is the directory name you would like to create? ");
            scanf("%s", directory_name);
            dir_length = strlen(directory_name);
            snprintf(dir_length_str, 4096, "%d", dir_length); //convert int to str
        
            //send the directory name length
            if ((send_val = send(s, dir_length_str, strlen(dir_length_str) ,0)) < 0){
                printf("Error writing file length to the server\n");
                exit(1);
            }

            //send the directory name
            if ((send_val = send(s, directory_name, strlen(directory_name) ,0)) < 0){
                printf("Error writing the file length to the server\n");
                exit(1);
            }
            int rec_bytes; //number of bytes received
            bzero(buf, sizeof(buf));
            //receive confirm from server
            if ((rec_bytes = recv(s, buf, sizeof(buf), 0)) < 0){
                printf("Error receiving!\n");
                exit(1);
            }

            //check confirms for return message
            if(!strcmp(buf, "-2")){
                printf("The directory already exists on server!\n");
            } else if (!strcmp(buf, "-1")){
                printf("Error in making directory\n");
            } else {
                printf("The directory was successfully made\n");
            }

        } else if (!strcmp(operation_buf,"RMD")) {
            char directory_name[4096];
            short int dir_length;
            char dir_length_str[4096];

            printf("What is the directory name you would like to remove?");
            scanf("%s", directory_name);
            dir_length = strlen(directory_name);
            snprintf(dir_length_str, 4096, "%d", dir_length); //convert int to str
        
            //send the directory name length
            if ((send_val = send(s, name_length_str, strlen(name_length_str) ,0)) < 0){
                printf("Error writing file length to the server\n");
                exit(1);
            }

            //send the directory name
            if ((send_val = send(s, name_length_str, strlen(name_length_str) ,0)) < 0){
                printf("Error writing the file length to the server\n");
                exit(1);
            }
            int rec_bytes; //number of bytes received
            bzero(buf, sizeof(buf));
            //receive confirm from server
            if ((rec_bytes = recv(s, buf, sizeof(buf), 0)) < 0){
                printf("Error receiving!\n");
                exit(1);
            }

            char confirm[5];
            //check confirms for return message
            if(!strcmp(buf, "-1")){
                printf("The directory does not exist on server!\n");
            } else {
                while(1){
                    printf("Are you sure you want to delete the directory? Yes or No? \n");
                    scanf("%s", confirm);
                    if(!strcmp(confirm, "Yes")){
                       //send stuff to server wait display info 

                       break; 

                    } else if (!strcmp(confirm, "No")){
                        printf("Delete abandoned by the user!\n");
                        break;
                    } else{
                        printf("You did not enter a valid response, try again\n");
                    }
                }
            } 

        } else if (!strcmp(operation_buf,"CHD")) {


        } else if (!strcmp(operation_buf,"XIT")) {
            close(s);
            printf("Session has been closed, have a nice day.\n");
            break;
        }
    }

    return 0;
}
