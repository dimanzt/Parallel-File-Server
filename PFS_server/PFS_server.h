/*
 * PFS_server.h
 *
 *  Created on: Nov 4, 2012
 *
 */

#ifndef PFS_SERVER_H_
#define PFS_SERVER_H_

//Included Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

//Local Included Headers
#include "config.h"

//Local Defined Variables
#define CLIENT_CACHE_SIZE 2 // 2 Megabytes
#define ONEKB 1024
#define ONEMB 1024*1024

//Communications Management
#define CONNECTIONS_LIMIT 20
#define BUFFER_SIZE ONEKB*5
#define STDIN 0  // file descriptor for standard input
#define separator ' '//for protocol in packet
char myIP[NI_MAXHOST]; // or char myIP[INET6_ADDRSTRLEN]; //client's ip
char myPORT[5];// client's port
int myTFD; // TCP socket descriptor
int myUFD; // UDP socket descriptor
char mmIP[NI_MAXHOST];// = "127.0.0.1";//"172.16.231.207";//"130.203.100.56";
char mmPORT[5];// = "10000";//MM's IP&PORT
int mmFD;//MM's File Descriptor
typedef struct{
	char ip[NI_MAXHOST];// = "127.0.0.1";//"172.16.231.207";//"130.203.100.56";
	char port[5];// = "10000";//MM's IP&PORT
	int fd; //Meta-data Manager Socket Descriptor
} MM;
MM mm;
//typedef struct{ //File server specification
//	char ip[NI_MAXHOST]; //IP addresses of FSs
//	char port[5]; //Ports of FSs
//	int fd; //File Servers' file descriptor
//} CLIENT;
//CLIENT client[CONNECTIONS_LIMIT];
struct timeval tv;//for select() config
fd_set list;		// file descriptor list
fd_set list_copy;	// file descriptor list copy
int fdmax;        // maximum file descriptor number
unsigned fsnum; //Number of connected servers
int newfd;        // newly accept()ed socket descriptor
struct sockaddr_storage remoteaddr; // client address
char RemoteIP[INET6_ADDRSTRLEN];//client ip as a remote ip
socklen_t addrlen;//address length
char buffer[BUFFER_SIZE];//Server buffer
int NumBytes;//number of bytes in buffer
struct addrinfo Thints, Uhints;//, *ai, *p, *servinfo;
int yes=1;        // for setsockopt() SO_REUSEADDR, below
struct addrinfo *mm_ai;
int setup(int argc, char *argv[]);
unsigned str2int(char* str, int len);
int int2str(int i, char* str, int base);
int Connect2Server(char* ip, char* port, char* packet);
void getownip(char* host);
void* get_address(struct sockaddr *socketaddress); // Get socket address as IPv4 or IPv6
int findspace(char* str, int offset, int num);

typedef struct{
	char filename[20];
	//unsigned length;
	char length[20];
	char ip[INET6_ADDRSTRLEN];
	char port[10];
}chunk;
chunk db[100];
unsigned dbi;//database index

struct TData{
	int  tid;
	//int  fd;
	char *buf;
};

void* createfile(void* TD);
void* openfile(void* TD);
void* readfile(void* TD);
void* writefile(void* TD);
void* deletefile(void* TD);

#endif /* PFS_SERVER_H_ */
