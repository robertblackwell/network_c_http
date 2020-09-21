#include <c_eg/server.h>
#include <stdio.h>

int main()
{
    printf("Hello this is main \n");
    ServerRef sref = Server_new(9001);
    Server_listen(sref);

}