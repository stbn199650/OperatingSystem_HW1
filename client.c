#include "client.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define MAX_SIZE 500
int main(int argc, char **argv)
{
	//create connection to server
	int sock_desc;
	struct sockaddr_in serv_addr;

	//Create Socket
	sock_desc = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_desc < 0) {
		printf("Failed to create socket\n");
		return 1;
	}
	puts("Socket Created\n");
	serv_addr.sin_family = AF_INET;     //ipv4 connection
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(8888);

	//connect to server
	if (connect(sock_desc, (struct sockaddr *) &serv_addr,
	            sizeof (serv_addr)) < 0) {
		perror("Failed to connect to server");
		return 1;
	}
	puts("Connect to server successfully");

	char ex, msg[10]= {'0','0','0','0','0','0','0','0','0','0'};
	int pid = 1, a = 0, i = 0, x = 0, n = 0, num = 0;
	char server_reply[MAX_SIZE], multi_reply[10];

	//query interface
	while(1) {
		for(i=0; i<8; i++)
			msg[0]='0';
		pid=0;

		printf("=====================================================\n");
		printf("(a)list all process ids\n(b)thread's IDs\n");
		printf("(c)child's PIDs\n(d)process name\n");
		printf("(e)state of process(D,R,S,T,t,W,X,Z\n");
		printf("(f)command line of executing process(cmdline)\n");
		printf("(g)parent's PID\n(h)all ancients of PIDs\n");
		printf("(i)virtual memory size(VmSize)\n");
		printf("(j)physical memory size(VmRSS)\n(k)exit\n");
		printf("Which? ");
		scanf("%c%c", &msg[0],&ex);

		a = 0;
		if((int)msg[0] > 97 && (int)msg[0] < 107) { //b~j
			printf("pid? ");
			scanf("%d%c", &pid,&ex);
			if(pid<10 && pid>=0)    //get pid length
				a=1;
			else if(pid<100 && pid>9)
				a=2;
			else if(pid<1000 && pid>99)
				a=3;
			else if(pid<10000 && pid>999)
				a=4;
			else if(pid<100000 && pid>9999)
				a=5;
			else if(pid<1000000 && pid>99999)
				a=6;
			else if(pid<10000000 && pid>999999)
				a=7;
			else {
				a=0;
				pid=0;
				continue;
			}
			//concatenate send message
			sprintf(msg,"%c%d%d",msg[0],a,pid);
		}

		//not in a~k
		if((int)msg[0] <97 || (int)msg[0] > 107) {
			printf("\nPlease Enter Again\n");
			continue;
		}
		if(msg[0]=='k') {   //input==k,close socket
			close(sock_desc);
			printf("Connection Closed\n");
			exit(0);
		}
		//send message to server
		if(send(sock_desc,msg,strlen(msg),0)<0) {
			puts("Send message Failed");
			return 1;
		}
		for(i=0; i<500; i++)
			server_reply[i]='?';

		//------------------------------------//
		if(msg[0]=='b' || msg[0]=='d' || msg[0]=='e' || msg[0]=='f' || msg[0]== 'g'
		   || msg[0]=='i' || msg[0]=='j') {

			//receive message failed
			if(recv(sock_desc,server_reply,500,0)<0) {
				puts("\nRecv Failed");
				continue;
			}
			//the pid doesn't exist
			if(server_reply[0] == '#') {
				puts("\nNo such pid");
				continue;
			}
			if(server_reply[0] == '@') {
				puts("\nNo Command Line");
				continue;
			}

			if(msg[0] == 'b')
				printf("\n[tid] ");
			else if(msg[0] == 'd')
				printf("\n[Process Name] ");
			else if(msg[0] == 'e')
				printf("\n[Process State] ");
			else if(msg[0] == 'f') {
				printf("\n[Cmdline] ");
				for(i=0; i<500; i++) {
					if(server_reply[i] != '?')
						printf("%c",server_reply[i]);
					else
						break;
				}
				printf("\n");
			} else if(msg[0] == 'g')
				printf("\n[Parent's Pid] ");
			else if(msg[0] == 'i')
				printf("\n[VmSize](kB) ");
			else if(msg[0] == 'j')
				printf("\n[VmRSS](kB) ");

			//print msg from server
			if(msg[0] != 'f')
				puts(server_reply);

		} else { //handle multiple return(a,i,j)

			//receive message
			if(recv(sock_desc,server_reply,10,0)<0) {
				puts("Recv Failed");
				break;
			}
			//the pid doesn't exist
			if(server_reply[0] == '#') {
				puts("\nNo such pid");
				continue;
			}
			//server_reply[0] record total amount of pid
			char temp_reply[10];
			if(msg[0] == 'a') {
				puts("\n[All Pids]");
				num = atoi(server_reply);
				if(num!=0)
					recv(sock_desc,temp_reply,2000,0);
				puts(temp_reply);
				//send(sock_desc,server_reply,10,0);

				continue;
			} else if(msg[0] =='c') {
				if(server_reply[0] == '@') {
					puts("\nNo Child Pid");
					continue;
				}
				num = atoi(server_reply);
				puts("\n[Child's Pid]");
				for(i=0; i<num; i++) {
					//get message from server
					recv(sock_desc,temp_reply,10,0);
					/*if((recv(sock_desc,temp_reply,10,0))<0) {
						puts("no message receive\n");
						continue;
					}*/
					puts(temp_reply);
					//send message to server to avoid unsynchronized
					send(sock_desc,temp_reply,10,0);
				}
				continue;
			} else if(msg[0] == 'h') {

				num=atoi(server_reply);
				puts("\n[Ancients of pid]");
				for(i=0; i<num; i++) {
					//get message from server
					recv(sock_desc,temp_reply,10,0);
					/*if((recv(sock_desc,temp_reply,10,0))<0) {
						puts("no message receive\n");
						continue;
					}*/
					puts(temp_reply);
					//send message to server to avoid unsynchronized
					send(sock_desc,temp_reply,10,0);
				}
				continue;
			}
		}
	}

	close(sock_desc);

	return 0;
}

