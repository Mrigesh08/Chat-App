#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>

//global variables
#define MAXPENDING 5
#define BUFSIZE 100
#define stringSize 20

key_t key_shm;
key_t key_shm_grp;
int shmid;
int shmgrpid;
struct node * node_arr;
struct grp *node_grp_arr;
int msgType=0;

struct node{
	char user_id[stringSize];
	int type;
	char of;
};

struct msgform{
	long mtype;
	char mtext[BUFSIZE];
	char sender_id[20];
};

struct grp{
	char grp_name[stringSize];
	unsigned int members;
};

unsigned int make_set_from_index(int index){
	return ((unsigned int)1<<(index-1));
}

int search_user_grp(struct grp g,int user_type){
	return (int)((g.members>>(user_type-1))%2);
}

unsigned int set_union(unsigned int a,unsigned int b){
	return (a | b);
}

void handler(int signo){
	msgType++;
}

struct node *make_node(char *user_id,int type,char of){
	struct node *temp=(struct node *)malloc(sizeof(struct node));
	int i=0;
	while(user_id[i]!='\0'){
		temp->user_id[i]=user_id[i];
		i++;
	}
	temp->user_id[i]='\0';
	temp->type=type;
	temp->of=of;
	return temp;
}

int search_user(struct node *arr,char *ch){
	int i=0;
	while(i<100 && arr[i].type!=-1){
		if(strcmp(arr[i].user_id,ch)==0){
			return arr[i].type;
		}
		
		i++;
	}
	return -1;
}

int search_grp(struct grp *arr,char *ch){
	int i=0;
	while(i<20 && arr[i].members!=0){
		if(strcmp(arr[i].grp_name,ch)==0){
			return i;
		}
		i++;
	}
	return -1;
}

void add_user(struct node *arr,char *user_id,int type,char of){
	int i=0;
	while(i<100 && arr[i].type!=-1){
		i++;
	}
	strcpy(arr[i].user_id,user_id);
	arr[i].type=type;
	arr[i].of=of;
}

int add_grp(struct grp *arr,char *grp_name){
	int i=0;
	while(i<20 && arr[i].members!=0){
		if(strcmp(arr[i].grp_name,grp_name)==0){
			return -1;
		}
		i++;
	}
	strcpy(arr[i].grp_name,grp_name);
	return i;
}

char * get_users(struct node *arr){
	if(arr[0].type==-1){
		return "none online";
	}
	else{
		char *str=(char *)malloc(sizeof(char)*2000);
		char str1[20];
		int i=0;
		while(i<100 && arr[i].type!=-1){
			sprintf(str1,"%s : %c   ",arr[i].user_id,arr[i].of);
			strcat(str,str1);
			i++;
		}
		return str;
	}
}

void broadcast(struct node *arr,char *sender,char *ch,int msgid){
	int sender_type=search_user(arr,sender);
	int i=0;
	struct msgform mf;
	strcpy(mf.mtext,ch);
	strcpy(mf.sender_id,sender);
	while(i<100 && arr[i].type!=-1){
		if(arr[i].type!=sender_type){
			mf.mtype=arr[i].type;
			msgsnd(msgid, &mf, sizeof(mf), IPC_NOWAIT);
		}
		i++;
	}
}

void message_to_grp(unsigned int members,char *sender,char *ch,int msgid){
	struct msgform mf;
	strcpy(mf.mtext,ch);
	strcpy(mf.sender_id,sender);
	for(int i=0;i<sizeof(unsigned int);i++){
		if( ((int)(members%2)) ==1){
			mf.mtype=i+1;
			msgsnd(msgid, &mf, sizeof(mf), IPC_NOWAIT);
		}
		members=members>>1;
	}
}

void mark_offline(struct node *arr,char *sender){
	int sender_type=search_user(arr,sender);
	int i=0;
	while(i<100 && arr[i].type!=-1){
		if(arr[i].type==sender_type){
			arr[i].of='x';
		}
		i++;
	}
}

void mark_online(struct node *arr,char *sender){
	int sender_type=search_user(arr,sender);
	int i=0;
	while(i<100 && arr[i].type!=-1){
		if(arr[i].type==sender_type){
			arr[i].of='o';
			break;
		}
		i++;
	}
}

char * trim(char *ch){
	char *temp=(char *)malloc(sizeof(char)*100);
	int i=0;
	while(ch[i]==' '){
		i++;
	}
	int j=strlen(ch)-1;
	while(ch[j]==' '){
		j--;
	}
	int k=0;
	while(i<=j){
		temp[k]=ch[i];
		k++;
		i++;
	}
	temp[k]='\0';
	return temp;
}

int main(){

	int serverSocket;
	struct sockaddr_in serverAddress, clientAddress;
	int bind_add;
	int clientLength;
	pid_t childPid;
	signal(SIGUSR1,handler);

	//creating message queue
	key_t key;
    int msgid;
    key = ftok("progfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);


	//creating SERVER SOCKET
	serverSocket = socket (AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0){
		printf ("Error while server socket creation\n");
		exit (0);
	}
	printf ("Server Socket Created\n");


	//setting the address structure for SERVER
	memset (&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(9000);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY
	printf ("Server address assigned\n");

	//binding SERVER to address
	bind_add = bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
	if (bind_add < 0){
		printf ("Error while binding\n");
		exit (0);
	}
	printf ("Binding successful\n");

	//listen call for SERVER
	int temp1 = listen(serverSocket, MAXPENDING);
	if (temp1 < 0){
		printf ("Error in listen\n");
		exit (0);
	}
	printf ("Now Listening\n");

	clientLength = sizeof(clientAddress);

	//************creating shared memory*************

	key_shm = ftok("/bin/bash",65);
	if(key_shm<0){
		printf("error in ftok %d\n",key_shm);
	}
	shmid = shmget(key_shm,100*(sizeof(struct node)),0666|IPC_CREAT);
	if(shmid<0){
		printf("shared memory not created\n");
		exit(0);
	}

	node_arr=(struct node *)shmat(shmid,NULL,0);
	if(node_arr==(struct node *)-1){
		printf("error\n");
		exit(0);
	}
	printf("%p\n",node_arr);

	key_shm_grp = ftok("/bin/ls",65);
	if(key_shm_grp<0){
		printf("error in ftok %d\n",key_shm_grp);
	}
	shmgrpid = shmget(key_shm_grp,20*(sizeof(struct grp)),0666|IPC_CREAT);
	if(shmgrpid<0){
		printf("shared group memory not created\n");
		exit(0);
	}

	node_grp_arr=(struct grp *)shmat(shmgrpid,NULL,0);
	if(node_grp_arr==(struct grp *)-1){
		printf("error\n");
		exit(0);
	}
	printf("%p\n",node_grp_arr);

	//****************shared memory creation done*******************

	for(int i=0;i<20;i++){
		node_grp_arr[i].members=0;
	}

	for(int i=0;i<100;i++){
		node_arr[i].type=-1;
		node_arr[i].of='$';
	}

	//****************server code*******************

	while(1){

		//accept call for all concurrent CLIENTS
		int clientSocket = accept (serverSocket, (struct sockaddr*) &clientAddress, &clientLength);
		if(clientSocket < 0){
			printf ("Error in accept\n");
		}
		printf("connection established with a client\n");


		//creating concurrent SERVER  by forking
		if((childPid = fork())<0){
			printf ("Error in creating child process\n");
		}
		else if(childPid==0){
			//************child process*************
			close(temp1);

			//********attaching shared memory in child************

			shmid = shmget(key_shm,100*(sizeof(struct node)),0666);
			if(shmid<0){
				printf("shared memory not created\n");
				exit(0);
			}

			struct node *node_arr2=(struct node *)shmat(shmid,NULL,0);
			if(node_arr2==(struct node *)-1){
				printf("error\n");
				exit(0);
			}

			shmgrpid = shmget(key_shm_grp,20*(sizeof(struct grp)),0666);
			if(shmgrpid<0){
				printf("shared group memory not created\n");
				exit(0);
			}

			struct grp *node_grp_arr2=(struct grp *)shmat(shmgrpid,NULL,0);
			if(node_grp_arr2==(struct grp *)-1){
				printf("error\n");
				exit(0);
			}

			//***********shared memory attachment done*************

			//variables
			char *msg_to_client;
			int send_to_type;
			int non_block_recv;
			char *pch=(char *)malloc(sizeof(char)*100);
			struct msgform mf,mf1;
			char * msg1=(char *)malloc(sizeof(char)*BUFSIZE);
			int msgrcv_return;
			char msg[BUFSIZE];
			char *online_users=(char *)malloc(sizeof(char)*200);
			int searchU,userType;
			struct node *n;

			//asking for user_name
			bzero(msg,BUFSIZE);
			strcpy(msg,"enter your username");
			send (clientSocket, msg, strlen(msg), 0);
			bzero(msg,BUFSIZE);
			recv (clientSocket, msg, BUFSIZE-1, 0);
			searchU=search_user(node_arr2,msg);
			if(searchU==-1){
				kill(getppid(),SIGUSR1);			
				userType=msgType+1;
				add_user(node_arr2,msg,userType,'o');
			}
			else{
				mark_online(node_arr2,msg);
				userType=searchU;
			}

			//sending list of online users
			online_users=get_users(node_arr2);
			send (clientSocket, online_users, strlen(online_users), 0);

			//storing own name and type
			char *child_user_id=(char *)malloc(sizeof(char)*20);
			strcpy(child_user_id,msg);
			int type_user=userType;

			//receiving sending begins for a concurrent server
			while(1){

				//non blocking sending process
				non_block_recv=recv (clientSocket, msg1, BUFSIZE-1, MSG_DONTWAIT);

				if(non_block_recv<0 && (errno==EWOULDBLOCK || errno==EAGAIN)){
					//do nothing(nothing is received from the client)
				}
				else if(non_block_recv<0){
					printf("error in receiving data\n");
				}
				else if(non_block_recv>0){

					if(strncmp(msg1,"exit",4)==0){
						printf("%s disconnected...\n",child_user_id);
						mark_offline(node_arr2,child_user_id);
						break;
					}
					else if(strncmp(msg1,"get_users",9)==0){
						online_users=get_users(node_arr2);
						send (clientSocket, online_users, strlen(online_users)+1, 0);
					}
					printf("message received is: %s\n",msg1);
					pch = strtok (msg1,":");
					pch=trim(pch);
					printf("first token : %s.\n",pch);
					//send_to=pch;
					if(strncmp(pch,"broad",5)==0){
						pch = strtok (NULL, ":");
						broadcast(node_arr2,child_user_id,pch,msgid);
					}
					else if(strncmp(pch,"mk_grp",6)==0){
						int user,grp_index;
						pch = strtok (NULL, ":");
						pch=trim(pch);
						printf("group name =%s\n",pch);

						if((grp_index=add_grp(node_grp_arr2,pch))!=-1){

							send_to_type=search_user(node_arr2,child_user_id);
							node_grp_arr2[grp_index].members=set_union(node_grp_arr2[grp_index].members,make_set_from_index(send_to_type));
							pch = strtok (NULL, ":");
							pch=trim(pch);
							printf("user in group= %s\n",pch);
							while(pch!=NULL){
								user=search_user(node_arr2,pch);
								if(user!=-1){
									node_grp_arr2[grp_index].members=set_union(node_grp_arr2[grp_index].members,make_set_from_index(user));
								}
								pch=strtok(NULL, ":");
								pch=trim(pch);
								printf("user in group= %s\n",pch);
							}

						}
						else{
							send(clientSocket, "group name already exist", strlen("group name already exist")+1, 0);
						}
					}
					else{
						int grp_num=search_grp(node_grp_arr2,pch);
						send_to_type=search_user(node_arr2,pch);
						if(grp_num!=-1){
							pch = strtok (NULL, ":");
							pch=trim(pch);
							message_to_grp(node_grp_arr2[grp_num].members,child_user_id,pch,msgid);
						}
						else if(send_to_type!=-1){
							pch = strtok (NULL, ":");
							pch=trim(pch);
							mf.mtype=send_to_type;
							strcpy(mf.mtext,pch);
							strcpy(mf.sender_id,child_user_id);
							msgsnd(msgid, &mf, sizeof(mf), IPC_NOWAIT);
							bzero(msg1,BUFSIZE);
						}
						else{
							send (clientSocket, "no such user or group", strlen("no such user or group")+1, 0);
						}
					}
				}

				//receiving the process from the message queue
				while(1){
					msgrcv_return=msgrcv(msgid, &mf1, sizeof(mf1), userType, IPC_NOWAIT | MSG_NOERROR );
					if(msgrcv_return<0 && errno==ENOMSG){
						break;
					}
					else if(msgrcv_return<0){
						printf("error in receiving from message queue\n");
						break;
					}
					else{
						msg_to_client=(char *)malloc(sizeof(mf1.sender_id)+sizeof(mf1.mtext)+sizeof(char)*4);
						sprintf(msg_to_client,"%s : %s",mf1.sender_id,mf1.mtext);
						send (clientSocket, msg_to_client, strlen(msg_to_client), 0);
					}
				}

			}
			//child process ends
			exit(0);
		}
		close(clientSocket);
	}

	return 1;
}