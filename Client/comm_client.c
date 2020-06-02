#include "structs.h"
#include "comm.h"

int rcv_board(char *message, int *board_x, int *board_y){
    char type;
    int x, y;

    printf("message: %s\n", message);
    if(sscanf(message, "%c %d %d", &type, &x, &y) == 3){
        if(type == 'b'){
			*board_x = x;
			*board_y = y;
			return 0;
		} 
        else{
            printf("error: incorrect initial message");
            return -1;
        }
    }
    else{
        printf("error: incorrect initial message");
        return -1;
    }
}

int send_color(int server_fd, struct color *new_color){
    int err;
        
	err = write(server_fd, new_color, sizeof(*new_color)); 
	if(err == -1){
		perror("write: ");
		return -1;
	}

    return 0;
}
