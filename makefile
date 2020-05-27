program_LIBRARIES := SDL2 SDL2_image pthread m
LDLIBS += $(foreach library,$(program_LIBRARIES),-l$(library))

program_FLAGS := Wall O3 pedantic
LFLAGS += $(foreach flag,$(program_FLAGS),-$(flag))

all: client server

server: server.o UI_library.o
	gcc -o server server.o UI_library.o $(LFLAGS) $(LDLIBS)

client: client.o UI_library.o
	gcc -o client client.o UI_library.o $(LFLAGS) $(LDLIBS)

client.o: Client/client.c Client/client.h Client/UI_library.h
	gcc -c Client/client.c $(LFLAGS) $(LDLIBS)

server.o: Server/server.c Server/structs.h Server/server.h Server/UI_library.h
	gcc -c Server/server.c $(LFLAGS) $(LDLIBS)

UI_library.o: UI_library/UI_library.c UI_library/UI_library.h
	gcc -c UI_library/UI_library.c $(LFLAGS) $(LDLIBS)

clean:
	rm -f *.o *.~ server *.~ client