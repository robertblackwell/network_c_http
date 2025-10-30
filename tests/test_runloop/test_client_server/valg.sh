valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --verbose \
  --log-file=valgrind-out.txt \
  ../../../build/tests/test_runloop/test_client_server_original/test_echo