#include <c_eg/server.h>
#include <stdio.h>
#include <mcheck.h>
int main()
{
    mcheck(NULL);
//    mcheck_check_all();
    printf("Hello this is main \n");
    ServerRef sref = Server_new(9001);
    Server_listen(sref);

}