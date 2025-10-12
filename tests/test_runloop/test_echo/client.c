#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

typedef struct Params_s {
    bool i_flag;
    long nbr_threads;
    long nbr_connections;
    long nbr_consecutive_msgs;
} Params;

Params params  = {.i_flag = true, .nbr_threads = 1, .nbr_connections = 1, .nbr_consecutive_msgs = 1};

int interactive_main();
int parse_args(int argc, char* argv[], Params* a);
int get_option_param(char* token, long* value);
void help();

int main(int argc, char* argv[])
{
    int status = parse_args(argc, argv, &params);
    if (status != 0) {
        help();
        exit(-1);
    }
    if (params.i_flag) {
        interactive_main();
    } else {

    }
}


void help() {
    printf("This program only supports short options\n"
        "\t-i interactive mode\n"
        "\t-t<n> number of threads to run\n"
        "\t-c<n> number of consecutive connections per thread\n"
        "\t-m<n> number of consecutive msgs per connection)\n"
        "\t-d<n> number of seconds between consecutive messages");
}
int get_option_param(char* token, long* value)
{
    char* p = &token[2];
    char* end_p;
    long v = strtol(p, &end_p, 10);
    if ((p == end_p) || (*end_p != '\0')) {
        return -1;
    }
    *value = v;
    return 0;
}
int parse_args(int argc, char** argv, Params* a)
{
    int status = 0;
    for(int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (arg[0] == '-') {
            switch(arg[0]) {
                case 'i':
                    a->i_flag = true;
                    return 0;
                    break;
                case 't':
                    status = get_option_param(arg, &(a->nbr_threads));
                    break;
                case 'c':
                    status = get_option_param(arg, &(a->nbr_connections));
                    break;
                case 'm':
                    status = get_option_param(arg, &(a->nbr_consecutive_msgs));
                    break;
                case 'h':
                default:
                    help();
                    exit(0);
                    break;
            }
        } else {
            status = -1;
            help();
            exit(-1);
        }
    }
    return status;
}
int auto_main(int argc, char* argv[])
{
    return 0;
}
int interactive_main() {
    struct sockaddr_in server;
    int lfd;
    char r_buff[100] = "", s_buff[100] = "";

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = 9002;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    int status = connect(lfd, (struct sockaddr *)&server, sizeof server);
    assert(status == 0);
    while (1) {
        printf("\nclient: ");
        fgets(s_buff, 100, stdin);
        if (strcmp(s_buff, "end\n") == 0)
            break;
        char buffer[1000];
        size_t x = snprintf(buffer, 800, "%s", s_buff);
        size_t lenx = strlen(buffer);
        printf("sending len:%lu msg:%s", lenx, buffer);
        send(lfd, s_buff, lenx, 0);

        recv(lfd, r_buff, sizeof r_buff, 0);
        printf("[server] %s", r_buff);
        if (strcmp(r_buff, "end") == 0)
            break;

        printf("\n");
    }

    close(lfd);

    return 0;
}