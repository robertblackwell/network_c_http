#include <c_http/sync/client.h>
#include <c_http/alloc.h>
#include <c_http/buffer/cbuffer.h>
#include <c_http/message.h>
#include <c_http/sync/ll_reader.h>
#include <c_http/sync/writer.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


ClientRef Client_new()
{
    ClientRef this = eg_alloc(sizeof(Client));
    this->parser = NULL;
    this->rdr = NULL;
    this->wrtr = NULL;
}
void Client_free(ClientRef* this_ptr)
{
    ClientRef this = *this_ptr;
    if(this->parser) Parser_free(&(this->parser));
    if(this->rdr) Reader_free(&(this->rdr));
    if(this->wrtr) Writer_free(&(this->wrtr));
    close(this->sock);
    eg_free(*this_ptr);
    *this_ptr = NULL;
}
void Client_connect(ClientRef this, char* host, int portno)
{
    int sockfd, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        printf("ERROR opening socket");
    server = gethostbyname(host);
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
    this->sock = sockfd;
    this->wrtr = Writer_new(sockfd);
    this->parser = Parser_new();
    RdSocket rdsock = RealSocket(sockfd);
    this->rdr = Reader_new(this->parser, rdsock);

}
void Client_roundtrip(ClientRef this, const char* req_buffers[], MessageRef* response_ptr)
{
    int buf_index = 0;
    int buf_len;
    char* buf;
    while(req_buffers[buf_index] != NULL) {
        buf = (char*)req_buffers[buf_index];
        buf_len = strlen(buf);
        int bytes_written = write(this->sock, buf, buf_len);
        buf_index++;
    }
    int rc = Reader_read(this->rdr, response_ptr);
    BufferChainRef bc = Message_get_body(*response_ptr);
    if(bc == NULL) {
        bc = BufferChain_new();
    }

    IOBufferRef cb = BufferChain_compact(bc);

    close(this->sock);
}

