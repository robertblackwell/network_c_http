#ifndef h_kq_asio_forward_h
#define h_kq_asio_forward_h
typedef struct AsioStream_s AsioStream, *AsioStreamRef;
typedef struct AsioListener_s AsioListener, *AsioListenerRef;
typedef void(*AsioReadcallback)(void* arg, long length, int error_number);
typedef void(*AsioWritecallback)(void* arg, long length, int error_number);


#endif