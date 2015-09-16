/*
 * PFS_client.h
 *
 *  Created on: Nov 4, 2012
 *
 */

#ifndef PFS_CLIENT_H_
#define PFS_CLIENT_H_

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
#define ONEKB 1024
#define ONEMB 1024*1024
#define CLIENT_CACHE_SIZE 2 // KB = 2 Megabytes
#define CLIENT_CACHE_LINES CLIENT_CACHE_SIZE*ONEMB/ONEKB

//Buffer or Cache Management
//int cache[CLIENT_CACHE_SIZE * ONEMB / sizeof(int)];
unsigned metacache[CLIENT_CACHE_LINES][4];//Cache line = 1KB
// We don't need real data only metadata!!!
// 2MB cache / 1KB block = 2000 = 2^11 lines or sets, 2^32 / 2M = 2^11 = 2K -> TAG = 11bit
// 0 -> int for (free=0,used=1,changed=2,dirty=3), 1 -> int for TAG, 2 -> int for LRU up to 100, 3 -> ofi
#define MAXRU 100
#define LowThr  25 //25%
#define HighThr 50 //50%
// Cache line?=1KB //could be direct mapped!
// Mem. Address Boundaries? //0000000 - FFFFFFFF -> 00000 - 1FFFFF (2MB)
int harvester();
int flusher();
int save2cache(unsigned fdi, unsigned os, unsigned len);
int readcache(unsigned fdi, unsigned os, unsigned len);
int MMR(char* buf, int bufsize);
int release(char* buf, int bufsize);

//Communications Management
#define BUFFER_SIZE ONEKB*5
#define STDIN 0  // file descriptor for standard input
#define separator ' '//for protocol in packet
char myIP[NI_MAXHOST]; // or char myIP[INET6_ADDRSTRLEN]; //client's ip
char myPORT[5];// client's port
int myFD; // client's socket descriptor!
char mmIP[NI_MAXHOST];// = "127.0.0.1";//"172.16.231.207";//"130.203.100.56";
char mmPORT[5];// = "10000";//MM's IP&PORT
int mmFD;//MM's File Descriptor
typedef struct{
	char ip[NI_MAXHOST];// = "127.0.0.1";//"172.16.231.207";//"130.203.100.56";
	char port[5];// = "10000";//MM's IP&PORT
	int fd; //Socket Descriptor
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
unsigned fsnum; //Number of connected servers
int newfd;        // newly accept()ed socket descriptor
int yes; // for setsockopt() SO_REUSEADDR, below
struct addrinfo hints;//Setup Communication
//struct sockaddr_storage remoteaddr; // client address
//socklen_t addrlen;
//char buffer[BUFFER_SIZE];    // buffer for client data
//char command[100]; //command packet
//int NumBytes;
//char RemoteIP[INET6_ADDRSTRLEN];
unsigned str2int(char* str, int len);
int int2str(int i, char* str, int base);
int Connect2Server(char* ip, char* port);
void getownip(char* host);
void* get_address(struct sockaddr *socketaddress); // Get socket address as IPv4 or IPv6
int findspace(char* str, int offset, int num);
int Listen2Clients();

#define MAX_OPEN_FILES 10
typedef struct{
	int fd;
	char filename[20];
	char mode[5];
	//unsigned stripe_size;
	unsigned stripe_width;
	unsigned fsoffset;
}OpenFile;
OpenFile* openfiles[MAX_OPEN_FILES];
unsigned ofnum;
unsigned fdnum;

//Functions
int initialize(int argc, char *argv[]);
int finalize();
unsigned min(unsigned i, unsigned j);

//Project's Default Functions
int pfs_create(const char *filename, int stripe_width);
int pfs_open(const char *filename, const char *mode);
ssize_t pfs_read(int filedes, void *buf, ssize_t nbyte, off_t offset, int *cache_hit);
ssize_t pfs_write(int filedes, const void *buf, size_t nbyte, off_t offset, int *cache_hit);
int pfs_close(int filedes);
int pfs_delete(const char *filename);
int pfs_fstat(int filedes, struct pfs_stat *buf);// Check the config.h file for the definition of pfs_stat structure

#endif /* PFS_CLIENT_H_ */

/*
In this project, your will implement a simple Parallel File System (PFS) which provides access to
file data for parallel applications. The following figure gives an overview of a simple PFS and its
components.
File Servers
Metadata
Manager
FS1
FS2
FS3
...
FSn
Client Node
File System Interface
Client Cache
File Operations
- Create()
- Open()
- Read()
- Write()
- Seek()
- Close()
- Delete()
Free Space
Used Space
Modified
Not Modified
Like most other files systems, the PFS is also designed as client-server architecture with multiple
servers which typically run on separate nodes and have disks attached to them. Each file in the
PFS system is striped across the disks on the file servers and the application processes interact
with the file system using a client library. The PFS also has a Metadata Manager which handles
all the metadata associated with files. The Metadata Manager does not take part in the actual
read/write operations. The client, file servers and the metadata manager need not run on different
machines though running them on different machines generally results in higher performance.
As part of your project, you will be implementing all the four primary components of a simple
PFS:
1. Metadata Manager
2. File Servers
3. Client
4. Client Cache
A brief description of all the components is given below.

 */
