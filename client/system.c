#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <lmcons.h>

void mem_details(char* memory) {
  MEMORYSTATUSEX mem;
  mem.dwLength = sizeof(mem);
  GlobalMemoryStatusEx(&mem);
  sprintf(memory, "%lld GB", mem.ullTotalPhys / 1073741824);
}

void cpu_details(char* speed) {
  DWORD buf_size = MAX_PATH;
  DWORD mhz = MAX_PATH;
  HKEY hkey;
  RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hkey);
  RegQueryValueEx(hkey, "~MHz", 0, 0, (LPBYTE)&mhz, &buf_size);
  double ghz = (double)mhz / 1000.f;
  sprintf(speed, "%.2f GHz", ghz);
}

void user_details(char* name) {
  DWORD len = (DWORD)(sizeof(name) / sizeof(name[0]));
  GetUserName(name, &len);
}
