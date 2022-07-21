#ifndef SHARED_H
#define SHARED_H

BYTE* read_file_bytes(char* path, long* fp_len);
BYTE* write_file_bytes(char* path, long* fp_len, SOCKET socket, int alloc);

#endif
