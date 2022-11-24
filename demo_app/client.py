import  socket
import time
import pprint

def delay():
    # return 
    time.sleep(0.0000001)

def test_check(test_name: str, expected: bytes, result: bytes):
    if expected == result:
        print("Test {} success".format(test_name))
    else:
        print("Test {} failed Expected : {} got : {}".format(test_name, expected, result))

def make_big_frame(opcode:str, body: str, repeat:int):
    frame = ""
    frame = "\x01" + opcode + "\x02" + body*repeat + "\x03" + "L"
    return frame

def make_frame(opcode: str, body: str):
    frame = ""
    frame = "\x01" + opcode + "\x02" + body + "\x03" + "L"
    return frame

def make_bytes_response_frame(body: str):
    frame = ""
    frame = "\x01" + 'R' + "\x02" + body + "\x03" + "L" + "\x04"
    return bytes(frame, 'utf-8')
def make_big_bytes_response_frame(body: str, repeat:int):
    frame = ""
    frame = "\x01" + "R" + "\x02" + body*repeat + "\x03" + "L" + "\x04"
    return bytes(frame, 'utf-8')


def fragment_frame(frame: str, nbr_fragments: int):
    ln = len(frame)
    frag_size = ln // nbr_fragments    
    frags = []
    for i in range(nbr_fragments - 1):
        pos = i * frag_size
        f = frame[pos: pos + frag_size]
        frags.append(f)
    final_pos = (nbr_fragments - 1) * frag_size
    frags.append(frame[final_pos:])
    return frags
    


def send_fragments(sock, frags):
    for frag in frags:
        sock.sendall(bytes(frag, "utf-8"))
        # time.sleep(0.5)        

def send_frame_as_fragments(sock: socket.socket, frame:str, number_fragments):
    send_fragments(sock, fragment_frame(frame, number_fragments))        

def test_simple():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 9011))
    for i in range(1):
        s.sendall(b"\x01Q\x02123456789\x03L")
        data = s.recv(1024)
        expected = b"\x01R\x02123456789\x03L\x04"
        if data != expected:
            print("test_simple failed expected : {} got: {}".format(expected, data))
        else:
            print("test_simple Passed")
        s.close()

def test_simple_multiple_buffers():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 9011))
    for i in range(1):
        s.sendall(b"\x01Q\x02123456789abcdefghijklmno")
        # delay()
        s.sendall(b"ABCDEFGHIJK\x03L")
        # delay()
        data = s.recv(3*1024)
        expected = make_bytes_response_frame("123456789abcdefghijklmnoABCDEFGHIJK")
        test_check("test_simple_multiple_buffers", expected, data)
    s.close()

def test_message_data_left_in_buffer():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 9011))
    for i in range(2):
        s.sendall(b"\x01Q\x02123456789abcdefghijklmno\x03L\x01Q\x02MNBVCXZ")
        # delay()
        s.sendall(b"ABCDEFGHIJK\x03L")
        delay()
        data = s.recv(3*1024)
        s.settimeout(3)
        try:
            data2 = s.recv(3*1024)
            data_total = data + data2
        except:
            data_total = data
        
        expected = make_bytes_response_frame("123456789abcdefghijklmno") + make_bytes_response_frame("MNBVCXZABCDEFGHIJK")
        test_check("test_message_data_left_in_buffer", expected, data_total)
    s.close()
        

def test_send_fragments():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 9011))
    for i in range(5):
        frame = make_frame("Q", "[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]")
        send_frame_as_fragments(s, frame, 4)
        data = s.recv(1024)
        expected = make_bytes_response_frame("[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]")
        test_check("test_send_fragments", expected, data)
    s.close();
def test_send_big_fragments():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 9011))
    for i in range(1):
        frame = make_big_frame("Q", "[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]", 10)
        send_frame_as_fragments(s, frame, 1)
        data = s.recv(100000)
        expected = make_big_bytes_response_frame("[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]", 10)
        test_check("test_send_big_fragments", expected, data)
    s.close()

    
# expected = make_frame("R", "[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]")

test_simple()
test_message_data_left_in_buffer()
test_simple_multiple_buffers()
test_send_big_fragments()
