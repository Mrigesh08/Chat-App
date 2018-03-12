#include <stdio.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>// different address structures are declared here
#include <stdlib.h> // atoi() which convert string to integer
#include <string.h>
#include <unistd.h> // close() function
#include <errno.h>

#define BUFSIZE 100

void print_instructions(){
	printf("************************************************\n");
	printf("enter your USERNAME when asked(max 20 characters)\n");
	printf("to get all user list use 'get_users' command\n");
	printf("Status 'o' means user is online and 'x' means that the user if offline\n");
	printf("to exit the chat system use 'exit' command\n");
	printf("to send message to a particular user '<username:message>' username and colon\n");
	printf("\t eg=> user1: hello\n");
	printf("to broadcast a message use command 'broad'\n");
	printf("\t eg=> broad: hello\n");
	printf("to make a group use 'mk_grp' command\n");
	printf("\t mk_grp:<group_name>:<user_list>\n");
	printf("\t eg=> mk_grp:group1:user1:user2:user3\n");
	printf("to send message to a group of users '<group_name>: <message>'\n");
	printf("\t eg=> group1: hello\n\n");
	printf("NOTE: Press ENTER to see if any new messages have been received\n");
	printf("************************************************\n");
}

void strEcho(int sockfd){
	char buff[BUFSIZE];
	char buff1[BUFSIZE];
	int n,non_block_recv;
	while(1){
		while(1){
			non_block_recv=recv (sockfd, buff1, BUFSIZE-1, MSG_DONTWAIT);
			if(non_block_recv<0 && (errno==EWOULDBLOCK || errno==EAGAIN)){
				//do nothing
				break;
			}
			else if(non_block_recv<0){
				printf("error in receiving data\n");
			}
			else if(non_block_recv>0){
				printf("from %s\n",buff1);
				bzero(&buff1,BUFSIZE);
			}
		}
		//printf("out of while\n");
		gets(buff);
		if(buff[0]!='\0'){
			send (sockfd, buff, strlen(buff), 0);
			if((strncmp(buff,"exit",4))==0){
				printf("Client Exit...\n");
				break;
			}
			bzero(&buff,BUFSIZE);
		}
	}
}

int main(){

	int n=0;
	char msg[BUFSIZE];

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0){
		printf ("Error in opening a socket\n");
		exit (0);
	}
	printf ("Client Socket Created\n");

	struct sockaddr_in serverAddr;
	memset (&serverAddr,0,sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9000); //You can change port number here
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Specify server's IP address here
	printf ("Address assigned\n");

	int clientSocket = connect (sock, (struct sockaddr*)&serverAddr,sizeof(serverAddr));
	if (clientSocket < 0){
		printf ("Error while establishing connection\n");
		exit (0);
	}
	printf ("Connection Established\n");
	print_instructions();
	bzero(&msg,BUFSIZE);
	recv (sock, msg, BUFSIZE-1, 0);
	printf("%s\n",msg);
	while((msg[n++]=getchar())!='\n');
	msg[n-1]='\0';
	send (sock, msg, strlen(msg), 0);
	bzero(&msg,BUFSIZE);
	recv (sock, msg, BUFSIZE-1, 0);
	printf("all users are :\n%s\n",msg);

	strEcho(sock);
	close(sock);

	return 1;
}