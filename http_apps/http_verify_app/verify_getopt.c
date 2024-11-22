#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "verify_getopt.h"
//#include <rbl/logger.h>

void verify_usage()
{
    printf("Name: demo_verify\n");
    printf("\nDescription\n");
    printf("\tThis is a multi-process multi-threaded sync client that sends Demo request messages \n");
    printf("\tto a server, waits for the response, and measures the response time of each request.\n");
    printf("\tIt amalgamates those individual response times into summary statistics.  \n");
    printf("\n");
    printf("\tThat is this program creates a number (-n) processes each with (-t) threads. \n");
    printf("\tEach thread connects to the server (-c) times and performs (-r) requests/response cycles.\n");
    printf("\ton each connection.\n");
    printf("\n");
    printf("\tThe total number of request/response cycles is (-t) * (-c) * (-r)\n");
    printf("\n");
    printf("\nOptions\n");

    printf("\t-h\tPrints this usage message. Does not take an argument\n");
    printf("\t-i\tHost ip address                                          - default 127.0.0.1\n");
    printf("\t-p\tPort number to use                                       - default 9011\n");
    printf("\t-n\tNbr processes                                            - default 2\n");
    printf("\t-t\tNbr threads                                              - default 2\n");
    printf("\t-c\tNbr connections per thread                               - default 3\n");
    printf("\t-r\tNbr roundtrips per connection                            - default 3\n");

}
void verify_process_args(int argc, char* argv[], char** host_ip_p, int* port, int* nbr_roundtrips_per_connection_p, int* nbr_connections_per_thread_p, int* nbr_threads_p) {
    int c;
    char* host_ptr = NULL;
    int port_number = 9011;
    int nbr_processes = 1;
    int nbr_threads = 1;
    int nbr_connections_per_thread = 1;
    int nbr_roundtrips_per_connection = 1;
    while ((c = getopt(argc, argv, "n:h:r:c:p:t:")) != -1) {
        switch (c) {
            case 'n':
                nbr_processes = atoi(optarg);
                break;
            case 't':
                nbr_threads = atoi(optarg);
                break;
            case 'r':
                nbr_roundtrips_per_connection = atoi(optarg);
                break;
            case 'c':
                nbr_connections_per_thread = atoi(optarg);
                break;
            case 'p':
                port_number = atoi(optarg);
                break;
            case 'h':
                int len = (int)strlen(optarg);
                host_ptr = malloc(len+1);
                strcpy(host_ptr, optarg);
                break;
            case '?':
                printf("There was an error in the options list\n");
            default:
                verify_usage();
                exit(0);
        }
    }
    if(host_ptr != NULL) *host_ip_p = host_ptr;
    *port = port_number;
    *nbr_roundtrips_per_connection_p = nbr_roundtrips_per_connection;
    *nbr_connections_per_thread_p = nbr_connections_per_thread;
    *nbr_threads_p = nbr_threads;
}

