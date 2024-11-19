//
// Created by robert on 11/10/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "verify_getopt.h"
//#include <rbl/logger.h>

void verify_usage()
{
    printf("Name: demo_verify\n");
    printf("\nDescription\n");
    printf("\tThis is a multi-process multi-threaded async_client that sends receive STX.....ETX messages \n");
    printf("\tto a server and measures the response time of each request and amalgamates those  \n");
    printf("\treading into summary statistics.  \n");
    printf("\n");
    printf("\tThat is this program creates a number (-t) of threads. Each thread connects to the server a number of  \n");
    printf("\ttimes (-c) and sends a requests/recieves a response (called a roundtrip) a number of times (-r). \n");
    printf("\n");
    printf("\tThe total number of request/response cycles is (-t) * (-c) * (-r)\n");
    printf("\n");
    printf("\nOptions\n");

    printf("\t-h\tPrints this usage message. Does not take an argument\n");
    printf("\t-i\tHost ip address                                          - default 127.0.0.1\n");
    printf("\t-p\tPort number to use                                       - default 9011\n");
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
//        printf("c = %c\n", (char)c);
        switch (c) {
            case 'n':
//                RBL_LOG_FMT("-t options %s", optarg);
                nbr_processes = atoi(optarg);
                break;
            case 't':
//                RBL_LOG_FMT("-t options %s", optarg);
                nbr_threads = atoi(optarg);
                break;
            case 'r':
//                RBL_LOG_FMT("-r options %s", optarg);
                nbr_roundtrips_per_connection = atoi(optarg);
                break;
            case 'c':
//                RBL_LOG_FMT("-c options %s", optarg);
                nbr_connections_per_thread = atoi(optarg);
                break;
            case 'p':
//                RBL_LOG_FMT("-p options %s", optarg);
                port_number = atoi(optarg);
                break;
            case 'h':
//                RBL_LOG_FMT("-h options %s", optarg);
                int len = (int)strlen(optarg);
                host_ptr = malloc(len+1);
                strcpy(host_ptr, optarg);
//                port_number = atoi(optarg);
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
//    *nbr_processes_p = nbr_processes;
}
