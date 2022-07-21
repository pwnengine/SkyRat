CFLAGS=-m64 -std=c99 -pedantic -Wall
SERVERFILES=server/microui.c server/renderer.c server/server.c 
CLIENTFILES=client/client.c client/screen.c
SERVERBUILD=build/server/server.exe
CLIENTBUILD=build/client.exe

all: build

build: serverbuild clientbuild

serverbuild: $(SERVERFILES) 
	gcc $(CFLAGS) $(SERVERFILES) -o $(SERVERBUILD) `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image -lSDL2_ttf -lws2_32 

clientbuild: $(CLIENTFILES)
	gcc $(CFLAGS) $(CLIENTFILES) -o $(CLIENTBUILD) -lws2_32 -lgdi32 -lmswsock

clean: 
	rm build/client.exe build/server.exe client/client.c~ server/server.c~ server/microui.c~ server/microui.h~ server/renderer.c~ server/renderer.h~
