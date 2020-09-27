#include <c_eg/client.h>
#include <c_eg/alloc.h>
#include <c_eg/buffer/contig_buffer.h>
#include <c_eg/message.h>
#include <c_eg/reader.h>
#include <c_eg/writer.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


Client* Client_new()
{
    Client* this = eg_alloc(sizeof(Client));
}
void Client_free(Client** this_ptr)
{
    Client* this = *this_ptr;
    Parser_free(&(this->parser));
    Rdr_free(&(this->rdr));
    Wrtr_free(&(this->wrtr));
    eg_free(*this_ptr);
    *this_ptr = NULL;
}
void Client_connect(Client* this, char* host, int portno)
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
    printf("Please enter the message: ");
    this->sock = sockfd;
    this->wrtr = Wrtr_new(sockfd);
    this->parser = Parser_new();
    RdSocket rdsock = RealSocket(sockfd);
    this->rdr = Rdr_new(this->parser, rdsock);

}
void Client_roundtrip(Client* this, char* req_buffers[], Message** response_ptr)
{
    int buf_index = 0;
    int buf_len;
    char* buf;
    while(req_buffers[buf_index] != NULL) {
        buf = req_buffers[buf_index];
        buf_len = strlen(buf);
        printf("Write %d %s\n", buf_len, buf);
        int bytes_written = write(this->sock, buf, buf_len);
        buf_index++;
    }
    int rc = Rdr_read(this->rdr, response_ptr);

    close(this->sock);
}

