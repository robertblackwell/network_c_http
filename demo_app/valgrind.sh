
#just wanted to save the valgrind command
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes ../cmake-build-debug/demo_app/demo_app
