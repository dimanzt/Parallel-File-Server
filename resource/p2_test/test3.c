#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pfs.h"

/* This test set assumes the following */
/* Client Cache Size: 1MB */
/* Existence of a file named pfs_file1 of size at least 2MB */

#define ONEKB 1024

int main(int argc, char *argv[])
{
  int fdes;
  int num_misses=0;
  off_t myoff = 0;
  char *buf;
  int cache_hit, go_on=1;
  struct pfs_stat mystat;
  ssize_t nread;
  int file_size;

  // Open the file in the write mode
  fdes = pfs_open("file", 'w');
  if(fdes < 0)
    {
      printf("Error opening file\n");
      exit(0);
    }
  pfs_fstat(fdes, &mystat);
  file_size = mystat.pst_size;

  buf = (char*)malloc(200*ONEKB);
  while(go_on)
    {
      nread = pfs_read(fdes, (void *)buf, 100*ONEKB, myoff%, &cache_hit);
      if(!cahce_hit)
		num_misses++;
      printf("Read %d bytes from the file\n", nread);
      printf("Number of cache misses = %d\n", num_misses);
      if ( (myoff+(100*ONEKB)) > file_size)
		myoff=0;
      else
	myoff+=(100*ONEKB);
      printf("Read again? (1 or 0):");
      scanf("%d", &go_on);
    }

  free(buf);
  pfs_close(fdes);
}
