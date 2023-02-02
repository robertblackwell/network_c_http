#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <c_http/common/alloc.h>
#include <c_http/unittest.h>
#include <c_http/common/cbuffer.h>
#include <c_http/logger.h>
#include <c_http/common/list.h>
#include <c_http/saved/sync_server.h>
#include <c_http/common/hdrlist.h>
#include <c_http/client.h>
#include <c_http/common/message.h>
#include <c_http/reader.h>
#include <c_http/writer.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char* req1[] = {
(char *) "GET /target HTTP/1.1\r\n",
(char *) "Host: ahost\r\n",
(char *) "Connection: close\r\n",
(char *) "Content-length: 0\r\n\r\n",
NULL
};

int test_client_01()
{
    ClientRef client = Client_new();
    Client_connect(client, "localhost", 9001);
    MessageRef response = Message_new();
    Client_roundtrip(client, req1, &response);
    CbufferRef cb = BufferChain_compact(Message_get_body(response));
    return 0;
}

int client(int argc, char *argv[])
{
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        printf("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        printf("ERROR connecting");
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
        printf("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
        printf("ERROR reading from socket");
    printf("%s\n",buffer);
}

char* simple_response_body(char* message, socket_handle_t socket, int pthread_self_value)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char dt_s[64];
    assert(strftime(dt_s, sizeof(dt_s), "%c", tm));

    char* body = "<html>"
                 "<head>"
                 "</head>"
                 "<body>"
                 "%s"
                 "<p>Date/Time is %s</p>"
                 "<p>socket: %d</p>"
                 "<p>p_thread_self %ld</p>"
                 "</body>"
                 "</html>";

    char* s1;
    int len1 = asprintf(&s1, body, message, dt_s, socket, pthread_self_value);

    return s1;
}
#ifdef lslsl
typedef struct X_s {
    socket_handle_t socket;
} X, *XRef;
X wrtr_s = {42};
XRef wrtr = &wrtr_s;

void Writer_start(HttpStatus status, HdrListRef headers)
{
    const char* reason_str = http_status_str(status);
    char* first_line = NULL;
    int len = asprintf(&first_line, "HTTP/1.1 %d %s\r\n", status, reason_str);
    if(first_line == NULL) goto failed;

    CbufferRef cb_output_ref = NULL;
    if((cb_output_ref = Cbuffer_new()) == NULL) goto failed;
    CbufferRef serialized_headers = NULL;
    serialized_headers = HdrList_serialize(headers);

    Cbuffer_append(cb_output_ref, (void*)first_line, len);
    /// this is clumsy - change HdrList_serialize() to deposit into an existing ContigBuffer
    Cbuffer_append(cb_output_ref, Cbuffer_data(serialized_headers), Cbuffer_size(serialized_headers));
    Cbuffer_append_cstr(cb_output_ref, "\r\n");
    int x = len+2;

    free(first_line);
    Cbuffer_dispose(&serialized_headers);
    Cbuffer_dispose(&cb_output_ref);
    return;
    failed:
    if(first_line != NULL) free(first_line);
    if(serialized_headers != NULL) Cbuffer_dispose(&serialized_headers);
    if(cb_output_ref != NULL) Cbuffer_dispose(&cb_output_ref);

}


int test_handle_request()
{
    printf("Handle request\n");
    char* msg = "<h2>this is a message</h2>";
    char* body = simple_response_body(msg, wrtr->socket, pthread_self());
    int body_len = strlen(body);
    char* body_len_str;
    asprintf(&body_len_str, "%d", body_len);
    HdrListRef hdrs = HdrList_new();
    KVPairRef hl_content_length = KVPair_new(HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str));
    HdrList_add_front(hdrs, hl_content_length);
    char* content_type = "text/html; charset=UTF-8";
    KVPairRef hl_content_type = KVPair_new(HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type));
    HdrList_add_front(hdrs, hl_content_type);

//    Writer_start(wrtr, HTTP_STATUS_OK, hdrs);
//    Writer_write_chunk(wrtr, (void*) body, body_len);

    HdrList_dispose(&hdrs);
//    KVPair_dispose(&(hl_content_length));
//    KVPair_dispose(&(hl_content_type));
    free(body);
    free(body_len_str);

    return 0;
}
#endif
int main()
{
    UT_ADD(test_client_01);
    int rc = UT_RUN();
    return rc;
}