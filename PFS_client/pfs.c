/*
 * PFS_client.c
 *
 *  Created on: Nov 4, 2012
 *
 */

#include "pfs.h"

int harvester()
{
/*Client Cache
The PFS uses local cache on each client having a default size of 2 MB. The client cache consists
of (1) free space and (2) used space. The used space in the client cache can further be divided
into blocks that have been modified (i.e. dirty blocks which have not been flushed to the server
yet) or not.
*/
	int i;
	int RU[MAXRU+1];//We know that it's under LowThr
	int CLs = CLIENT_CACHE_SIZE * ONEMB / ONEKB;
	for(i=0;i<CLs;i++)
		RU[i] = 0;
	//int cache[CLIENT_CACHE_SIZE * ONEMB / ONEKB][3];
	for(i=0;i<CLs;i++)
		if(metacache[i][0] < 2) //not changed
			RU[((metacache[i][2]<MAXRU)?metacache[i][2]:MAXRU)]++;
	int sum = RU[0];
	int LRUi = 1;
	while((LRUi<=MAXRU)&&((CLs*HighThr/100)>sum))
	{
		sum += RU[LRUi];
		LRUi++;
	}
	if(LRUi<=MAXRU)
	{
		for(i=0;i<CLs;i++)
			if(metacache[i][0] == 1) //used
			{
				metacache[i][0] = 0;//free
				metacache[i][1] = 0;//TAG=0
				metacache[i][2] = 0;//RU=0
			}
	}
/*
As part of the cache management scheme, you need to implement the following two threads:
1. Harvesters Thread: The job of the harvester thread is to maintain sufficient amount of
free space in the client cache. Whenever the free space falls below a specific lower
threshold, the harvester thread frees up memory from the used space based on an LRU
policy till the amount of free space reaches an upper threshold.

If the client has the appropriate tokens, and a read/write call issued by the client results in a
cache hit, the operations should be performed on locally on the client cache. On the other hand,
whenever there is a cache miss, the blocks need to be fetched from the file server which will then
be cached locally on the client using the available free space. The client returns the information
about a cache hit or miss for every read/write call using the Boolean argument cache_hit which
is passed as a parameter to both the pfs_read() and pfs_write()function calls. If the client
needs to acquire fresh tokens from the token manager for a read/write call, the file data need to
be fetched from the file servers. Whenever a client is required to relinquish a part of its token, it
needs to flush the dirty blocks to the file servers before relinquishing the tokens to the token
manager.*/
	return 0;
}

int flusher()
{
	int i;
	for(i=0;i<CLIENT_CACHE_LINES;i++)
		if(metacache[i][0] == 2) //changed or dirty
		{//flush(cache[i]);
			char buf[ONEKB];
			int ofi = metacache[i][3];
			OpenFile* of = openfiles[ofi];
			int cachehit;
			pfs_write(of->fd, buf, ONEKB, metacache[i][1]*ONEKB, &cachehit);
		}
//	2. Flusher Thread: The job of the flusher thread is to periodically (every 30 seconds) flush
//	all the dirty blocks (modified blocks) back to the appropriate file servers.
//	Note that you will have to implement a hashing scheme to map and look up the file blocks onto
//	the cache.
	return 0;
}

//Send packet func
//Receive Packet func
//Switch or Multithread???

int pfs_create(const char *filename, int stripe_width)
{//CREATE project2 4 END.
	char spacket[BUFFER_SIZE] = ""; //Buffer for client data
	char len[10];
	strcat(spacket,"CREATE ");
	strcat(spacket, filename);
	strcat(spacket, " ");
	int2str(min(stripe_width,fsnum),len,10);
	strcat(spacket, len);
	strcat(spacket," END.");
	if (send(mmFD, spacket, sizeof(spacket), 0) == -1)//CREATE pfs_file1 3
		perror("Error in sending!");
	char rpacket[BUFFER_SIZE] = ""; //Buffer for client data
	//int size =
	MMR(rpacket, BUFFER_SIZE);

	char buf[BUFFER_SIZE]; //Buffer for client data
	char FileName[20];
	int fnI = findspace(rpacket, 0, 1); //project2_2
	int osI = findspace(rpacket, fnI, 1); //0
	int swI = findspace(rpacket, osI, 1); //4
	int sI = findspace(rpacket, swI, 1); //0
	//int lI = findspace(rpacket, sI, 1); //0
	strncpy(FileName, rpacket+fnI, osI-fnI-1);
	FileName[osI-fnI-1] = '\0';
	int os = str2int(rpacket+osI,swI-osI-1);
	int sw = str2int(rpacket+swI, sI-swI-1);
	strcpy(buf, "CREATE ");
	strcat(buf, FileName);
	strcat(buf, " END.");
	int i;
	for(i=0; i<sw; i++)
	{//Send for each FSs -> 2 CREATE pfs_file1 0,1,2,3
		if (send(fs[(i+os)%sw].fd, buf, sizeof(buf), 0) == -1)
			perror("Error in sending!");
	}
	return 0;
}

int pfs_open(const char *filename, const char *mode)
{
	char spacket[BUFFER_SIZE]; //Buffer for client data
	//char len[10];
	strcpy(spacket,"OPEN ");
	strcat(spacket, filename);
	strcat(spacket, " ");
	strcat(spacket, mode);
	//int2str(stripe_width,len,10);
	//strcat(spacket, len);
	strcat(spacket," END.");
	if (send(mmFD, spacket, sizeof(spacket), 0) == -1)	//OPEN pfs_file1 w
		perror("Error in sending!");
	char rpacket[BUFFER_SIZE]; //Buffer for client data
	//int size =
	MMR(rpacket, BUFFER_SIZE);

	char buf[BUFFER_SIZE]; //Buffer for client data
	char FileName[20];
	char Mode[5];
	int fnI = findspace(rpacket, 0, 1); //project2_2
	int osI = findspace(rpacket, fnI, 1); //0
	int swI = findspace(rpacket, osI, 1); //4
	int sI = findspace(rpacket, swI, 1); //0
	int lI = findspace(rpacket, sI, 1); //0
	int mI = findspace(rpacket, lI, 1); //w
	int endI = findspace(rpacket, mI, 1);
	strncpy(FileName, rpacket+fnI, osI-fnI-1);
	FileName[osI-fnI-1] = '\0';
	strncpy(Mode, rpacket+mI, endI-mI-1);
	Mode[endI-mI-1] = '\0';
	int os = str2int(rpacket+osI,swI-osI-1);
	int sw = str2int(rpacket+swI, sI-swI-1);
	strcpy(buf, "OPEN ");
	strcat(buf, FileName);
	strcat(buf, " ");
	strcat(buf, Mode);
	strcat(buf, " END.");
	int i;
	for(i=0; i<sw; i++)
	{//Send for each FSs -> OPEN pfs_file1 w END.
		//if (!fork())
		//{ // this is the child process
		//	close(myTFD); // child doesn't need the listener
		if (send(fs[(i+os)%sw].fd, buf, sizeof(buf), 0) == -1)
			perror("Error in sending!");
		//	close(newfd);
		//	exit(0);
		//}
	}
	OpenFile* of = malloc(sizeof(OpenFile));
	strcpy(of->filename, FileName);
	strcpy(of->mode, Mode);
	of->fd = fdnum;
	fdnum++;
	of->fsoffset = os;
	of->stripe_width = sw;
	openfiles[ofnum] = of;
	ofnum++;
	return of->fd;
}

ssize_t pfs_read(int filedes, void *buf, ssize_t nbyte, off_t offset, int *cache_hit)
{//READ filename w offset length END.
	*cache_hit = readcache(filedes, offset, nbyte);//what's the usage of filedes here????
	if(*cache_hit)
		return nbyte;//cachehit
	int ofI = 0;
	while((ofI < ofnum)&&(openfiles[ofI]->fd != filedes))
		ofI++;
	if(ofI == ofnum)
		return -1;
	OpenFile* of = openfiles[ofI];
	char spacket[BUFFER_SIZE]; //Buffer for client data
/*
	char Offset[7];
	char Length[7];
	strcat(spacket,"READ ");
	strcat(spacket, of->filename);
	int2str(offset/ONEKB,Offset,10);
	int2str(nbyte/ONEKB,Length,10);
	strcat(spacket, Offset);
	strcat(spacket, " ");
	strcat(spacket, Length);
	strcat(spacket, " ");
	strcat(spacket," END.");
	if (send(mmFD, spacket, sizeof(spacket), 0) == -1)	//READ pfs_file1 offset length END.
		perror("Error in sending!");
	char rpacket[BUFFER_SIZE] = ""; //Buffer for client data
	int size = MMR(rpacket, BUFFER_SIZE);//READ filename offset length END.
*/
	//offset ... nbyte
	//start = fsoffset + offset/(size*ONEKB), offset -> fsoffset + (offset+length)/(size*ONEKB)
	//first: offset % (size*ONEKB) -> min(offset+length, size*ONEKB)
	//intermediates:
	//last: 0 -> (offset+length)%(size*ONEKB)
	unsigned fsstart, from, to, plen;
	unsigned os = offset;
	unsigned len = nbyte;
	int i;
	for(i= of->fsoffset + (offset/(STRIP_SIZE*ONEKB)); (i-of->fsoffset)*(STRIP_SIZE*ONEKB) < (offset+nbyte); i++)
	{//Send for each FSs -> READ pfs_file1 w from to(plen) END.
		//if (!fork())
		//{ // this is the child process
		//	close(myTFD); // child doesn't need the listener
		strcpy(spacket,"READ ");
		strcat(spacket, of->filename);
		strcat(spacket, " ");
		strcat(spacket, of->mode);
		strcat(spacket, " ");
		//0 ,1 ,2 ,3  + 4 ,5 ,6 ,7  + 8 ,9 ,10,11
		//12,13,14,15 + 16,17,18,19 + 20,21,22,23
		//24,25,26,27 + 28,29,30,31 + 32,33,34,35
		//os=19->32 -> l=13, i=4, sw=3,
		fsstart = (i/of->stripe_width)*(STRIP_SIZE*ONEKB);
		from = fsstart + os%(STRIP_SIZE*ONEKB);
		to = fsstart + min(os%(STRIP_SIZE*ONEKB)+len, STRIP_SIZE*ONEKB);
		plen = to - from;
		char From[10];
		char PLen[10];
		int2str(from, From, 10);
		int2str(plen, PLen, 10);
		strcat(spacket, From);
		strcat(spacket, " ");
		strcat(spacket, PLen);
		strcat(spacket, " END.");
		if (send(fs[i%of->stripe_width].fd, spacket, sizeof(spacket), 0) == -1)
			perror("Error in sending!");
		//	close(newfd);
		//	exit(0);
		//}
		int size;
		char rpacket[plen]; //Buffer for client data
		if ((size = recv(fs[i%of->stripe_width].fd, rpacket, sizeof(rpacket), 0)) == -1)
			perror("Error in receiving!");
		rpacket[size] = '\0';
		printf("Server: %s\n",rpacket);
		strncat(buf, rpacket, plen);
		save2cache(filedes,os,plen);
		os += plen;
		len -= plen;
	}
	return nbyte;
}

ssize_t pfs_write(int filedes, const void *buf, size_t nbyte, off_t offset, int *cache_hit)
{//WRITE filename w offset length END.
	int ofI = 0;
	while((ofI < ofnum)&&(openfiles[ofI]->fd != filedes))
		ofI++;
	if(ofI == ofnum)
		return -1;
	OpenFile* of = openfiles[ofI];
	//save2cache(filedes,os,plen);

	char spacket[BUFFER_SIZE]; //Buffer for client data
	char Offset[7];
	char Length[7];
	strcpy(spacket,"WRITE ");
	strcat(spacket, of->filename);
	strcat(spacket, " ");
	strcat(spacket, of->mode);
	strcat(spacket, " ");
	int2str(offset/ONEKB,Offset,10);
	int2str(nbyte/ONEKB,Length,10);
	strcat(spacket, Offset);
	strcat(spacket, " ");
	strcat(spacket, Length);
	strcat(spacket," END.");
	if (send(mmFD, spacket, sizeof(spacket), 0) == -1)	//OPEN pfs_file1 w Offset Length END.
		perror("Error in sending!");
	char rpacket[BUFFER_SIZE] = ""; //Buffer for client data
	//int size =
	MMR(rpacket, BUFFER_SIZE);//WRITE filename fsoff sw offset length END.
	//int fnI = findspace(rpacket, 0, 1); //project2_2
	//int osI = findspace(rpacket, fnI, 1); //0
	//int swI = findspace(rpacket, osI, 1); //4
	//int sI = findspace(rpacket, swI, 1); //0
	//int lI = findspace(rpacket, sI, 1); //0
	//int endI = findspace(rpacket, lI, 1); //w

	//offset ... nbyte
	//start = fsoffset + offset/(size*ONEKB), offset -> fsoffset + (offset+length)/(size*ONEKB)
	//first: offset % (size*ONEKB) -> min(offset+length, size*ONEKB)
	//intermediates:
	//last: 0 -> (offset+length)%(size*ONEKB)
	unsigned fsstart, from, to, plen;
	unsigned os = offset;
	unsigned len = nbyte;
	int i;
	for(i= of->fsoffset + (offset/(STRIP_SIZE*ONEKB)); (i-of->fsoffset)*(STRIP_SIZE*ONEKB) < (offset+nbyte); i++)
	{//Send for each FSs -> WRITE pfs_file1 from to(plen) END.
		//if (!fork())
		//{ // this is the child process
		//	close(myTFD); // child doesn't need the listener
		strcpy(spacket,"WRITE ");
		strcat(spacket, of->filename);
		strcat(spacket, " ");
		//0 ,1 ,2 ,3  + 4 ,5 ,6 ,7  + 8 ,9 ,10,11
		//12,13,14,15 + 16,17,18,19 + 20,21,22,23
		//24,25,26,27 + 28,29,30,31 + 32,33,34,35
		//os=19->32 -> l=13, i=4, sw=3,
		fsstart = (i/of->stripe_width)*(STRIP_SIZE*ONEKB);
		from = fsstart + os%(STRIP_SIZE*ONEKB);
		to = fsstart + min(os%(STRIP_SIZE*ONEKB)+len, STRIP_SIZE*ONEKB);
		plen = to - from;
		char From[10];
		char PLen[10];
		int2str(from, From, 10);
		int2str(plen, PLen, 10);
		strcat(spacket, From);
		strcat(spacket, " ");
		strcat(spacket, PLen);
		strcat(spacket, " END.\n");
		strncat(spacket, buf+os-offset, plen);
		if (send(fs[i%of->stripe_width].fd, spacket, sizeof(spacket), 0) == -1)
			perror("Error in sending!");//WRITE pfs_file1 from to(plen) END.
		//if (send(fs[i%of->stripe_width].fd, buf+os-offset, plen, 0) == -1)
		//	perror("Error in sending!");
		//int size;
		//char* rpacket = malloc(sizeof(plen)); //Buffer for client data
		//if ((size = recv(fs[i%of->stripe_width].fd, rpacket, sizeof(rpacket), 0)) == -1)
			//Wait for response -> WRITE project2_2 0 4 0 0 w END. -> 0 1 2 3
		//	perror("Error in receiving!");
		//rpacket[size] = '\0';
		//printf("Server: %s\n",rpacket);
		//strncpy(buf, rpacket, sizeof(rpacket));
		//free(rpacket);
		save2cache(filedes,os,plen);
		os += plen;
		len -= plen;
	}

	strcpy(spacket,"RELEASE ");
	strcat(spacket, of->filename);
	strcat(spacket, " ");
	int2str(offset/ONEKB,Offset,10);
	int2str(nbyte/ONEKB,Length,10);
	strcat(spacket, Offset);
	strcat(spacket, " ");
	strcat(spacket, Length);
	strcat(spacket," END.");
	if (send(mmFD, spacket, sizeof(spacket), 0) == -1)	//RELEASE pfs_file1 Offset Length END.
		perror("Error in sending!");

	return nbyte;
}

int pfs_close(int filedes)
{//flush
	if(filedes>=fdnum)
		return -1;

	int ofI = 0;
	while((ofI < ofnum)&&(openfiles[ofI]->fd != filedes))
		ofI++;
	if(ofI == ofnum)
		return -1;
	OpenFile* of = openfiles[ofI];

	int cI;
	for(cI=0;cI<CLIENT_CACHE_LINES;cI++)
		if(metacache[cI][0]!=0)
			flusher();

	int i;
	for(i=ofI;i<ofnum-1;i++)
		openfiles[i] = openfiles[i+1];
	ofnum--;
	free(of);
	return 0;
}

int pfs_delete(const char *filename)
{
	char spacket[BUFFER_SIZE]; //Buffer for client data
	strcpy(spacket,"DELETE ");
	strcat(spacket, filename);
	strcat(spacket," END.");
	if (send(mmFD, spacket, sizeof(spacket), 0) == -1)	//DELETE pfs_file1 END.
		perror("Error in sending!");
	char rpacket[BUFFER_SIZE] = ""; //Buffer for client data
	//int size =
	MMR(rpacket, BUFFER_SIZE);//DELETE project2_2 fsoffset sw END.

	//char buf[] = "";
	int fnI = findspace(rpacket, 0, 1); //project2_2
	int fsI = findspace(rpacket, fnI, 1); //0
	int swI = findspace(rpacket, fsI, 1); //4
	int endI = findspace(rpacket, swI, 1); //w
	int os = str2int(rpacket+fsI, swI-fsI-1);
	int sw = str2int(rpacket+swI, endI-swI-1);

	int i;
	for(i=0; i<sw; i++)
	{//Send for each FSs -> DELETE pfs_file1 END.
		if (send(fs[(i+os)%sw].fd, spacket, sizeof(spacket), 0) == -1)
			perror("Error in sending!");
	}

	return 0;
}

int pfs_fstat(int filedes, struct pfs_stat *buf)
{
	if(filedes>=fdnum)
		return -1;

	int ofI = 0;
	while((ofI < ofnum)&&(openfiles[ofI]->fd != filedes))
		ofI++;
	if(ofI == ofnum)
		return -1;
	OpenFile* of = openfiles[ofI];

	char spacket[BUFFER_SIZE]; //Buffer for client data
	strcpy(spacket,"STAT ");
	strcat(spacket, of->filename);
	strcat(spacket," END.");
	if (send(mmFD, spacket, sizeof(spacket), 0) == -1)	//STAT pfs_file1 END.
		perror("Error in sending!");
	//char rpacket[sizeof(struct pfs_stat)] = ""; //Buffer for client data
	//int size =
	MMR(buf, sizeof(struct pfs_stat));

	return 0;
}

int initialize(int argc, char *argv[])
{
	tv.tv_sec = 2;
	tv.tv_usec = 500000;
	FD_ZERO(&list);			// clear list
	FD_ZERO(&list_copy);	// clear temporary sets
	FD_SET(STDIN, &list);	// Add stdin
	fdmax = STDIN; // The biggest file descriptor
	yes = 1;
	ofnum = 0;//number of open files
	fdnum = 0;//number of file descriptors

	int cI;
	for(cI=0;cI<CLIENT_CACHE_LINES;cI++)
	{
		metacache[cI][0] = 0;//free
		metacache[cI][1] = -1;//TAG
		metacache[cI][2] = 0;//LRU
		metacache[cI][3] = -1;//OFi
	}

	srand (time(NULL));//randomize rand function
	int2str(rand() % (65536 - 1025) + 1025, myPORT, 10);//get a random port number for my pfs client
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getownip(myIP);	//inet_ntop(remoteaddr.ss_family, get_address((struct sockaddr*)&remoteaddr), myIP, INET6_ADDRSTRLEN);
	printf("My IP address and Port number are %s %s\n",myIP,myPORT);

	if(argc!=4){
		fprintf(stderr,"Usage: pfs filename mmip mmport\n");
		strcpy(mmIP, MMIP);
		strcpy(mmPORT, MMPORT);
		//exit(1);
	} else {
		strcpy(mmIP, argv[2]);
		strcpy(mmPORT, argv[3]);
	}	//MM's IP&PORT

	mmFD = Connect2Server(mmIP, mmPORT);//Connect to Meta-data Manager (MM)
	FD_SET(mmFD, &list);//add to fd list
	fdmax = mmFD; // The biggest file descriptor
	char rpacket[BUFFER_SIZE] = ""; //Buffer for client data
	int size = MMR(rpacket, BUFFER_SIZE);//IMPORT 4 127.0.0.1 10001 127.0.0.1 10002 127.0.0.1 10003 127.0.0.1 10004 END.
	int fsnumI = findspace(rpacket,0,1);//File server number Index
	char packettype[6];
	strncpy(packettype, rpacket, fsnumI-1);
	packettype[fsnum-1] = '\0';
	if(!strcmp(packettype, "IMPORT"))
	{
		int firstI = findspace(rpacket,fsnumI,1);//First file server Index
		int nextI;// = findspace(packet,ffsI,1); //Next file server Index
		fsnum = str2int(rpacket+fsnumI,firstI-fsnumI-1);//number of file servers
		int i;
		for(i=0;i<fsnum;i++)
		{
			nextI = findspace(rpacket,firstI,1);//Port index
			strncpy(fs[i].ip, rpacket + firstI, nextI - firstI - 1);
			fs[i].ip[nextI - firstI - 1] = '\0';
			firstI = findspace(rpacket,nextI,1);//IP index of next file server
			strncpy(fs[i].port, rpacket + nextI, firstI - nextI - 1);
			fs[i].port[firstI - nextI - 1] = '\0';
		}
		for(i=0;i<fsnum;i++)
		{
			fs[i].fd = Connect2Server(fs[i].ip, fs[i].port);//Connect to FS
			FD_SET(fs[i].fd, &list);//add to fd list
			fdmax = fs[i].fd; // The biggest file descriptor
			size = recv(fs[i].fd, rpacket, sizeof(rpacket), 0);
			if (size == -1)
				perror("Error in receiving!");
			rpacket[size] = '\0';
			printf("Server: %s\n",rpacket);//IMPORT 4 127.0.0.1 10001 127.0.0.1 10002 127.0.0.1 10003 127.0.0.1 10004 END.
		}
	}
	else
		printf("No File Servers Imported!");


	pthread_t harvesterThr, flusherThr;	//Buffer Management Threads
	pthread_create(&harvesterThr, NULL, (void *) harvester, NULL); //??? debug it!
	pthread_create(&flusherThr, NULL, (void *) flusher, NULL);
	//pthread_join(harvesterThr, NULL);//every x-sec??? or when falls under threshold!???
	//pthread_join(flusherThr, NULL);//every 30 seconds???

	return 0;
}

int finalize()
{
	close(mmFD);
	int i;
	for(i=0;i<NUM_FILE_SERVERS;i++)
		close(fs[i].fd);
	FD_ZERO(&list);			// clear list
	FD_ZERO(&list_copy);	// clear temporary sets
	return 0;
}

int Connect2Server(char* ip, char* port)
{
	int server;
	struct addrinfo *temp_ai, *servinfo;
	int aierror = getaddrinfo(ip, port, &hints, &servinfo);
	if (aierror != 0)
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(aierror));
	for(temp_ai = servinfo; temp_ai != NULL; temp_ai = temp_ai->ai_next)
	{
		//create socket
		if ((server = socket(temp_ai->ai_family, temp_ai->ai_socktype, temp_ai->ai_protocol)) == -1){
			perror("Client: socket error!");
			continue;
		}
		//connect to server
		if (connect(server, temp_ai->ai_addr, temp_ai->ai_addrlen) == -1)
		{
			close(server);
			perror("Client: connect error!");
			continue;
		}
		break;
	}
	if (temp_ai == NULL)
	{
		fprintf(stderr, "Failed to connect!\n");
		exit(1);
	}
	char RemoteIP[INET6_ADDRSTRLEN];
	inet_ntop(temp_ai->ai_family, get_address((struct sockaddr *)temp_ai->ai_addr), RemoteIP, sizeof(RemoteIP));
	printf("Connected to Server (%s:%s)\n", RemoteIP, port);
	freeaddrinfo(servinfo); // Successfully connected to server
	return server;
}

int Listen2Clients()
{
	int client;
	struct addrinfo hints, *ai, *temp_ai;
	int aierror = getaddrinfo(NULL, myPORT, &hints, &ai);
	if (aierror != 0)
	{
		fprintf(stderr, "Client: %s\n", gai_strerror(aierror));
		exit(1);
	}
	for(temp_ai = ai; temp_ai != NULL; temp_ai = temp_ai->ai_next)
	{
		client = socket(temp_ai->ai_family, temp_ai->ai_socktype, temp_ai->ai_protocol);
		if (client < 0) continue;
		setsockopt(client, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if (bind(client, temp_ai->ai_addr, temp_ai->ai_addrlen) < 0)
		{
			close(client);
			continue;
		}
		break;
	}
	if (temp_ai == NULL)
		fprintf(stderr, "Client: failed to bind\n");
	freeaddrinfo(ai); // all done with this
	if (listen(client, NUM_FILE_SERVERS) == -1)
		perror("listen");
	return client;
}

void getownip(char* host)
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	if (getifaddrs(&ifaddr) == -1) {
	   perror("getifaddrs");
	   exit(EXIT_FAILURE);
	}
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
	   if (ifa->ifa_addr == NULL)
		   continue;
	   family = ifa->ifa_addr->sa_family;
	   if (family == AF_INET || family == AF_INET6) {
		   s = getnameinfo(ifa->ifa_addr,
				   (family == AF_INET) ? sizeof(struct sockaddr_in) :
										 sizeof(struct sockaddr_in6),
				   host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		   if (s != 0) {
			   printf("getnameinfo() failed: %s\n", gai_strerror(s));
			   exit(EXIT_FAILURE);
		   }
		   if( ((int)(host[0])<(int)'9')
				   && ((int)(host[0])>(int)'0')
				   && ( (host[0]!='1') || (host[1]!='2') || (host[2]!='7') ) )
			   break;
	   }
   }
   freeifaddrs(ifaddr);
}

unsigned str2int(char* str, int len)
{
	int l = len - 1;
	unsigned i = 0;
	unsigned p10 = 1;
	while(l>=0)
	{
		i += (((char)(str[l]))-(char)'0')*p10;
		l--;
		p10 *= 10;
	}
	return i;
}

int int2str(int i, char* str, int base)
{// i=12682768 as int -> 12682768 in str
	if(i)
	{
		int max = 1;
		int p = 0;
		while(i>=max)
		{
			p++;
			max = max * base;
		}
		max = max / base;
		int t = i;
		int pc = 0;
		while(pc<p)
		{
			str[pc] = (char)((t / max) + '0');
			t = t % max;
			max = max / base;
			pc++;
		}
		str[p] = '\0';
		return p;
	}
	else
	{
		str[0] = '0';
		str[1] = '\0';
		return 1;
	}
}

int findspace(char* str, int offset, int num)
{//str=string, offset=from what point in string, num=number of spaces
	int i = offset;
	while ((num > 0) && (str[i] != '\0'))
	{
		if(str[i] == separator)
			num--;
		i++;
	}
	return i;//1 192.168.1.3 10001 file1 1234
}//return the index (0-...) of the char right after num-th space

void *get_address(struct sockaddr *socketaddress)
{
	if (socketaddress->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)socketaddress)->sin_addr);
	}
	return &(((struct sockaddr_in6*)socketaddress)->sin6_addr);
}

unsigned min(unsigned i, unsigned j)
{
	if(i<j)
		return i;
	else
		return j;
}

int save2cache(unsigned fdi, unsigned os, unsigned len)
{
	int i;
	for(i=os/ONEKB;i<(os+len)/ONEKB;i++)
	{
		if(metacache[i%(CLIENT_CACHE_LINES)][0] == 0)//free
		{
			metacache[i%(CLIENT_CACHE_LINES)][0] = 1;//used
			metacache[i%(CLIENT_CACHE_LINES)][1] = i;//TAG
			metacache[i%(CLIENT_CACHE_LINES)][2] = 1;//RU
			metacache[i%(CLIENT_CACHE_LINES)][3] = fdi;//ofi
		}
		else if(metacache[i%(CLIENT_CACHE_LINES)][3] == fdi)
		{//used or changed
			metacache[i%(CLIENT_CACHE_LINES)][0] = 2;//changed
			metacache[i%(CLIENT_CACHE_LINES)][1] = i;//TAG
			metacache[i%(CLIENT_CACHE_LINES)][2] = 1;//RU
		}
		else
		{//different fd want to change prior
			while(metacache[i%(CLIENT_CACHE_LINES)][0] == 0);//Flusher must update
			metacache[i%(CLIENT_CACHE_LINES)][0] = 1;//used
			metacache[i%(CLIENT_CACHE_LINES)][1] = i;//TAG
			metacache[i%(CLIENT_CACHE_LINES)][2] = 1;//RU
			metacache[i%(CLIENT_CACHE_LINES)][3] = fdi;//ofi
		}
	}
	return 0;
}

int readcache(unsigned fdi, unsigned os, unsigned len)
{
	int i;
	for(i=os/ONEKB;i<(os+len)/ONEKB;i++)
	{
		if(metacache[i%(CLIENT_CACHE_LINES)][0] == 0)//free -> cache miss
			return 0;//miss
		else if(metacache[i%(CLIENT_CACHE_LINES)][1] != i) //TAG has been changed
			return 0;//miss
		else if(metacache[i%(CLIENT_CACHE_LINES)][3] != fdi) //FD has been changed
			return 0;//miss
		metacache[i%(CLIENT_CACHE_LINES)][2]++;
	}
	return 1;//hit
}

int MMR(char* buf, int bufsize)
{
	int size;
	int rv = 0;
	do
	{
		size = recv(mmFD, buf, bufsize, 0);
		if(size == -1)
			perror("Error in receiving!");
		buf[size] = '\0';
		printf("MM: %s\n",buf);
		if(!strncmp(buf, "RELEASE", 7))
		{
			rv=1;
			release(buf,size);
		}
		else
			rv=0;
	}while(rv);
	return size;
}

int release(char* buf, int bufsize)
{//RELEASE pfs_file1 Offset Length END.
	char FileName[20];
	int fnI = findspace(buf, 0, 1); //pfs_file1
	int osI = findspace(buf, fnI, 1); //Offset
	int lI = findspace(buf, osI, 1); //Length
	int endI = findspace(buf, lI, 1); //END.
	strncpy(FileName, buf+fnI, osI-fnI-1);
	FileName[osI-fnI-1] = '\0';
	int ofi = 0;
	while( (ofi<ofnum) && (strncmp(openfiles[ofi]->filename, FileName, osI-fnI-1)) )
		ofi++;
	if(ofi==ofnum)
		return -1;
	//OpenFile* of = openfiles[ofi];
	int i;
	int os = str2int(buf+osI, lI-osI-1);
	int len = str2int(buf+endI, endI-lI-1);
	for(i=os/ONEKB;i<(os+len)/ONEKB;i++)
		if((metacache[i%(CLIENT_CACHE_LINES)][0] > 0) && (metacache[i%(CLIENT_CACHE_LINES)][3] == ofi))
			metacache[i%(CLIENT_CACHE_LINES)][0] = 0;//free or can be 3!
	return 0;
}
