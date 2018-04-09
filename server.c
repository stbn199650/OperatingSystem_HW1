#include "server.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>      //strlen
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>   //inet_addr
#include<unistd.h>     //write
#include<pthread.h>
#include<dirent.h>

#define MAX_SIZE 500
void *connection_handler(void *socket_desc);
int main(int argc, char **argv)
{
	// write someting here...
	int sock_desc, client_sock, c, *new_sock, bnd;
	char *msg;
	struct sockaddr_in server, client;

	//create socket
	sock_desc = socket(AF_INET,SOCK_STREAM,0);
	if(sock_desc < 0) {
		perror("Couldn't create socket");
		return '1';
	}
	puts("Socket Created");

	//define address of socket
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	//bind
	if(bind(sock_desc, (struct sockaddr *)&server,sizeof(server))<0) {
		perror("bind failed");
		return 1;
	}
	puts("bind success");

	//listen
	listen(sock_desc, 3);
	puts("Waiting for incoming connection...");
	c = sizeof(struct sockaddr_in);

	//accept connection from an incoming client
	while(client_sock=accept(sock_desc, (struct sockaddr *)&client,
	                         (socklen_t*)&c)) {
		puts("Connection accepted");
		pthread_t sniffer_thread;
		new_sock = (int*)malloc(sizeof(int));
		*new_sock = client_sock;

		if(pthread_create( &sniffer_thread, NULL,  connection_handler,
		                   (void*) new_sock) < 0) {
			perror("could not create thread");
			return 1;
		}//puts("Thread Created");
	}

	if(client_sock<0) {
		perror("accept failed");
		return 1;
	}

	return 0;
}

//handle connection for each client
void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int count = 0, i = 0, j = 0, x = 0;
	int pid = 0, n = 0, VmSize = 0, VmRSS = 0;
	int parent_pid = 0, value=0;
	int pid_length[500];

	char  file_name[40];
	char process_name[100];
	char char_pid[8], state,get_nothing[10],pid_dir[500][10];
	char ex_data[2][100], client_message[7];
	char re[2][100], cmd[500], multi_data[500][10],path[100];
	char all_pid[2000];

	FILE *fp;
	DIR *dir;
	struct dirent *ent = NULL;

	//receive message from client
	while((n=recv(sock,client_message,8,0)) > 0) {

		//handle pid in b to j
		if(((int)client_message[0]) > 97 && ((int)client_message[0]) < 107) {
			//sprintf concatenate pid
			if(client_message[1] == '1') {
				sprintf(char_pid,"%c",client_message[2]);
				pid = atoi(char_pid);   //atoi turn string into int
			} else if(client_message[1] == '2') {
				sprintf(char_pid,"%c%c",client_message[2],client_message[3]);
				pid = atoi(char_pid);
			} else if(client_message[1] == '3') {
				sprintf(char_pid,"%c%c%c",client_message[2],client_message[3],
				        client_message[4]);
				pid = atoi(char_pid);
			} else if(client_message[1] == '4') {
				sprintf(char_pid,"%c%c%c%c",client_message[2],client_message[3],
				        client_message[4],client_message[5]);
				pid = atoi(char_pid);
			} else if(client_message[1] == '5') {
				sprintf(char_pid,"%c%c%c%c%c",client_message[2],client_message[3],
				        client_message[4],client_message[5],client_message[6]);
				pid = atoi(char_pid);
			} else if(client_message[1] == '6') {
				sprintf(char_pid,"%c%c%c%c%c%c",client_message[2],client_message[3],
				        client_message[4],client_message[5],client_message[6],client_message[7]);
				pid = atoi(char_pid);
			} else if(client_message[1] == '7') {
				sprintf(char_pid,"%c%c%c%c%c%c%c",client_message[2],client_message[3],
				        client_message[4],client_message[5],client_message[6],client_message[7],
				        client_message[8]);
				pid = atoi(char_pid);
			} else {
				re[0][0]='#';
				send(sock,re[0],2,0);
				continue;
			}
			printf("pid= %d\n",pid);
		}

		x = 0;  //initialize
		count = 0;
		for(i=0; i<2; i++)
			for(j=0; j<100; j++)
				re[i][j]='!';

		if(client_message[0]=='a' || client_message[0]=='c' || client_message[0]=='h') {
			for(i=0; i<500; i++)
				for(j=0; j<10; j++)
					multi_data[i][j]='!';
			for(i=0; i<500; i++) //length of pid
				pid_length[i]=0;

			switch(client_message[0]) {
			case 'a':
				for(i=0; i<MAX_SIZE; i++)
					for(j=0; j<10; j++)
						pid_dir[i][j]='!';
				for(i=0; i<2000; i++)
					all_pid[i]='!';

				sprintf(path,"/proc");   //all pid is under /proc
				dir = opendir(path);    //open directory
				if(!dir) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}
				count = 0;  //amount of pid
				printf("[all process id]\n");
				while((ent=readdir(dir)) != NULL) { //get directory name
					if((int)(ent->d_name[0])>47 && (int)(ent->d_name[0]<58)) {
						//if dir name have number, it's pid,store
						if(count==0)
							sprintf(all_pid,"%s",ent->d_name);
						else
							sprintf(all_pid,"%s\t%s",all_pid,ent->d_name);

						//puts(ent->d_name);
						count++;
					}
				}
				sprintf(re[0],"%d",count);//cast count to char

				//if dir exist, server send pid amount to client 1st
				send(sock,re[0],10,0);
				send(sock,all_pid,2000,0);

				printf("count= %d\n",count);
				closedir(dir);
				break;
			case 'c':
				sprintf(path,"/proc/%d/task/%d/children",pid,pid);
				//cast pid from int to char
				fp = fopen(path,"r");    //open directory
				if(!fp) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}
				count=0;
				while((fscanf(fp,"%d",&value)) != EOF) {
					sprintf(multi_data[count],"%d",value);
					for(i=0; i<10; i++) {
						if(multi_data[count][i]!='!') {
							pid_length[count]++;
						}
					}
					count++;
				}
				if(count == 0) {
					re[0][0]='@';
					send(sock,re[0],2,0);
					continue;
				}
				sprintf(re[0],"%d",count); //total amount of pid
				send(sock,re[0],10,0);
				for(i=0; i<count; i++) {
					//send children pid to client
					send(sock,multi_data[i],pid_length[i],0);
					//receive message from client to avoid unsynchronized
					recv(sock,get_nothing,10,0);
					//if server just keep send message without wait for client will cause error
				}

				printf("count=%d\n",count);
				fclose(fp);
				break;
			case 'h':   //ancients of pid
				sprintf(path,"/proc/%d/stat",pid);
				fp = fopen(path,"r");
				if(!fp) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}
				count=0;
				//get all ancient's pid
				do {
					fscanf(fp,"%d %s %c %d",&x,ex_data[0],&state,&parent_pid);
					//get parent's pid
					sprintf(multi_data[count],"%d",parent_pid);
					for(i=0; i<10; i++) {
						if(multi_data[count][i]!='!') {
							++pid_length[count];
						}
					}
					count++;
					fclose(fp);

					//parent's file name
					sprintf(path,"/proc/%d/stat",parent_pid);
					puts(path);
					fp = fopen(path,"r");
				} while(fp);

				sprintf(re[0],"%d",count);
				send(sock,re[0],10,0);
				for(i=0; i<count; i++) {
					//send ancient pid to client
					send(sock,multi_data[i],pid_length[i],0);
					//receive message from client to avoid unsynchronized
					recv(sock,get_nothing,10,0);
				}

				break;
			}

		} else { //one return string
			//handle event
			switch(client_message[0]) {

			case 'b': //tid
				sprintf(path,"/proc/%d/task",pid);  //cast pid from int to char
				dir = opendir(path);    //open directory
				if(!dir) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}
				while((ent=readdir(dir)) != NULL) { //get directory name
					x++;
					if(x==3) {
						sprintf(re[0],"%s",ent->d_name);
						//cast d_name into char
						printf("[tid] %s\n",re[0]);
					}
				}

				for(i=0; i<100; i++) { //how many element re[][] used
					if(re[0][i] != '!')
						count++;
					else
						break;
				}

				//send message back to client
				send(sock,re[0],count,0);
				closedir(dir);
				break;
			case 'd':   //process name
				sprintf(file_name,"/proc/%d/status",pid);
				fp = fopen(file_name,"r");
				if(!fp) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}

				for(i=0; i<100; i++)
					process_name[i]='!';
				fscanf(fp,"%s %s",re[0],process_name);
				printf("[process name] %s\n",process_name);
				for(i=0; i<100; i++) {
					if(process_name[i] != '!')
						count++;
				}

				//send message back to client
				send(sock,process_name,count,0);
				fclose(fp);
				break;

			case 'e':   //state of process
				sprintf(file_name,"/proc/%d/stat",pid);
				fp = fopen(file_name,"r");
				if(!fp) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}

				fscanf(fp,"%d %s %c",&x,ex_data[0],&state);
				//printf("state=%s\n",re[0]);
				sprintf(re[0],"%c",state);

				send(sock,re[0],2,0);
				fclose(fp);
				break;
			case 'f':   //command line
				sprintf(file_name,"/proc/%d/cmdline",pid);
				//cast pid to char, concatenate file name
				fp = fopen(file_name,"r");    //open file
				if(!fp) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}
				for(i=0; i<500; i++)
					cmd[i] = '!';
				if((fscanf(fp,"%s",cmd)) != EOF) {
					x++;
				}
				if(x==0) {  //no cmd in pid
					re[0][0]='@';
					send(sock,re[0],2,0);
					continue;
				}
				for(i=0; i<500; i++)
					if(cmd[i] != '!')
						count++;

				send(sock,cmd,count,0);
				fclose(fp);
				break;
			case 'g':   //parent's pid
				sprintf(file_name,"/proc/%d/stat",pid);
				fp = fopen(file_name,"r");
				if(!fp) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}
				fscanf(fp,"%d %s %c %d",&x,ex_data[0],&state,&parent_pid);
				printf("parent pid=%d\n",parent_pid);
				sprintf(re[0],"%d",parent_pid);
				for(i=0; i<100; i++) {
					if(re[0][i] != '!')
						count++;
				}

				send(sock,re[0],count,0);
				fclose(fp);
				break;
			case 'i':   //VmSize
				sprintf(file_name,"/proc/%d/statm",pid);
				fp = fopen(file_name,"r");
				if(!fp) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}
				fscanf(fp,"%d",&VmSize);
				VmSize*=4;
				printf("VmSize= %d kB\n",VmSize);
				sprintf(re[0],"%d",VmSize);

				for(i=0; i<100; i++)
					if(re[0][i] != '!')
						count++;

				send(sock,re[0],count,0);
				fclose(fp);
				break;
			case 'j':   //VmSize
				sprintf(file_name,"/proc/%d/statm",pid);
				fp = fopen(file_name,"r");
				if(!fp) {
					re[0][0]='#';
					send(sock,re[0],2,0);
					continue;
				}
				fscanf(fp,"%d %d",&VmSize,&VmRSS);
				VmRSS*=4;
				printf("VmRSS= %d kB\n",VmRSS);
				sprintf(re[0],"%d",VmRSS);

				for(i=0; i<100; i++)
					if(re[0][i] != '!')
						count++;

				send(sock,re[0],count,0);
				fclose(fp);
				break;
			}
		}
	}

	//close connection
	if(n == 0)
		puts("Client Disconnected");
	else if(n == -1)
		perror("recv failed");
	close(sock);
	free(socket_desc);

	return 0;
}


