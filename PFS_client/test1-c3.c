#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pfs.h"

/* Client #3 */
/* Open an existing file and reads the file statistics */
/* Writes another 3 KB of data to the end of the file */
/* Reads 2 KB of data from the middle of the file */
/* It takes a source file as a command line argument. Data from source
   file is written on to the new pfs_file */
/* test1-c1.c, test1-c2.c test1-c3.c must be executed one after the other*/

#define ONEKB 1024

int main_c3(int argc, char *argv[])
{
	argc=2;argv[1]="1";
	initialize(argc,argv);
  int ifdes, fdes;
  int err_value;
  char input_fname[20];
  char *buf;
  ssize_t nread;
  struct pfs_stat mystat;
  int cache_hit;

  // the command line arguments include an input filename
  if (argc != 2)
    {
      printf("usage: a.out <input filename>\n");
      exit(0);
    }
  strcpy(input_fname, argv[1]);
  ifdes = open(input_fname, O_RDONLY);
  buf = (char *)malloc(4*ONEKB);
  nread = pread(ifdes, (void *)buf, 3*ONEKB,8*ONEKB);

  // All the clients open the pfs file 
  fdes = pfs_open("pfs_file1", "w");
  if(fdes < 0)
    {
      printf("Error opening file\n");
      exit(0);
    }

  //At client 3: print the file metadata
  pfs_fstat(fdes, &mystat);
  printf("File Metadata:\n");
  printf("Time of creation: %s\n", ctime(mystat.pst_ctime));
  printf("Time of last modification: %s\n", ctime(mystat.pst_mtime));
  printf("File Size: %d\n", mystat.pst_size);

  //Write the next 3 kbytes of data from the input file onto pfs_file
  err_value = pfs_write(fdes, (void *)buf, 3*ONEKB, 8*ONEKB, &cache_hit);
  printf("Wrote %d bytes to the file\n", err_value);

  err_value = pfs_read(fdes, (void *)buf, 2*ONEKB, ONEKB, &cache_hit);
  printf("Read %d bytes of data from the file\n", err_value);
  printf("%s\n",buf);

  pfs_close(fdes);
  pfs_delete("pfs_file1");
  free(buf);
  close(ifdes);
}
