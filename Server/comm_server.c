#include "structs.h"
#include "comm.h"
#include "list_handler.h"

int send_board(int x, int y, player *new_player){
    char message[50];
    int err;

    err = sprintf(message, "b %d %d\n", x, y);
	if(err == -1){
		printf("error: ");
		return -1;
	}
	printf("message: %s\n", message);

   	err = write(new_player->sock_fd, message, strlen(message)); 
	if(err == -1){
		perror("write: ");
		return -1;
	}

    return 0;
}

int rcv_color(int sock_fd){
    int err;
	bool unique_color = true;
	struct player *list;
	struct color *new_color = malloc(sizeof(struct color));


	// falta usar mutex para impedir que várias threads corram ao mesmo tempo

    err = recv(sock_fd, new_color , sizeof(*new_color), 0);
	if (err == -1){
		perror("receive: ");
		exit(EXIT_FAILURE);
	} 

	list = getList();
	while(list->next != NULL && unique_color){
		if(list->p_color->r != new_color->r && list->p_color->g != new_color->g && 
			list->p_color->b != new_color->b) unique_color = false;
	}

	if(!unique_color) return -1;

	printf("r: %d g: %d b: %d\n", new_color->r, new_color->g, new_color->b);

	return 0;
}
