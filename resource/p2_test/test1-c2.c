#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pfs.h"

/* Client #2 */
/* Open an existing file writes another 4 KB of data to the end of the file */
/* It takes a source file as a command line argument. Data from source
   file is written on to the new pfs_file */
/* test1-c1.c, test1-c2.c test1-c3.c must be executed one after the other*/

#define ONEKB 1024

int main(int argc, char *argv[])
{
  int ifdes, fdes;
  int err_value;
  char input_fname[20];
  char *buf;
  ssize_t nread;
  int cache_hit;

  // the command line arguments include an input filename
  if (argc != 2)
    {
      printf("usage: a.out <input filename>\n");
      exit(0);
    }
  strcpy(input_fname, argv[1]);
  ifdes = open(input_fname, O_RDONLY);
  buf = (char *)malloc(5*ONEKB);
  nread = pread(ifdes, (void *)buf, 4*ONEKB,4*ONEKB);

  // All the clients open the pfs file 
  fdes = pfs_open("pfs_file1", 'w');
  if(fdes < 0)
    {
      printf("Error opening file\n");
      exit(0);
    }

  //At Client 2
  //Write the next 4 kbytes of data from the input file onto pfs_file
  err_value = pfs_write(fdes, (void *)buf, 4*ONEKB, 4*ONEKB, &cache_hit);
  printf("Wrote %d bytes to the file\n", err_value);
 
  pfs_close(fdes);
  free(buf);
  close(ifdes);
}
