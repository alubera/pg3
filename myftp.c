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

    //Prompt user for operation
    printf("Hello, please enter an operation: REQ, UPL, DEL, LIS, MKD, RMD, CHD, XIT");

    char operation_buf[3]; //operation buf to send to server
    bzero(operation_buf, 3);
    fgets(operation_buf, 3, stdin); //put request into buffer

    int write_val;

    //send the buffer to the server
    if ((write_val = write(s, operation_buf, strlen(operation_buf))) < 0){
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
        if ((write_val = write(s, name_length_str, strlen(name_length_str))) < 0){
            printf("Error writing file length to the server");      
            exit(1);
        }

        //send the file name
        if ((write_val = write(s, file_name, name_length)) < 0){
            printf("Error writing file name to the server");
            exit(1);
        }

        //recieve the 32-bit, decode
                 
    } else if (!strcmp(operation_buf,"UPL")) {


    } else if (!strcmp(operation_buf,"DEL")) {


    } else if (!strcmp(operation_buf,"LIS")){

    
    } else if (!strcmp(operation_buf,"MKD")) {


    } else if (!strcmp(operation_buf,"RMD")) {


    } else if (!strcmp(operation_buf,"CHD")) {


    } else if (!strcmp(operation_buf,"DEL")) {
        close(s);
        printf("Session has been closed, have a nice day.");
    }

    return 0;
}
