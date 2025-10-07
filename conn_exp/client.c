#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
struct sockaddr_in server;
int lfd;
char r_buff[100] = "", s_buff[100] = "";

lfd = socket(AF_INET, SOCK_STREAM, 0);
server.sin_family = AF_INET;
server.sin_port = 9002;
server.sin_addr.s_addr = inet_addr("127.0.0.1");

connect(lfd, (struct sockaddr *)&server, sizeof server);

while (1) {
    printf("\nclient: ");
    gets(s_buff);
    send(lfd, s_buff, sizeof s_buff, 0);
    if (strcmp(s_buff, "end") == 0)
    break;

    recv(lfd, r_buff, sizeof r_buff, 0);
    printf("[server] %s", r_buff);
    if (strcmp(r_buff, "end") == 0)
    break;

    printf("\n");
}

close(lfd);

return 0;
}