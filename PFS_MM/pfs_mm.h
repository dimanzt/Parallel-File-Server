/*
 * pfs_mm.h
 *
 *  Created on: Nov 22, 2012
 *
 */

#ifndef PFS_MM_H_
#define PFS_MM_H_

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
//char mmIP[NI_MAXHOST];// = "127.0.0.1";//"172.16.231.207";//"130.203.100.56";
//char mmPORT[5];// = "10000";//MM's IP&PORT
int mmFD;//MM's File Descriptor
typedef struct{
	char ip[NI_MAXHOST];// = "127.0.0.1";//"172.16.231.207";//"130.203.100.56";
	char port[5];// = "10000";//MM's IP&PORT
	int fd; //Meta-data Manager Socket Descriptor
} MM;
MM mm;
typedef struct{ //File server specification
	char ip[NI_MAXHOST]; //IP addresses of FSs
	char port[5]; //Ports of FSs
	int fd; //File Servers' file descriptor
} FS;
FS fs[NUM_FILE_SERVERS];
struct timeval tv;//for select() config
fd_set list;		// file descriptor list
fd_set list_copy;	// file descriptor list copy
int fdmax;        // maximum file descriptor number
struct sigaction SigAct; //Signal Action
unsigned fsnum; //Number of connected servers
//int newfd;        // newly accept()ed socket descriptor
struct sockaddr_storage remoteaddr; // client address
char RemoteIP[INET6_ADDRSTRLEN];//client ip as a remote ip
socklen_t addrlen;//address length
char buffer[BUFFER_SIZE];//packet buffer
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
void sigchld_handler(int s);
unsigned str2int(char* str, int len);
unsigned min(unsigned i, unsigned j);
unsigned max(unsigned i, unsigned j);

#define MAX_FD_NUMBER 10
#define MAX_SIZE 1000
typedef struct{
	unsigned fsoffset; //from which fs start
	unsigned strip_width; // stripe_size=2 stripe_width=3 fsnum=4 -> 1,2 3,4 5,6 -,- | 7,8 9,10 11,12 -,- | ...
	int* fdarray;	//0 1 2 3 4    5 6 7    8 9
	int fdlist[MAX_SIZE];
	int fd[MAX_FD_NUMBER]; //client[0..4] -> 0..5..12..19..22..100
	unsigned start[MAX_FD_NUMBER]; //start block 0,5,12,19,22
	unsigned length[MAX_FD_NUMBER]; //number of blocks 5,7,7,3,78
	unsigned fdnum; //5
	//char ip[INET6_ADDRSTRLEN];
	//char port[10];
}Recipe;
//chunk db[100];
//unsigned dbi;//database index

typedef struct{
	char filename[20];
	unsigned size;
	time_t creation;
	time_t modification;
	Recipe rcp;
}FileMetaData;
#define MAX_FILE_NUMBER 100
FileMetaData* fmdarray[MAX_FILE_NUMBER];//??? hash algorithm for better search to find files
unsigned filenum;
unsigned StripeWidthStart;

struct TData{
	int  tid;
	//int  fd;
	char *buf;
};

pthread_mutex_t mutex;// = PTHREAD_MUTEX_INITIALIZER;

void* createfile(void* TD);
void* openfile(void* TD);
void* readfile(void* TD);
void* writefile(void* TD);
void* deletefile(void* TD);
void* getstat(void* TD);
void* release(void* TD);

#endif /* PFS_MM_H_ */

/*
Metadata Manager
The metadata manager is responsible for the storage and maintaining all the metadata associated
with a file. For this project, we will assume a flat file system consisting of only one root
directory. For all the files in the directory, you need to maintain the following metadata:
1. Filename
2. File size
3. Time of creation
4. Time of last modification
5. File recipe

Note that a given file in the PFS is striped across various file servers to facilitate parallel access.
The specifics of this file distribution are described by what is known as the file recipe. You are
free to abstract out the components of a file recipe as part of your system design. Note that the
block size and the stripe size will be a fixed value which will be provided to you as part of a
configuration file. The stripe width which defines how many nodes the file will be split across,
will be specified by the user when the file is created. When a client opens a file, the metadata
manager returns to the client the file recipe. Thus the client directly communicates to the file
servers to access file data.

Token Management Function
The metadata manager also performs the task of a token manager for ensuring consistency in the
system. The goal here is to guarantee single-node equivalent POSIX/UNIX consistency for file
system operations across the system. Every file system operation acquires an appropriate read or
write lock to synchronize with conflicting operations on other nodes before reading or updating
any file system data. For this project, you need to implement block-range locking in order to
allow parallel applications to write concurrently to different blocks of the same file. The token
manager coordinates locks by handing out lock tokens. There are two types of lock tokens that
are issued by the token manager: read tokens and write tokens. Multiple nodes can
simultaneously acquire read tokens for the same file-block. However, block-range tokens for
parallel writes are negotiated as follows. The first node to read/write a file acquires the block-
range token for the whole file (zero to infinity). As long as no other nodes require access to the
same file, all read and write operations are processed locally. When a second node begins writing
to the same file it will need to revoke at least part of the block-range token held by the first node.
When the first node receives a revoke request, it checks whether the file is still in use. If the file
has since been closed, the first node will give up the whole token, and the second node will then
be able to acquire a token covering the whole file. On the other hand, if the second node starts
writing to a file before the first node closes the file, the first node will relinquish only part of its
block-range token. If the first node is writing sequentially at offset o1 and the second node at
offset o2, the first node will relinquish its token from o2 to infinity (if o2 > o1) or from zero to o1
(if o2 < o1).
File Servers
There are a fixed number of file servers in the network which will be also be specified in the
configuration file provided to you. Every file in the PFS is striped across a number of these file
servers. You can simulate file striping by implementing each stripe using the native file system
in the file servers.
 */
