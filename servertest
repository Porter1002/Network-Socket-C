#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<linux/in.h>
#include<pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <search.h>
int SERV_PORT = 1100;
struct thread_pass{
    int sockfd;
    userlink *head;
    msg *bmsg;//try to send msg to user
    usermsg *umsg;//try to deal msg from user

}pass;

strruct buf{
    int sockfd;
    userlink *head;

}bufpack;

int main(int argc ,const char*argv[]){
    int sockfd=socket(AF_INET6,SOCK_DGRAM , 0);
	struct sockaddr_in serveraddr , clientaddr;
    if(sockfd< 0)
	{
		perror("socket error");
		exit(1);
	}
    serveraddr.sin_family=AF_INET6;
    serveraddr.sin_port=htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sockfd,(struct sockaddr *)&serveraddr, sizeof(struct sockaddr))==-1){
        perror("Bind error. ");
        exit(1);
    }
    socklen_t clientlen =sizeof(clientaddr);
    pthread_t server_send_clientmsg,client_send_servermsg;
    bufpack.head=head;
    bufpack.sockfd =sockfd;
    pthread_create(&client_send_servermsg,NULL,servermsg_thread,NULL);//client give server
    pthread_create(&server_send_clientmsg,NULL,clientmsg_thread,NULL);//server give client

    

    return 0; 
}





void* servermsg_thread(void *p){//client give server    *******now only deal msg

}

void* clientmsg_thread(void*p){//server give client    *******now only deal msg

}
