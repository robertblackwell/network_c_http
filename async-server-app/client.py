import socket
import time, datetime
import pprint
import sys
import getopt

import statistics
# import urllib3


class ThreadContext:
    def __init__(self):
        self.intervals = []
        self.nbr_connections = 0
        self.nbr_requests = 0
        pass


def socket_recv(s):
    data = ""
    while True:
        b = s.recv(1024).decode()
        data += b
        if len(b) < 1024:
            break
    return data


def test_client(port, nbr_connections: int, nbr_requests_per_connection: int, ctx: ThreadContext):
    for c in range(nbr_connections):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        rc = s.connect_ex(('localhost', port))

        if rc != 0:
            raise RuntimeError("connect failed")
        for r in range(nbr_requests_per_connection):
            connection_type = "keep-alive"
            if r == nbr_requests_per_connection - 1:
                connection_type = "close"
            start = datetime.datetime.now()
            buffer1 = "GET /echo HTTP/1.1\r\nCONNECTION: {}\r\nJUNK: {}.{}\r\n\r\n".format(connection_type, c,
                                                                                           r).encode()
            s.sendall(buffer1)
            data1 = socket_recv(s)
            end_time = datetime.datetime.now()
            elapsed = (end_time - start).microseconds
            ctx.intervals.append(elapsed)
        s.close()


# def http_test_client(port, nbr_connections: int, nbr_requests_per_connection: int, ctx: ThreadContext):
#     import http
#     headers = {
#     }
#     for c in range(nbr_connections):
#         connection = http.client.HTTPConnection("localhost", port)
#         connection.connect()
#         for r in range(nbr_requests_per_connection):
#             connection_type = "keep-alive"
#             if r == nbr_requests_per_connection - 1:
#                 connection_type = "close"
#             headers["CONNECTION"] = connection_type
#             headers["JUNK"] = "{}-{}".format(c, r)
#             start = datetime.datetime.now()
#             buffer1 = "GET /echo HTTP/1.1\r\nCONNECTION: {}\r\nJUNK: {}.{}\r\n\r\n".format(connection_type, c,
#                                                                                            r).encode()
#             connection.request("GET", "/echo", None, headers=headers)
#             data1 = connection.getresponse()
#             chunk = data1.read(20000)
#             end_time = datetime.datetime.now()
#             elapsed = (end_time - start).microseconds
#             ctx.intervals.append(elapsed)
#         connection.close()


def run_threads(port, nbr_threads, nbr_connections_per_thread, nbr_requests_per_connection):
    import threading
    intervals = []
    threads = []
    contexts = []
    for t in range(nbr_threads):
        ctx = ThreadContext()
        contexts.append(ctx)
        thread = threading.Thread(None, test_client, None,
                                  [port, nbr_connections_per_thread, nbr_requests_per_connection, ctx])
        threads.append(thread)

    for th in threads:
        th.start()

    for th in threads:
        th.join()

    for ctx in contexts:
        intervals = intervals + ctx.intervals

    print("Threads: {} Connections Per thread: {} requests per Connection: {} ".format(
        nbr_threads, nbr_connections_per_thread, nbr_requests_per_connection))
    print("Total number of requests {} {}".format(len(intervals),
                                                  nbr_threads * nbr_connections_per_thread * nbr_requests_per_connection))
    print("Elapsed time mean: {} us std-dev: {} us".format(statistics.mean(intervals),
                                                           statistics.stdev(intervals, statistics.mean(intervals))))

    print("Done")


def usage():
    print("Name: test_client")
    print("Purpose: Test a webserver by sending a number of GET requests to localhost on the given port")
    print("Options/Aruments")
    print(" ")

    print("\t-p\tport")
    print("\t-t\tnumber of threads")
    print("\t-c\tnumber of connections per threads")
    print("\t-t\tnumber of requests per connection")
    print("\t-h\tprints this help message")


def test_client_main():
    argv = sys.argv[1:]
    try:
        opts, args = getopt.getopt(argv, "p:t:c:r:h")

    except:
        pass

    for opt, arg in opts:
        if opt in ["-p"]:
            port = int(arg)
        elif opt in ["-t"]:
            nbr_threads = int(arg)
        elif opt in ["-c"]:
            nbr_connections_per_thread = int(arg)
        elif opt in ["-r"]:
            nbr_requests_per_connection = int(arg)
        else:
            usage()
            sys.exit(-1)

    run_threads(port, nbr_threads, nbr_connections_per_thread, nbr_requests_per_connection)


test_client_main()
