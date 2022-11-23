import  socket
import time
import pprint

def make_big_frame(opcode:str, body: str, repeat:int):
    frame = ""
    frame = "x\01" + opcode + "\x02" + body*repeat + "\x03" + "L"
    return frame

def make_frame(opcode: str, body: str):
    frame = ""
    frame = "x\01" + opcode + "\x02" + body + "\x03" + "L"
    return frame

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
        time.sleep(1.0)
        data = s.recv(1024)
        pprint.pprint(data)
        expected = b"\x01R\x02123456789\x03L\x04"
        if data != expected:
            print("test_simple failed expected : {} got: {}".format(expected, data))
        else:
            print("test_simple Passed")
        print("Next loop")
        s.close()

def test_simple_multiple_buffers():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 9011))
    for i in range(1):
        s.sendall(b"\x01Q\x02123456789abcdefghijklmno")
        time.sleep(0.5)
        s.sendall(b"ABCDEFGHIJK\x03L")
        time.sleep(0.5)
        data = s.recv(1024)
        pprint.pprint(data)
        print("Next loop")
    s.close()

def test_message_data_left_in_buffer():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 9011))
    for i in range(2):
        s.sendall(b"\x01Q\x02123456789abcdefghijklmno\x03L\x01Q\x02MNBVCXZ")
        time.sleep(0.5)
        s.sendall(b"ABCDEFGHIJK\x03L")
        time.sleep(0.5)
        data = s.recv(1024)
        pprint.pprint(data)
        print("Next loop")
    s.close()
        

def test_send_fragments():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 9011))
    for i in range(5):
        frame = make_frame("Q", "[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]")
        send_frame_as_fragments(s, frame, 4)
        data = s.recv(1024)
        pprint.pprint(data)
        print("Next loop")
    s.close();
def test_send_big_fragments():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 9011))
    for i in range(1):
        frame = make_big_frame("Q", "[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]", 100)
        send_frame_as_fragments(s, frame, 1)
        data = s.recv(100000)
        pprint.pprint(data)
        print("loop complete sent: {} bytes received : {} bytes".format(len(frame), len(data)))
    s.close()

test_simple()
test_message_data_left_in_buffer();
test_simple_multiple_buffers()
test_send_big_fragments()
