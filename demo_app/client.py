import  socket
import time
import pprint

host = 'localhost'
port = 9011

def delay():
    # return 
    time.sleep(0.0000001)

def make_frame(opcode: str, body: str) -> bytes:
    assert(len(opcode) == 1)
    frame = "\x02" + opcode + body + "\x03"
    return bytes(frame, 'utf-8')

def test_check(test_name: str, expected: bytes, result: bytes):
    if expected == result:
        print("Test {} success".format(test_name))
    else:
        print("Test {} failed Expected : {!r} got : {!r}".format(test_name, expected, result))

def make_big_frame(opcode:str, body: str, repeat:int):
    frame = make_frame(opcode, body*repeat)
    return frame


def make_bytes_response_frame(body: str)->bytes:
    frame = make_frame('R', body)
    return frame

def make_big_bytes_response_frame(body: str, repeat:int):
    frame = make_frame('R', body*repeat)
    return frame


def fragment_frame(frame: str, nbr_fragments: int):
    ln = len(frame)
    frag_size = ln // nbr_fragments    
    if nbr_fragments == 1:
        return frame
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
    r = s.connect_ex((host, port))
    print(f"connect result: {r}");
    for i in range(3):
        m = make_frame('Q', '123456789')
        s.sendall(m)
        data = s.recv(1024)
        expected = make_frame('R', '123456789')
        if data != expected:
            print("test_simple failed expected : {} got: {}".format(expected, data))
        else:
            print("test_simple Passed")
    s.close()

def test_frame_error_simple():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    for i in range(1):
        f = make_frame('Q', '123456789')
        expected = make_frame('R', '123456789')
        s.sendall(f)
        data = s.recv(1024)
        if data != expected:
            print("test_simple failed expected : {} got: {}".format(expected, data))
        else:
            print("test_simple Passed")
        s.close()

def test_error_close_during_frame():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    
    s.sendall(b"\x01Q\x02123456789poiuytrew")
    s.close()
    print("End of test_error_close_during_frame")
                
def test_error_close_after_frame():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    
    s.sendall(b"\x01Q\x02123456789poiuytrew\x03L")
    s.close()
    print("End of test_error_close_during_frame")
                
def test_error_close_during_receive():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    s.sendall(b"\x01Q\x02123456789poiuytrew\x03L")
    data = s.recv(3);
    s.close();
    print("End of test_error_close_during_receive")
                
        
def test_simple_multiple_buffers():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    for i in range(1):
        f1 = b'\x02Q123456789abcdefghijklmno'
        s.sendall(f1)
        # delay()
        s.sendall(b"ABCDEFGHIJK\x03")
        # delay()
        data = s.recv(3*1024)
        expected = make_bytes_response_frame("123456789abcdefghijklmnoABCDEFGHIJK")
        test_check("test_simple_multiple_buffers", expected, data)
    s.close()

def test_message_data_left_in_buffer():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    for i in range(2):
        s.sendall(b"\x01Q\x02123456789abcdefghijklmno\x03L\x01Q\x02MNBVCXZ")
        # delay()
        s.sendall(b"ABCDEFGHIJK\x03L")
        delay()
        data = s.recv(3*1024)
        s.settimeout(5)
        try:
            data2 = s.recv(3*1024)
            data_total = data + data2
        except:
            data_total = data
        
        expected = make_bytes_response_frame("123456789abcdefghijklmno") + make_bytes_response_frame("MNBVCXZABCDEFGHIJK")
        test_check("test_message_data_left_in_buffer", expected, data_total)
    s.close()
        
def test_send_2_frames_1_buffer():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    for i in range(1):
        frame1 = make_frame("Q", "[mnbvcxzlkjhgfdsapoiuytrewq]")
        frame2 = make_frame("Q", "[QWERTYUIOPASDFGHJKLZXCVBNM]")
        frame = b"\x01Q\x02[mnbvcxzlkjhgfdsapoiuytrewq]\x03L\x01Q\x02[QWERTYUIOPASDFGHJKLZXCVBNM]\x03L"
        s.sendall(frame)
        data = s.recv(3*1024)
        s.settimeout(3)
        try:
            data2 = s.recv(3*1024)
            data_total = data + data2
        except:
            data_total = data
        expected = b"\x01R\x02[mnbvcxzlkjhgfdsapoiuytrewq]\x03L\x04\x01R\x02[QWERTYUIOPASDFGHJKLZXCVBNM]\x03L\x04"
        test_check("test_send_fragments", expected, data_total)
    s.close()

def test_send_fragments():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    for i in range(5):
        frame = make_frame("Q", "[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]")
        send_frame_as_fragments(s, frame.decode('utf-8'), 4)
        data = s.recv(1024)
        expected = make_bytes_response_frame("[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]")
        test_check("test_send_fragments", expected, data)
    s.close()

def test_send_big_fragments():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    for i in range(1):
        frame = make_big_frame("Q", "[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]", 10)
        send_frame_as_fragments(s, frame.decode('utf-8'), 1)
        data = s.recv(100000)
        expected = make_big_bytes_response_frame("[mnbvcxzlkjhgfdsapoiuytrewq][QWERTYUIOPASDFGHJKLZXCVBNM]", 10)
        test_check("test_send_big_fragments", expected, data)
    s.close()

def test_errors():
    for i in range(1):
        test_error_close_during_receive()
        test_error_close_after_frame()
        test_error_close_during_frame()
        # test_frame_error_simple()
def test_good():
    for i in range(1):
        test_send_2_frames_1_buffer()
        test_simple()
        test_message_data_left_in_buffer()
        test_simple_multiple_buffers()
        test_send_big_fragments()
for j in range(3):
    test_simple()
# test_errors() 
# test_good()           
