#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
  char path[512];
  char wholename[512];
  char *next_piece;

  snprintf(path, 511, "/usr/bin:/bin:/tmp");
  next_piece = strtok(path, ":");
  printf("next piece = %s\n", next_piece);

  snprintf(wholename, 511, "%s/ls", next_piece);
  printf("looking for %s\n", wholename);
  if (access(wholename, X_OK) == 0) {
    printf("yey\n");
  }

  while (next_piece != NULL) {
    next_piece = strtok(NULL, ":");
    printf("next piece = %s\n", next_piece);
    snprintf(wholename, 511, "%s/ls", next_piece);
    printf("looking for %s\n", wholename);
    if (access(wholename, X_OK) == 0) {
    printf("yey\n");
    }
  }

return 0;
}
