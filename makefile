program_LIBRARIES := SDL2 SDL2_image pthread m
LDLIBS += $(foreach library,$(program_LIBRARIES),-l$(library))

program_FLAGS := Wall O3 pedantic
LFLAGS += $(foreach flag,$(program_FLAGS),-$(flag))

all: client server

server: server.o UI_library.o list_handler.o comm_server.o game_rules.o
	gcc -o server server.o UI_library.o list_handler.o comm_server.o game_rules.o $(LFLAGS) $(LDLIBS)

client: client.o UI_library.o comm_client.o
	gcc -o client client.o UI_library.o comm_client.o $(LFLAGS) $(LDLIBS)

client.o: Client/client.c Client/client.h Client/UI_library.h Client/structs.h Client/comm.h
	gcc -c Client/client.c $(LFLAGS) $(LDLIBS)

server.o: Server/server.c  Server/structs.h Server/server.h Server/list_handler.h Server/UI_library.h Server/comm.h Server/game_rules.h
	gcc -c Server/server.c $(LFLAGS) $(LDLIBS)

list_handler.o: Server/list_handler.c Server/list_handler.h
	gcc -c Server/list_handler.c $(LFLAGS) $(LDLIBS)

comm_client.o: Client/comm_client.c Client/comm.h Client/structs.h  Client/client.h
	gcc -c Client/comm_client.c $(LFLAGS) $(LDLIBS)

comm_server.o: Server/comm_server.c Server/comm.h Server/structs.h  Server/server.h Server/list_handler.h Server/game_rules.h
	gcc -c Server/comm_server.c $(LFLAGS) $(LDLIBS)

game_rules.o: Server/game_rules.c Server/structs.h Server/list_handler.h Server/UI_library.h Server/comm.h Server/game_rules.h
	gcc -c Server/game_rules.c $(LFLAGS) $(LDLIBS)

UI_library.o: UI_library/UI_library.c UI_library/UI_library.h
	gcc -c UI_library/UI_library.c $(LFLAGS) $(LDLIBS)

clean:
	rm -f *.o *.~ server *.~ client *.~list_handler *.~comm_client *.~comm_server *.~game_rules *.~UI_library