#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <windows.h>
#include "shared.h"

BYTE* read_file_bytes(char* path, long* fp_len) {
  FILE* fp;
  fp = fopen(path, "rb+");
  fseek(fp, 0, SEEK_END);
  *fp_len = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  BYTE* fp_buf = (BYTE*)malloc(sizeof(BYTE) * (*(fp_len)));
  fread(fp_buf, sizeof(BYTE), *fp_len, fp);
  fclose(fp);

  printf("\nthis is the len of the ss.bmp : %ld\n", *fp_len);
  return fp_buf;
}

BYTE* write_file_bytes(char* path, long* fp_len, SOCKET socket, int alloc) {
  BYTE* file_bytes = (BYTE*)malloc(sizeof(BYTE) * (*(fp_len)));
  long bytes_recv = 0;
  while(bytes_recv < fp_len) {
    bytes_recv += (long)recv(socket, file_bytes + bytes_recv, fp_len - bytes_recv, 0);
    printf("\nbytes recv: %ld\n", bytes_recv);
  }
  
  FILE* fp;
  fp = fopen(path, "wb+");
  fwrite(file_bytes, sizeof(BYTE), fp_len, fp);
  fclose(fp);

  if(alloc == 1)
    return file_bytes;

  free(file_byte);

  return 0;
}

