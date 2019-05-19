/**
 * @file     x.cpp
 *           
 *
 * @author   lili  <lilijreey@gmail.com>
 * @date     05/04/2019 11:18:30 AM
 *
 */
#include <stdio.h>
#include <errno.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <sys/types.h>	       /* See NOTES */
       #include <sys/socket.h>
       #include <sys/un.h>
const char *SOCKNAME = "/tmp/mysock";

int sfd;
struct sockaddr_un addr;

void errExit(const char *msg)
{
  perror(msg);
  exit(1);
}

int main()
{
  sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sfd == -1)
    errExit("socket");

  /*  Create socket */
  memset(&addr, 0, sizeof(struct sockaddr_un));

  /*  Clear structure */
  addr.sun_family = AF_UNIX;

  /*  UNIX domain address */
  strncpy(addr.sun_path, SOCKNAME, sizeof(addr.sun_path) - 1);

  if (-1 == remove(SOCKNAME) && errno != ENOENT)
    errExit("remove failed");

  if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
    errExit("bind");

  if (-1 == listen(sfd, 5))
    errExit("listen failed");

  for (;;)
  {

  }

  sleep(10);
}
