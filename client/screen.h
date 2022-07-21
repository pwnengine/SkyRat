#ifndef SCREEN_H
#define SCREEN_H

BYTE* read_file(char* path, long* fp_len);
void get_screen_size(long* x, long* y);
void take_screenshot(char* path, SOCKET sock);

#endif
