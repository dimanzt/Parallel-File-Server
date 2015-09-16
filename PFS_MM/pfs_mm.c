/*
 * pfs_mm.c
 *
 *  Created on: Nov 22, 2012
 *
 */

#include "pfs_mm.h"


void* deletefile(void* TD)
{//DELETE pfs_file1 END.
	struct TData* td = (struct TData*)TD;
	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	int endI = findspace(td->buf, fnI, 1);
	strncpy(FileName, td->buf+fnI, endI-fnI-1);//catch filename
	FileName[endI-fnI-1] = '\0';

	int findI = 0;
	while((findI<filenum)&&(strncmp(fmdarray[findI]->filename,FileName,sizeof(FileName))))
		findI++;
	if(findI<filenum)
	{
		FileMetaData* fmd = fmdarray[findI];
		char buffer[BUFFER_SIZE];//packet buffer
		strcpy(buffer, "DELETE ");
		strcat(buffer, fmd->filename);
		strcat(buffer, " ");
		char offset[5];
		int2str(fmd->rcp.fsoffset, offset, 10);
		strcat(buffer, offset);
		strcat(buffer, " ");
		char SW[5];
		int2str(fmd->rcp.strip_width, SW, 10);
		strcat(buffer, SW);
		strcat(buffer, " END.");
		if (send(td->tid, buffer, sizeof(buffer), 0) == -1)
			perror("Error in sending!");//DELETE project2_2 fsoffset sw END.
		int fI = findI;
		for(fI=findI;fI<filenum-1;fI++)
			fmdarray[fI] = fmdarray[fI+1];
		free(fmd);
	}
	free(td);
	pthread_exit(NULL);
	return NULL;
}

void* getstat(void* TD)
{//STAT pfs_file1 END.
	struct TData* td = (struct TData*)TD;
	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	int endI = findspace(td->buf, fnI, 1);
	strncpy(FileName, td->buf+fnI, endI-fnI-1);//catch filename
	FileName[endI-fnI-1] = '\0';

	int findI = 0;
	while((findI<filenum)&&(strncmp(fmdarray[findI]->filename,FileName,sizeof(FileName))))
		findI++;
	if(findI<filenum)
	{
		FileMetaData* fmd = fmdarray[findI];
		//char buffer[sizeof(struct pfs_stat)];//packet buffer
		struct pfs_stat ps;
		ps.pst_ctime = fmd->creation;
		ps.pst_mtime = fmd->modification;
		ps.pst_size = fmd->size;
		//strcpy(buffer, ps);
		if (send(td->tid, &ps, sizeof(struct pfs_stat), 0) == -1)
			perror("Error in sending!");
	}
	free(td);
	pthread_exit(NULL);
	return NULL;
}

void* release(void* TD)
{//RELEASE pfs_file1 Offset Length END.
	struct TData* td = (struct TData*)TD;
	int fdc = 1;
	while( FD_ISSET(fdc, &list) && ((fdc != td->tid) && (fdc != myTFD)) )
		if (send(td->tid, td->buf, sizeof(td->buf), 0) == -1)
			perror("Error in sending!");
	free(td);
	pthread_exit(NULL);
	return NULL;
}

void* readfile(void* TD)
{//READ pfs_file1 offset length END.
	struct TData* td = (struct TData*)TD;
/*	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	//int mI = findspace(td->buf, fnI, 1);
	int osI = findspace(td->buf, fnI, 1);
	int lI = findspace(td->buf, osI, 1);
	int endI = findspace(td->buf, lI, 1);
	strncpy(FileName, td->buf+fnI, osI-fnI-1);//catch filename
	FileName[osI-fnI-1] = '\0';
	//char Mode[5];
	//strncpy(Mode, td->buf+mI, osI-mI-1);
	//Mode[endI-mI-1] = '\0';
	int offset = str2int(td->buf+osI,lI-osI-1);
	int length = str2int(td->buf+lI,endI-lI-1);
	int findI = 0;
	while((findI<filenum)&&(strncmp(fmdarray[findI]->filename,FileName,sizeof(FileName))))
		findI++;
	if(findI<filenum)
	{
		FileMetaData* fmd = fmdarray[findI];
		//if(fmd->size>0)	assign new size;
		//int blockI;
		//for(blockI=offset/ONEKB;blockI<(offset+length)/ONEKB;blockI++)
		//	if(fmd->rcp.fdlist[blockI] != 0)
		//		fmd->rcp.fdlist[blockI] = td->tid;
		//	else
		//	{
		//		while(fmd->rcp.fdlist[blockI]!=0);
		//		fmd->rcp.fdlist[blockI] = td->tid;
		//	}
		char buffer[BUFFER_SIZE];//packet buffer
		strcpy(buffer, "WRITE ");
		strcat(buffer, fmd->filename);
		strcat(buffer, " ");
		char offset[5];
		int2str(fmd->rcp.fsoffset, offset, 10);
		strcat(buffer, offset);
		strcat(buffer, " ");
		char SW[5];
		int2str(fmd->rcp.strip_width, SW, 10);
		strcat(buffer, SW);
		strcat(buffer, " ");
		strncat(buffer, td->buf+osI, lI-osI-1);
		strcat(buffer, " ");
		strncat(buffer, td->buf+lI, endI-lI-1);
		strcat(buffer, " END.");//WRITE filename fsoff sw offset length END.
		if (send(td->tid, buffer, sizeof(buffer), 0) == -1)
			perror("Error in sending!");
	}
*/
	free(td);
	pthread_exit(NULL);
	return NULL;
}

void* writefile(void* TD)
{//WRITE filename w offset length END.
	struct TData* td = (struct TData*)TD;
	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	int mI = findspace(td->buf, fnI, 1);
	int osI = findspace(td->buf, mI, 1);
	int lI = findspace(td->buf, osI, 1);
	int endI = findspace(td->buf, lI, 1);
	strncpy(FileName, td->buf+fnI, mI-fnI-1);//catch filename
	FileName[mI-fnI-1] = '\0';
	char Mode[5];
	strncpy(Mode, td->buf+mI, osI-mI-1);
	Mode[endI-mI-1] = '\0';
	int offset = str2int(td->buf+osI,lI-osI-1);
	int length = str2int(td->buf+lI,endI-lI-1);
	int findI = 0;
	while((findI<filenum)&&(strncmp(fmdarray[findI]->filename,FileName,sizeof(FileName))))
		findI++;
	if(findI<filenum)
	{
		FileMetaData* fmd = fmdarray[findI];
		//if(fmd->size>0)	assign new size;
		int blockI = offset/ONEKB;
		while(blockI<(offset+length)/ONEKB)
		{
			if(fmd->rcp.fdlist[blockI] != 0)
				blockI = offset/ONEKB;
				//fmd->rcp.fdlist[blockI] = td->tid;
			//else
			//{//??? if two concurrent writes have intersections!
			//	fmd->rcp.fdlist[blockI] = td->tid;
			//}
			blockI++;
		}
		//int rc =
		pthread_mutex_lock(&mutex);
		blockI = offset/ONEKB;
		do{
			fmd->rcp.fdlist[blockI] = td->tid;
			blockI++;
		}while(blockI<(offset+length)/ONEKB);
		pthread_mutex_unlock(&mutex);

		char buffer[BUFFER_SIZE];//packet buffer
		strcpy(buffer, "WRITE ");
		strcat(buffer, fmd->filename);
		strcat(buffer, " ");
		char fsoffset[5];
		int2str(fmd->rcp.fsoffset, fsoffset, 10);
		strcat(buffer, fsoffset);
		strcat(buffer, " ");
		char SW[5];
		int2str(fmd->rcp.strip_width, SW, 10);
		strcat(buffer, SW);
		strcat(buffer, " ");
		strncat(buffer, td->buf+osI, lI-osI-1);
		strcat(buffer, " ");
		strncat(buffer, td->buf+lI, endI-lI-1);
		strcat(buffer, " END.");//WRITE filename fsoff sw offset length END.
		if (send(td->tid, buffer, sizeof(buffer), 0) == -1)
			perror("Error in sending!");
		//fmd->modification = time(NULL);
		time(&fmd->modification);
		fmd->size = max(fmd->size, (offset+length)*ONEKB);
	}
	free(td);
	pthread_exit(NULL);
	return 0;
}

void* openfile(void* TD)
{//OPEN pfs_file1 w END.
	struct TData* td = (struct TData*)TD;
	char FileName[20];
	int fnI = findspace(td->buf, 0, 1);
	int mI = findspace(td->buf, fnI, 1);
	int endI = findspace(td->buf, mI, 1);
	strncpy(FileName, td->buf+fnI, mI-fnI-1);//catch filename
	FileName[mI-fnI-1] = '\0';
	char Mode[5];
	strncpy(Mode, td->buf+mI, endI-mI-1);
	Mode[endI-mI-1] = '\0';
	//unsigned sw = str2int(SW,endI-swI-1);
	//checkrepeatedfilename()???

	int findI = 0;
	while((findI<filenum)&&(strncmp(fmdarray[findI]->filename,FileName,sizeof(FileName))))
		findI++;
	if(findI<filenum)
	{
		FileMetaData* fmd = fmdarray[findI];
		char buffer[BUFFER_SIZE];//packet buffer
		strcpy(buffer, "OPEN ");
		strcat(buffer, fmd->filename);
		strcat(buffer, " ");
		char offset[5];
		int2str(fmd->rcp.fsoffset, offset, 10);
		strcat(buffer, offset);
		strcat(buffer, " ");
		char SW[5];
		int2str(fmd->rcp.strip_width, SW, 10);
		strcat(buffer, SW);
		if(!strcmp(Mode,"w"))
		{
			char start[10];
			char length[10];
			int2str(fmd->rcp.start[fmd->rcp.fdnum], start, 10);
			int2str(fmd->rcp.length[fmd->rcp.fdnum], length, 10);
			strcat(buffer, " ");
			strcat(buffer, start);
			strcat(buffer, " ");
			strcat(buffer, length);
			fmd->rcp.fdnum++;
		}
		strcat(buffer, " ");
		strcat(buffer, Mode);
		strcat(buffer, " END.");
		if (send(td->tid, buffer, sizeof(buffer), 0) == -1)
			perror("Error in sending!");
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
	int swI = findspace(td->buf, fnI, 1);
	int endI = findspace(td->buf, swI, 1);
	strncpy(FileName, td->buf+fnI, swI-fnI-1);//catch filename
	FileName[swI-fnI-1] = '\0';
	char SW[5];
	strncpy(SW, td->buf+swI, endI-swI-1);
	SW[endI-swI-1] = '\0';
	unsigned sw = str2int(SW,endI-swI-1);
	//checkrepeatedfilename()???

	FileMetaData* fmd = malloc(sizeof(FileMetaData));
	strcpy(fmd->filename, FileName);
	//fmd->creation=;
	//fmd->modification=;
	fmd->size = 0;
	//fmd->creation = time(NULL);
	//fmd->modification = time(NULL);
	time(&fmd->creation);
	time(&fmd->modification);
	fmd->rcp.fsoffset = StripeWidthStart++;
	fmd->rcp.strip_width = min(sw,NUM_FILE_SERVERS);
	fmd->rcp.fdarray = NULL;
	int lI;
	for(lI=0;lI<MAX_SIZE;lI++)
		fmd->rcp.fdlist[lI] = 0;
	fmd->rcp.fd[fmd->rcp.fdnum] = td->tid;
	fmd->rcp.start[fmd->rcp.fdnum] = 0;
	fmd->rcp.length[fmd->rcp.fdnum] = 0;//???

	char buffer[BUFFER_SIZE];//packet buffer
	strcpy(buffer, "CREATE ");
	strcat(buffer, fmd->filename);
	strcat(buffer, " ");
	char offset[5];
	int2str(fmd->rcp.fsoffset, offset, 10);
	strcat(buffer, offset);
	strcat(buffer, " ");
	strcat(buffer, SW);
	//char start[10];
	//char length[10];
	//int2str(fmd->rcp.start[fmd->rcp.fdnum], start, 10);
	//int2str(fmd->rcp.length[fmd->rcp.fdnum], length, 10);
	//strcat(buffer, " ");
	//strcat(buffer, start);
	//strcat(buffer, " ");
	//strcat(buffer, length);
	strcat(buffer, " END.");
	if (send(td->tid, buffer, sizeof(buffer), 0) == -1)
		perror("Error in sending!");
	//fmd->rcp.fdnum++;
	fmdarray[filenum] = fmd;
	filenum++;
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
			perror("select returns error!");
			exit(0);
		}
		int fdc;
		for(fdc = 0; fdc <= fdmax; fdc++)
		{
			if (FD_ISSET(fdc, &list_copy))
			{ // we got one!!
				//printf("I'm here! fdc = %d\n",fdc);
				if (fdc == myTFD)
				{// handle new connections
					addrlen = sizeof(remoteaddr);
					int newfd;
					newfd = accept(myTFD,(struct sockaddr *)&remoteaddr,&addrlen);
					if (newfd == -1)
						perror("accept");
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
						char buffer[BUFFER_SIZE];//packet buffer
						strcpy(buffer, "IMPORT ");
						char FSNum[10];
						int2str(NUM_FILE_SERVERS,FSNum,10);
						strcat(buffer, FSNum);
						int i;
						for(i=0;i<NUM_FILE_SERVERS;i++){//Send list of File servers
							strcat(buffer, " ");
							strcat(buffer, fs[i].ip);
							strcat(buffer, " ");
							strcat(buffer, fs[i].port);
						}
						strcat(buffer, " END.");
						//if (!fork())
						//{ // this is the child process
						//	close(myTFD); // child doesn't need the listener
						if (send(newfd, buffer, sizeof(buffer), 0) == -1)
							perror("Error in sending!");
						//	close(newfd);
						//	exit(0);
						//}
					}
				}
				//else if (fdc == myUFD)
				//{	//newfd only from myTFD!!!
				//}
				else
				{
					char buffer[BUFFER_SIZE];//packet buffer
					int NumBytes;//number of bytes in buffer
					if ((NumBytes = recv(fdc, buffer, sizeof buffer, 0)) <= 0)
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
						else if(!strncmp(buffer, "STAT", 4))
						{
							pthread_t thr;
							struct TData *td = malloc(sizeof(struct TData));
							td->tid = fdc;
							//td->fd = fdc;
							td->buf = buffer;
							pthread_create(&thr, NULL, getstat, (void *)td);
						}
						else if(!strncmp(buffer, "RELEASE", 7))
						{
							pthread_t thr;
							struct TData *td = malloc(sizeof(struct TData));
							td->tid = fdc;
							//td->fd = fdc;
							td->buf = buffer;
							pthread_create(&thr, NULL, release, (void *)td);
						}
					}
				} // END handle data from client
			} // END got new incoming connection
		} // END looping through file descriptors
	} // while(1)
	return 0;
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
	//dbi = 0; //database index = 0???
	filenum = 0;//number of files
	StripeWidthStart = 0; //0 1 2 , 3 0 , 1 2 3 0 , ...
	getownip(myIP);	//inet_ntop(remoteaddr.ss_family, get_address((struct sockaddr*)&remoteaddr), myIP, INET6_ADDRSTRLEN);
	strcpy(myPORT, MMPORT);
	myPORT[5] = '\0';
	printf("MM: my IP(port) = %s(%s).\n",myIP,myPORT);//Getting my ip and port

	struct addrinfo *addinfo;//Create UDP socket
	memset(&Uhints, 0, sizeof Uhints);
	Uhints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	Uhints.ai_socktype = SOCK_DGRAM;
	Uhints.ai_flags = AI_PASSIVE; // use my IP
	int aierr = getaddrinfo(NULL, MMPORT, &Uhints, &addinfo);
	if(aierr != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(aierr));
		return 1;
	}
	for(mm_ai = addinfo; mm_ai != NULL; mm_ai = mm_ai->ai_next) {
		if ((myUFD = socket(mm_ai->ai_family, mm_ai->ai_socktype, mm_ai->ai_protocol)) == -1) {
			perror("MM: socket failed!");
			continue;
		}
		if (bind(myUFD, mm_ai->ai_addr, mm_ai->ai_addrlen) == -1) {
			close(myUFD);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (mm_ai == NULL) {
		fprintf(stderr, "MM: failed to bind socket\n");
		return 2;
	}
	freeaddrinfo(addinfo);
	printf("MM: waiting for file servers to export...\n");

	unsigned addr_len = sizeof(remoteaddr);
	char buf[BUFFER_SIZE];
	int numbytes;
	int ipI;//ip index
	int portI;//port index
	int i;
	for(i=0;i<NUM_FILE_SERVERS;i++){
		if ((numbytes = recvfrom(myUFD, buf, BUFFER_SIZE-1 , 0,
				(struct sockaddr *) (&remoteaddr), &addr_len)) == -1){
			perror("recvfrom");
			return 3;
		}
		buf[numbytes] = '\0';
		ipI = findspace(buf,0,1);
		portI = findspace(buf,ipI,1);
		strncpy(fs[i].ip, buf+ipI, portI-ipI-1);
		fs[i].ip[portI-ipI-1] = '\0';
		strncpy(fs[i].port, buf+portI, findspace(buf,portI,1)-portI-1);
		fs[i].port[findspace(buf,portI,1)-portI-1] = '\0';
		printf("FS[%d]: %s\n", i, buf);//inet_ntop(remoteaddr.ss_family, get_address((struct sockaddr*)&remoteaddr), RemoteIP, sizeof RemoteIP), buf);
	}
	close(myUFD);//???

	struct addrinfo *addrinfo, *temp_ai;//Create TCP socket
	memset(&Thints, 0, sizeof(Thints));
	Thints.ai_family = AF_UNSPEC;
	Thints.ai_socktype = SOCK_STREAM;
	Thints.ai_flags = AI_PASSIVE;
	aierr = getaddrinfo(NULL, MMPORT, &Thints, &addrinfo);
	if (aierr != 0)
	{
		fprintf(stderr, "Client: %s\n", gai_strerror(aierr));
		return 1;
	}
	for(temp_ai = addrinfo; temp_ai != NULL; temp_ai = temp_ai->ai_next)
	{
		myTFD = socket(temp_ai->ai_family, temp_ai->ai_socktype, temp_ai->ai_protocol);
		if (myTFD == -1) continue;
		setsockopt(myTFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));// manipulate options for the socket referred to by the file descriptor
		if (bind(myTFD, temp_ai->ai_addr, temp_ai->ai_addrlen) < 0){
			close(myTFD);
			continue;
		}
		break;
	}
	if (temp_ai == NULL){
		fprintf(stderr, "Client: failed to bind\n");
		return 2;
	}
	freeaddrinfo(addrinfo);
	if (listen(myTFD, CONNECTIONS_LIMIT) == -1){
		perror("listen");
		return 3;
	}
	FD_SET(myTFD, &list);
	fdmax = myTFD;

	SigAct.sa_handler = sigchld_handler;
	sigemptyset(&SigAct.sa_mask);
	SigAct.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &SigAct, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	//pthread_t writefileThr, readfileThr;	//Buffer Management Threads
	//pthread_create(&writefileThr, NULL, (void *) writefile, NULL);
	//pthread_create(&readfileThr, NULL, (void *) readfile, NULL);
	//pthread_join(writefileThr, NULL);//every x-sec??? or when falls under threshold!???
	//pthread_join(readfileThr, NULL);//every 30 seconds???

	return 0;
}

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int findspace(char* str, int offset, int num)
{
	int i = offset;
	while(num > 0)
	{
		if(str[i] == ' ')
			num--;
		i++;
	}
	return i;//1 192.168.1.3 10001 file1 1234
}

int int2str(int i, char* str, int base)
{
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

void *get_address(struct sockaddr *socketaddress)
{
	if (socketaddress->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)socketaddress)->sin_addr);
	}
	return &(((struct sockaddr_in6*)socketaddress)->sin6_addr);
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

unsigned min(unsigned i, unsigned j)
{
	if(i<j)
		return i;
	else
		return j;
}

unsigned max(unsigned i, unsigned j)
{
	if(i>j)
		return i;
	else
		return j;
}
