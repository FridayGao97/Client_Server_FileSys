#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>


#define MAX_SIZE 2000
#define NUM_CLIENT 1
void *connection_handler(void *socket_desc);
char sbuff[MAX_SIZE],rbuff[MAX_SIZE],connect_buf[MAX_SIZE];
pthread_mutex_t mutex;

int cfileexists(const char * filename){
    /* try to open file to read */
    FILE *file;
    if ((file = fopen(filename, "r"))!=0){

        fclose(file);
        return 1;
    }
    return 0;
}

//https://www.geeksforgeeks.org/c-program-find-size-file/
int findSize(char file_name[]) 
{ 
    // opening the file in read mode 
    FILE* fp = fopen(file_name, "r"); 
  
    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found!\n"); 
        return -1; 
    } 
  
    fseek(fp, 0L, SEEK_END); 
  
    // calculating the size of the file 
    int res = ftell(fp); 
  
    // closing the file 
    fclose(fp); 
  
    return res; 
} 

int finish = 0;
char dfname[50];
/*
Send list command to server
*/
void sending(int sock_desc, char Fbyte) {

    char connect_buf[2000];
    //press q to exit
    if(Fbyte == 'q') {
        connect_buf[0] = 0x08;

        send(sock_desc, connect_buf, strlen(connect_buf), 0);
        //Read the message from the server into the buffer 
        if(recv(sock_desc,rbuff,MAX_SIZE,0)==0)
            printf("Error in receiving");
        else {
            if(rbuff[0] == 0x09) {
                //close client Socket
                finish = 1;
            } 
            else printf("Failure in close");
        }

        //clear connect_buf
        strcpy(connect_buf, "");

    }
    //list
    else if (Fbyte == 'l') {
        connect_buf[0] = 0x00;
        connect_buf[1] = '\0';
        send(sock_desc,connect_buf,sizeof(connect_buf),0);//sending
        strcpy(connect_buf, "");

    }
    //upload
    else if (Fbyte == 'u') {
            connect_buf[0] = 0x02;
            char filename[50];
            //get the filename from input
            sbuff[strcspn(sbuff,"\n")] = 0;
            strcpy(filename, "./");
            //for current location
            int end = strlen(sbuff);
            for (int i = 2; i <= end; i++){
                    connect_buf[i-1] = sbuff[i];
                    filename[i] = sbuff[i];
            }

            pthread_mutex_lock(&mutex);
            if(cfileexists(filename) == 0){
                //error
                printf("CERROR: file not found in local directory\n");
                strcpy(connect_buf,"");
                connect_buf[0] = 0x21;
                send(sock_desc, connect_buf, strlen(connect_buf), 0);
            }
            //check exist
            else{

                //connect_buf[strlen(sbuff)] = '\0';
                int usize = findSize(filename);
                if(usize != 0){
                    //convert int (4 bytes) to 4 bytes char
                    connect_buf[end] = (usize >> 24) & 0xFF;
                    connect_buf[end+1] = (usize >> 16) & 0xFF;
                    connect_buf[end+2] = (usize >> 8) & 0xFF;
                    connect_buf[end+3] = usize & 0xFF;

                    send(sock_desc, connect_buf, end+4, 0);

                    //send file contents
                    // struct stat stat_buf;
                    FILE* read_fd;
                    char buffer[2000];

                    int j;
                    read_fd = fopen (filename, "r");
                    if (usize < 2000){

                        fread(buffer, usize, 1, read_fd);
                        send(sock_desc, buffer, usize, 0);
                        fclose(read_fd);
                    }
    
                    else{
                        int i = usize/2000;
                        for ( j = 0;j<i;j++){
                            fread(buffer, 2000, 1, read_fd);
                            // printf("read_fd: %d\n",buffer);
                            send(sock_desc, buffer, 2000, 0);
                            memset(buffer,0,MAX_SIZE);
                        }
                        fread(buffer, (usize-2000*j), 1, read_fd);
                        send(sock_desc, buffer, (usize-2000*j), 0);
                        memset(buffer,0,(usize-2000*j));
                        fclose(read_fd);
                    } 
                }
            }

            pthread_mutex_unlock(&mutex);

    }
     //delete
    else if(Fbyte == 'r'){ 
        sbuff[strcspn(sbuff,"\n")] = 0;

        connect_buf[0] = 0x04;
        int i;
        for (i = 2; i <= strlen(sbuff); i++){
                connect_buf[i-1] = sbuff[i];
        }
        send(sock_desc, connect_buf, i+1, 0);

    }
    //download
    else if(Fbyte == 'd'){  
        sbuff[strcspn(sbuff,"\n")] = 0;

        connect_buf[0] = 0x06;
        strcpy(dfname, "./");
        int i;
        for (i= 2; i <= strlen(sbuff); i++){
                connect_buf[i-1] = sbuff[i];
                dfname[i] = sbuff[i];
        }
        send(sock_desc, connect_buf, i+1, 0);
        

    }
    else{
        printf("Not a vaild input\n");
    }
 
}

void recievefromserver (int sock_desc,char* recvbuff){
    char recvFbyte = recvbuff[0];

    if (recvFbyte == 0x01){
        recvbuff[strcspn(recvbuff,"\n")] = 0;
        char temp[4];
        temp[3] = 0;           
        temp[2] = 0;
        temp[1] = recvbuff[1];
        temp[0] =recvbuff[2];
        int lsize = *(int *)temp;
                    
        //print something like this: l06abc\0def\0h\0ghi\0hahahah\0lal\0";
        int i = 0;
        int j = 1;
        int last = 0;

        while(i< lsize){
            last = last +j +2;

            printf("OK+ ");

            for(int k = 0; recvbuff[last + k] != '\0'; k++){
                printf("%c",recvbuff[last+k]);
                j=k;
            }
            printf("\n");
            i++;
        }
        printf("OK\n");
    }
    else if (recvFbyte == 0x03){
        printf("OK\n");
        strcpy(rbuff,"");
    }
    else if (recvFbyte == 0x05){
        printf("OK\n");
        strcpy(rbuff,"");
    }

    //download
    else if (recvFbyte == 0x07){
        pthread_mutex_lock(&mutex);
        recvbuff[strcspn(recvbuff,"\n")] = 0;
        char temp[4];
        for(int i=0; i<4; i++)
            temp[3-i] = rbuff[1+i];
        int dsize = *(int *)temp;



        FILE * write_fd;

        write_fd = fopen (dfname, "w");
        int j;
        if(2000 >dsize){
            if (recv(sock_desc,recvbuff,MAX_SIZE,0)>0){
                

                
                fwrite(recvbuff,1, dsize, write_fd);    

            }
            fclose(write_fd);
        }

        else{
            int i = dsize/2000;
            for (j = 0;j<i;j++){
                if (recv(sock_desc,recvbuff,MAX_SIZE,0)>0){

                    fwrite(recvbuff,1, MAX_SIZE, write_fd);    

                }                
            }
            if (recv(sock_desc,recvbuff,dsize-2000*j,0)>0){
                fwrite(recvbuff,1, dsize-2000*j, write_fd);
            }
            fclose(write_fd);
        }

        pthread_mutex_unlock(&mutex);
        printf("OK\n");
        strcpy(rbuff,"");
    }
    else if (recvFbyte == 0x09){
        printf("OK\n");

    }
    else if (recvFbyte == 0x21){
        strcpy(rbuff,"");
        return;
    }
    else if (recvFbyte == -1){
        printf("%s \n", &(rbuff[1]));
    }

    //clear 
    strcpy(rbuff,"");
}

void * sendingThread(void *arg){
	int sock_desc = *((int *)arg);
	while(1)
	{
        //in this loop we first listen to read from stdin and send, then we will get information from server to print
        if (fgets(sbuff, MAX_SIZE , stdin) == NULL) //reading from stdin
			printf("Error on reading from stdin");
		// else
		// 	if (send(sock_desc,sbuff,strlen(sbuff),0) == -1)//sending
		// 		printf("error in sending stdin");

        //check input command
        char Fbyte = sbuff[0];
        //List
        if(Fbyte == 'l'){  
            sending(sock_desc, sbuff[0]);
            // printf("sending l\n");
            if(recv(sock_desc,rbuff,MAX_SIZE,0)>0){
                recievefromserver(sock_desc,rbuff);
            }
            else{
                printf("l:Error in receiving");
            }
        }
        //upload
        else if(Fbyte == 'u'){  
            //printf("sending u\n");
            sending(sock_desc, Fbyte);
            
            if(recv(sock_desc,rbuff,MAX_SIZE,0)>0){
                recievefromserver(sock_desc,rbuff);
            }
            else{
                printf("u:Error in receiving");
            }
        }
        //download
        else if(Fbyte == 'd'){  
            sending(sock_desc, Fbyte);
            //printf("sending d");
            if(recv(sock_desc,rbuff,MAX_SIZE,0)>0){
                recievefromserver(sock_desc,rbuff);
            }
            else{
                printf("d: Error in receiving");
            }
        }
        //delete
        else if(Fbyte == 'r'){  
            sending(sock_desc, Fbyte);
            //printf("sending r");
            if(recv(sock_desc,rbuff,MAX_SIZE,0)>0){
                recievefromserver(sock_desc,rbuff);
            }
            else{
                printf("r: Error in receiving");
            }
        }
        //quitting
        else if(Fbyte == 'q'){  
            sending(sock_desc, Fbyte);
            //printf("sending q\n");
            if(recv(sock_desc,rbuff,MAX_SIZE,0)>0){
                recievefromserver(sock_desc,rbuff);
            }
            else{
                printf("q: Error in receiving");
            }
            if(finish == 1) exit(0);

        }
        else{
            printf("Invalid command\n");
        }
    }
}
int main(int argc, char *argv[])
{
	//create sockets to server
    int sock_desc;
    struct sockaddr_in serv_addr;

    // generate port number
    char* IP_address = argv[1];
    int port_num = atoi(argv[2]);
    
    if((sock_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("Failed creating socket\n");


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP_address);
    serv_addr.sin_port = htons(port_num);

    if (connect(sock_desc, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        printf("Failed to connect to server\n");
    }

    pthread_t readThread;
    pthread_t sendThread;
    // Initialize our global mutex variable
    pthread_mutex_init(&mutex,NULL);

    if( pthread_create(&sendThread, NULL, sendingThread, &sock_desc) != 0 )
        printf("Failed to create thread\n");

    
    pthread_join(readThread,NULL);
    pthread_join(sendThread,NULL);


    // Clean up mutex
    pthread_mutex_destroy(&mutex);
    close(sock_desc);
    return 0;
}
