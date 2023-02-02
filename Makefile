BUILD_DIR := ./build
SRC_DIR := ./c_eg
INC_DIRS := ./c_eg ./vendor/src
INC_FLAGS := $(addprefix -I, $(INC_DIRS))


STATIC := $(BUILD_DIR)/libeg.a
LIBSRCS := c_eg/buffer/contig_buffer.c \
		c_eg/buffer/buffer_chain.c \
		c_eg/buffer/iobuffer.c \
		c_eg/alloc.c \
		c_eg/client.c \
		c_eg/datasource.c \
		c_eg/handler_example.c \
		c_eg/hdrlist.c \
		c_eg/kvpair.c \
		c_eg/list.c \
		c_eg/logger.c \
		c_eg/message.c \
		c_eg/parser.c \
		c_eg/parser_test.c \
		c_eg/queue.c \
		c_eg/rdsocket.c \
		c_eg/reader.c \
		c_eg/server.c \
		c_eg/unittest.c \
		c_eg/url.c \
		c_eg/utils.c \
		c_eg/worker.c \
		c_eg/writer.c \
		vendor/src/http-parser/http_parser.c

SVRSRCS := app/simple_server.c

LIBOBJS := $(LIBSRCS:%.c=$(BUILD_DIR)/%.o)
LIBHDRS := $(LIBSRCS:%.c=$(SRC_DIR)/%.h)

# 
# build c_eg library build/libeg.a
# 
$(STATIC): $(LIBOBJS)
	@echo "[Link (Static)]"
	@ar rcs $@ $^


$(LIBOBJS): $(BUILD_DIR)/%.o: %.c | mkdirs
	@echo $< $@
	gcc -pthread -std=c11 -I./ -I./vendor/src -c $< -o $@

# 
# build build/sync_app/simple_server
# 
main: $(LIBOBJS) $(STATIC)
	@echo main
	gcc ./app/app.c -pthread -I./ -I./vendor/src -o $(BUILD_DIR)/app/simple_server $(BUILD_DIR)/libeg.a


mkdirs:
	@mkdir -p build/c_eg/buffer
	@mkdir -p build/app
	@mkdir -p build/lib
	@mkdir -p build/vendor/src/http-parser

clean:
	rm -rf build 