
#include <c_eg/buffer/contig_buffer.h>
#include <c_eg/logger.h>
#include <c_eg/server.h>
#include <string>


TEST_CASE("buffer_chain_assignment")
{
	printf("IN test case\n");
	ServerRef sref = Server_new(9001);
	printf("Server created\n");
	Server_listen(sref);
	printf("after listen\n");
}
