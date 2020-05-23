#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i;
    int board_x;
    int board_y;

    // Name of file will be an input of main later
    fp = fopen("board.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // Read number of lines and columns of board
    if(fscanf(fp,"%d %d",&board_y,&board_x)!=2)
    {
      printf("Please give a valid input file.");
      exit(EXIT_FAILURE);
    }

    // Close and open file to start reading from beginning again
    fclose(fp);
    fp = fopen("board.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // Create contiguous 2D array to store bricks/board in general
    int *temp = calloc(board_x * board_y, sizeof(int));
    int **board = calloc(board_x, sizeof(int*));
    board[0] = temp;
    for(i = 1; i < board_x; i++)
      board[i] = board[i-1] + board_y;

    // Consume first line, that is correct due to previous fscanf
    getline(&line, &len, fp);

    // Read file line by line and store location of brick in board[][]
    i = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
        // If line doesn't have board_y columns
        if(read!=(board_y+1))
        {
          printf("Please give a valid input file.\n");
          exit(EXIT_FAILURE);
        }
        // For a given line, iterate all the columns
        for(int j = 0; j < board_y; j++)
        {
          // If there is a brick, store in board
          if(line[j] == 'B')
            board[i][j] = 1;
        }
        // i is the index of current line
        i++;
    }

    // If file doesnt have board_x lines
    if(i!=(board_x))
    {
      printf("Please give a valid input file.\n");
      exit(EXIT_FAILURE);
    }

    // Close file, free line reader
    fclose(fp);
    if (line)
        free(line);

    for (i=0; i<board_x; i++)
    {
        for(int j=0; j<board_y; j++)
        {
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }

    // Free contiguous board
    free(board[0]);
    free(board);
    exit(EXIT_SUCCESS);
}
