/*
 * PFS_server.c
 *
 *  Created on: Nov 4, 2012
 *
 */

#include "PFS_server.h"

void* readfile(void* TD)
{//READ pfs_file1 w from to(plen) END.
	struct TData* td = (struct TData*)TD;
	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	int mI = findspace(td->buf, fnI, 1);
	int fromI = findspace(td->buf, mI, 1);
	int plenI = findspace(td->buf, fromI, 1);
	int endI = findspace(td->buf, plenI, 1);
	strncpy(FileName, td->buf+fnI, mI-fnI-1);
	FileName[fromI-fnI-1] = '\0';
	int from = str2int(td->buf+fromI, plenI-fromI-1);
	int plen = str2int(td->buf+plenI, endI-plenI-1);

	char spacket[plen]; //Buffer for client data
	FILE* pF;
	pF = fopen (FileName,"r");
	if(pF!=NULL)
	{
		fseek(pF, from, SEEK_SET);
		int result = fread (spacket,1,plen,pF);
		if (result != plen)
			fputs ("Reading error",stderr);
		spacket[plen] = '\0';
		fclose(pF);
	}
	int size;
	if ( (size = send(td->tid, spacket, plen, 0)) == -1)
		perror("Error in sending!");
	free(td);
	pthread_exit(NULL);
	return NULL;
}

void* writefile(void* TD)
{//WRITE pfs_file1 from to(plen) END.\n
	struct TData* td = (struct TData*)TD;
	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	int fromI = findspace(td->buf, fnI, 1);
	int plenI = findspace(td->buf, fromI, 1);
	int endI = findspace(td->buf, plenI, 1);
	strncpy(FileName, td->buf+fnI, fromI-fnI-1);
	FileName[fromI-fnI-1] = '\0';
	int from = str2int(td->buf+fromI, plenI-fromI-1);
	int plen = str2int(td->buf+plenI, endI-plenI-1);
	td->buf[endI+5+plen] = '\0';
	//int size;
	//char* rpacket = malloc(plen); //Buffer for client data
	//if ((size = recv(td->tid, rpacket, sizeof(rpacket), 0)) == -1)
	//	perror("Error in receiving!");
	//rpacket[size] = '\0';
	//printf("Client: %s\n",rpacket);
	FILE* pF;
	pF = fopen (FileName,"a+");
	if(pF!=NULL)
	{
		fseek(pF, from, SEEK_SET);
		fputs(td->buf+endI+5,pF);//endI+5 is the start of filewrites!
		fclose(pF);
	}
	free(td);
	pthread_exit(NULL);
	return NULL;
}

void* deletefile(void* TD)
{//DELETE pfs_file1 END.
	struct TData* td = (struct TData*)TD;
	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	int endI = findspace(td->buf, fnI, 1);
	strncpy(FileName, td->buf+fnI, endI-fnI-1);
	FileName[endI-fnI-1] = '\0';
	if( remove(FileName) != 0 )
		perror( "Error deleting file" );
	else
		printf("%s successfully deleted.",FileName);
	free(td);
	pthread_exit(NULL);
	return NULL;
}

void* openfile(void* TD)
{//OPEN pfs_file1 w END.
	struct TData* td = (struct TData*)TD;
	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	int mI = findspace(td->buf, fnI, 1);
	strncpy(FileName, td->buf+fnI, mI-fnI-1);
	FileName[mI-fnI-1] = '\0';
	FILE* pF;
	pF = fopen (FileName,"a+");
	if(pF!=NULL)
	{
		//fputs (" ",pF);
		fclose (pF);
	}
	free(td);
	pthread_exit(NULL);
	return NULL;
}

void* createfile(void* TD)
{//CREATE project2 3 END.
	struct TData* td = (struct TData*)TD;
	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	int snI = findspace(td->buf, fnI, 1);
	strncpy(FileName, td->buf+fnI, snI-fnI-1);
	FileName[snI-fnI-1] = '\0';
	FILE* pF;
	pF = fopen (FileName,"a+");
	if(pF!=NULL)
		fclose (pF);
	free(td);
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char *argv[])
{
	setup(argc,argv);
	while(1)
	{
		list_copy = list;
		if (select(fdmax+1, &list_copy, NULL, NULL, &tv) == -1)
		{
			perror("select");
			exit(4);
		}
		int fdc;
		for(fdc = 0; fdc <= fdmax; fdc++)
		{
			if (FD_ISSET(fdc, &list_copy))
			{ // we got one!!
				if (fdc == myTFD)
				{// handle new connections
					addrlen = sizeof(remoteaddr);
					newfd = accept(myTFD,(struct sockaddr *)&remoteaddr,&addrlen);
					if ((newfd == -1)||(newfd>=CONNECTIONS_LIMIT))
						perror("Error in accept!");
					else
					{
						FD_SET(newfd, &list); // add to list set
						if (newfd > fdmax) // keep track of the max
							fdmax = newfd;
						printf("Client: new connection from %s on socket %d\n",
								inet_ntop(remoteaddr.ss_family,
										get_address((struct sockaddr*)&remoteaddr),
										RemoteIP, INET6_ADDRSTRLEN),
										newfd);
						//if (send(newfd, "Hello new client!\n", 18, 0) == -1)//send is not parallel???
						//	perror("Error in sending!");
						if (!fork()) { // this is the child process
							close(myTFD); // child doesn't need the listener
							if (send(newfd, "Hello new client!", 18, 0) == -1)
								perror("Error in sending!");
							close(newfd);
							exit(0);
						}
					}
				}
				else
				{
					char buffer[BUFFER_SIZE];//packet buffer
					int NumBytes;//number of bytes in buffer
					if ((NumBytes = recv(fdc, buffer, sizeof buffer, 0)) <= 0)// handle data from a client
					{	// got error or connection closed by client
						if (NumBytes == 0) // connection closed
							printf("Client on socket#%d hung up.\n", fdc);
						else
							perror("Error in receiving!");
						close(fdc); // close connection number fdc
						FD_CLR(fdc, &list); // remove from list set
					}
					else
					{	// we got some data from a client
						printf("Client on socket#%d: %s\n",fdc,buffer);
						if(!strncmp(buffer, "CREATE", 6))
						{
							//createfile(fdc, buffer);
							pthread_t thr;
							struct TData *td = malloc(sizeof(struct TData));
							td->tid = fdc;
							//td->fd = fdc;
							td->buf = buffer;
							pthread_create(&thr, NULL, createfile, (void *)td);
						}
						else if(!strncmp(buffer, "OPEN", 4))
						{
							//openfile(fdc, buffer);
							pthread_t thr;
							struct TData *td = malloc(sizeof(struct TData));
							td->tid = fdc;
							//td->fd = fdc;
							td->buf = buffer;
							pthread_create(&thr, NULL, openfile, (void *)td);
						}
						else if(!strncmp(buffer, "READ", 4))
						{
							//readfile(fdc, buffer);
							pthread_t thr;
							struct TData *td = malloc(sizeof(struct TData));
							td->tid = fdc;
							//td->fd = fdc;
							td->buf = buffer;
							pthread_create(&thr, NULL, readfile, (void *)td);
						}
						else if(!strncmp(buffer, "WRITE", 5))
						{
							pthread_t thr;
							struct TData *td = malloc(sizeof(struct TData));
							td->tid = fdc;
							//td->fd = fdc;
							td->buf = buffer;
							pthread_create(&thr, NULL, writefile, (void *)td);
							//writefile(fdc, buffer);
						}
						else if(!strncmp(buffer, "DELETE", 6))
						{
							pthread_t thr;
							struct TData *td = malloc(sizeof(struct TData));
							td->tid = fdc;
							//td->fd = fdc;
							td->buf = buffer;
							pthread_create(&thr, NULL, deletefile, (void *)td);
							//deletefile(fdc, buffer);
						}
					}
				} // END handle data from client
			} // END got new incoming connection
		} // END looping through file descriptors
	} // while(1)
	return 0;
}

void *get_address(struct sockaddr *socketaddress)
{
	if (socketaddress->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)socketaddress)->sin_addr);
	}
	return &(((struct sockaddr_in6*)socketaddress)->sin6_addr);
}

int setup(int argc, char *argv[])
{
	tv.tv_sec = 2;
	tv.tv_usec = 500000;
	FD_ZERO(&list);			// clear list
	FD_ZERO(&list_copy);	// clear temporary sets
	FD_SET(STDIN, &list);	// Add stdin
	fdmax = STDIN; // The biggest file descriptor
	yes = 1;
	dbi = 0; //database index = 0???
	srand (time(NULL));//randomize rand function
	int2str(rand() % (65536 - 1025) + 1025, myPORT, 10);//get a random port number for my pfs client
	getownip(myIP);	//inet_ntop(remoteaddr.ss_family, get_address((struct sockaddr*)&remoteaddr), myIP, INET6_ADDRSTRLEN);
	printf("My IP address and Port number are %s %s\n",myIP,myPORT);//Getting my ip and port

	if(argc!=3){
		fprintf(stderr,"Usage: PFS_server mmip mmport\n");
		strcpy(mmIP, MMIP);
		strcpy(mmPORT, MMPORT);
		//exit(1);
	} else {
		strcpy(mmIP, argv[1]);
		strcpy(mmPORT, argv[2]);
	}//Getting MM's IP&PORT

	struct addrinfo *addinfo;//Create UDP Socket
	memset(&Uhints, 0, sizeof Uhints);
	Uhints.ai_family = AF_UNSPEC;
	Uhints.ai_socktype = SOCK_DGRAM;
	int aierr = getaddrinfo(mmIP, mmPORT, &Uhints, &addinfo);
	if (aierr != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(aierr));
		return 1;
	}
	for(mm_ai = addinfo; mm_ai != NULL; mm_ai = mm_ai->ai_next) {	// loop through all the results and make a socket
		if ((myUFD = socket(mm_ai->ai_family, mm_ai->ai_socktype, mm_ai->ai_protocol)) == -1) {
			perror("Server: socket failed!");
			continue;
		}
		break;
	}
	if (mm_ai == NULL) {
		fprintf(stderr, "Server: failed to bind socket!\n");
		return 2;
	}
	char packet[BUFFER_SIZE];
	strcpy(packet, "EXPORT ");
	strcat(packet, myIP);
	strcat(packet, " ");
	strcat(packet, myPORT);
	strcat(packet, " END.");
	int numbytes;
	if ((numbytes = sendto(myUFD, packet, strlen(packet), 0, mm_ai->ai_addr, mm_ai->ai_addrlen)) == -1) {
		perror("Server: sendto");
		return 3;
	}
	printf("Server: sent %d bytes to %s %s\n", numbytes, mmIP, mmPORT);
	//freeaddrinfo(addinfo);

	memset(&Thints, 0, sizeof(Thints));//get a socket and bind it
	Thints.ai_family = AF_UNSPEC;
	Thints.ai_socktype = SOCK_STREAM;
	Thints.ai_flags = AI_PASSIVE;
	struct addrinfo *addrinfo, *temp_ai;//Create TCP Socket
	aierr = getaddrinfo(NULL, myPORT, &Thints, &addrinfo);
	if (aierr != 0)
	{
		fprintf(stderr, "Server: %s\n", gai_strerror(aierr));
		return 1;
	}
	for(temp_ai = addrinfo; temp_ai != NULL; temp_ai = temp_ai->ai_next)
	{
		myTFD = socket(temp_ai->ai_family, temp_ai->ai_socktype, temp_ai->ai_protocol);
		if (myTFD < 0) continue;
		setsockopt(myTFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));// manipulate options for the socket referred to by the file descriptor
		if (bind(myTFD, temp_ai->ai_addr, temp_ai->ai_addrlen) < 0)
		{
			close(myTFD);
			continue;
		}
		break;
	}
	if (temp_ai == NULL)
	{// if we got here, it means we did get bound
		fprintf(stderr, "Server: failed to bind!\n");
		return 2;
	}
	freeaddrinfo(addrinfo); // all done with this
	if (listen(myTFD, CONNECTIONS_LIMIT) == -1)
	{// listen
		perror("Server: fail to listen!\n");
		return 3;
	}
	FD_SET(myTFD, &list); // add the myTFD to the list set
	fdmax = myTFD; // keep track of the biggest file descriptor
	return 0;
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

