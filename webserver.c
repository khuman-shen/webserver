#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const int port = 8080;
const char *ip = "127.0.0.1";
#define MAX_BUFFER 1024

// This will read until EOF and assigns the buffer with the readed character but
// it does not increase the buffer size!
int readUntilEOF(FILE *fd, char *buffer) {
  int len = 0;
  char c;
  while ((c = fgetc(fd)) != EOF) {
    buffer[len++] = c;
  }
  buffer[len] = '\0';
  return len;
}

int main(int argc, char **argv) {

  // usage
  if (argc < 2 || argc > 2) {
    printf("\tUsage: %s <html_file>\n", argv[0]);
    exit(0);
  }

  FILE *fp;
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // argv[1] is the html file
  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("File does not exist or failed to read\n");
    exit(0);
  }
  char html[MAX_BUFFER];
  memset(html, 0, sizeof(html)); // clear buffer
  int htmlLen = readUntilEOF(fp, html);

  char buffer[MAX_BUFFER];
  int headerLen = snprintf(buffer, sizeof(buffer),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html; charset=UTF-8\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n\r\n",
                           htmlLen);

  memcpy(buffer + headerLen, html,
         htmlLen); // buffer+headerLen : Setting the starting point where the
                   // memcpy gonna start, buffer[0] contains header, we need to
                   // make sure it does not overwrite the headers

  struct sockaddr_in sockAddr;
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_port = htons(port);
  inet_pton(AF_INET, ip, &sockAddr.sin_addr.s_addr);

  if (bind(fd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) < 0) {
    perror("Bind Failed\n");
    exit(0);
  }

  printf("WebServer Started: %s:%d\nHosting : %s\n", ip, port, argv[1]);

  int clientFd;
  struct sockaddr_in clientAddr;
  socklen_t clientLen = sizeof(clientAddr);

  listen(fd, 10);
  while (1) {
    clientFd = accept(fd, (struct sockaddr *)&clientAddr, &clientLen);
    printf("Client Connect\n");
    send(clientFd, buffer, headerLen + htmlLen, 0);
  }
  close(fd);
  return 0;
}
