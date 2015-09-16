#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pfs.h"

/* This test set assumes the existence of a file named pfs_file1 of size at least 4KB*/
/* The same code is executed at multiple clients */
/* You can add some more clients which perform simultaneous read operations */

#define ONEKB 1024

int main(int argc, char *argv[])
{
  int fdes;
  int err_value;
  off_t myoff;
  char *buf;
  int cache_hit, go_on;
  ssize_t nread;

  // Open the file in the write mode
  fdes = pfs_open("file", 'w');
  if(fdes < 0)
    {
      printf("Error opening file\n");
      exit(0);
    }

  //Do overlapping writes onto the same pfs_file from multiple clients
  buf = (char*)malloc(ONEKB);
  while(go_on)
    {
      printf("Enter text to be written to the file\ntext>");
      scanf("%[^\n]s", buf);
      printf("Enter the offset:");
      scanf("%d", &myoff);
      err_value = pfs_write(fdes, (void *)buf, strlen(buf), myoff, &cache_hit);
      printf("Wrote %d bytes to the file\n", err_value);
      printf("Enter more text? (1 or 0):");
      scanf("%d", &go_on);
    }

  free(buf);
  pfs_close(fdes);
}
