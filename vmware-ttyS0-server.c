#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define SOCKET_NAME "/home/marko/vmware/mikucos/ttyS0"

struct sockaddr addr;

char buffer[256];

#define STDIN_INDEX 0
#define SOCKET_INDEX 1

struct pollfd polls[] = {
  {0, POLLIN, 0},
  {0, POLLIN, 0}
};

struct termios t;

void restore_term(int sig) {
  tcgetattr(0, &t);
  t.c_lflag |= ICANON | ECHO;
  tcsetattr(0, 0, &t);

  printf("got signal %d\n", sig);

  exit(1);
}

int main(int argc, char* argv[]) {
   char * socket_name;
   
   if(argc < 2)
     socket_name = SOCKET_NAME;
   else
     socket_name = argv[1];
  unlink(socket_name);

  int s = socket(AF_UNIX, SOCK_STREAM, 0);
  strcpy(addr.sa_data, socket_name);
  addr.sa_family = AF_UNIX;
  bind (s, (struct sockaddr *) &addr,
	strlen(addr.sa_data) + sizeof (addr.sa_family));

  if(listen(s, 5) == -1) {
    perror("listen");
    exit(1);
  }

  signal(SIGSEGV, restore_term);
  signal(SIGTERM, restore_term);
  signal(SIGINT, restore_term);
  signal(SIGPIPE, restore_term);

  fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
  tcgetattr(0, &t);
  t.c_lflag &= ~ICANON;
  t.c_lflag &= ~ECHO;
  tcsetattr(0, 0, &t);

  while(1) {
    int fd = accept(s, 0, 0);
    printf("\n-------- connection accepted -------\n");
    
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

    polls[SOCKET_INDEX].fd = fd;

    while(1) {
      int pres = poll(polls, 2, -1);
      if(pres == -1) {
	perror("poll");
	exit(1);
      }
      
      if(polls[STDIN_INDEX].revents & POLLIN) {
	int res;
	while((res = read(0, buffer, 1)) > 0) {
	  /*	  buffer[res] = 0;
	  printf("\e[01;31m%s\e[00m", buffer);
	  fflush(stdout); */

	  write(fd, buffer, 1);
	}
      }
      
      if(polls[SOCKET_INDEX].revents & POLLHUP) {
	break;
      }
      if(polls[SOCKET_INDEX].revents & POLLERR) {
	  printf("ERROR!\n");
	break;
      }
      if(polls[SOCKET_INDEX].revents & POLLIN) {
	int res;
	while((res = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
	  buffer[res] = 0;
	  write(1, buffer, res);
	  /*	  printf("\e[01;32m%s\e[00m", buffer);
		  fflush(stdout); */
	}	
	if(res == -1) {	 
	  if(errno != EAGAIN ) {
	    printf("errno %d\n", errno); 
	    perror("read");
	    close(fd);
	    break;
	  }
	}
      } 
    }
    
    printf("\n-------- connection disconnected -------\n");
  }
  
  unlink(socket_name);
}
