#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<time.h>

#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 20

typedef enum
{
	EMPTY = -1,
	I,
	J,
	L,
	O,
	S,
	T,
	Z
}ShapeId;

typedef enum
{
	RED = 41,
	GREEN,
	YELLOW,
	BLUE,
	PURPLE,
	CYAN,
	WHITE,
	BLACK = 0,
}Color;

typedef struct
{
	ShapeId shape;
	Color color;
	int size;
	char rotates[4][4][4];
}Shape;

typedef struct
{
	Color color;
	ShapeId shape;
	bool current;
}Block;

typedef struct
{
	int x;
	int y;
	int score;
	int rotate;
	int falltime;
	ShapeId queue[4];
}State;

Shape shapes[7] = {
	{
		.shape = I,
		.color = CYAN,
		.size = 4,
		.rotates =
		{
			{
				{0,0,0,0},
				{1,1,1,1},
				{0,0,0,0},
				{0,0,0,0},
			},
			{
				{0,0,1,0},
				{0,0,1,0},
				{0,0,1,0},
				{0,0,1,0},
			},
			{
				{0,0,0,0},
				{0,0,0,0},
				{1,1,1,1},
				{0,0,0,0},
			},
			{
				{0,1,0,0},
				{0,1,0,0},
				{0,1,0,0},
				{0,1,0,0},
			}
		}
	},
	{
		.shape = J,
		.color = BLUE,
		.size = 3,
		.rotates =
		{
			{
				{1,0,0},
				{1,1,1},
				{0,0,0},
			},
			{
				{0,1,1},
				{0,1,0},
				{0,1,0},
			},
			{
				{0,0,0},
				{1,1,1},
				{0,0,1},
			},
			{
				{0,1,0},
				{0,1,0},
				{1,1,0},
			}
		}
	},
	{
		.shape = L,
		.color = YELLOW,
		.size = 3,
		.rotates =
		{
			{
				{0,0,1},
				{1,1,1},
				{0,0,0},
			},
			{
				{0,1,0},
				{0,1,0},
				{0,1,1},
			},
			{
				{0,0,0},
				{1,1,1},
				{1,0,0},
			},
			{
				{1,1,0},
				{0,1,0},
				{0,1,0},
			}
		}
	},
	{
		.shape = O,
		.color = WHITE,
		.size = 2,
		.rotates = {
			{
				{1,1},
				{1,1}
			},
			{
				{1,1},
				{1,1}
			},
			{
				{1,1},
				{1,1}
			},
			{
				{1,1},
				{1,1}
			}
		}
	},
	{
		.shape = S,
		.color = GREEN,
		.size = 3,
		.rotates =
		{
			{
				{0,1,1},
				{1,1,0},
				{0,0,0},
			},
			{
				{0,1,0},
				{0,1,1},
				{0,0,1},
			},
			{
				{0,0,0},
				{0,1,1},
				{1,1,0},
			},
			{
				{1,0,0},
				{1,1,0},
				{0,1,0},
			}
		}
	},
	{
		.shape = T,
		.color = PURPLE,
		.size = 3,
		.rotates =
		{
			{
				{0,1,0},
				{1,1,1},
				{0,0,0},
			},
			{
				{0,1,0},
				{0,1,1},
				{0,1,0},
			},
			{
				{0,0,0},
				{1,1,1},
				{0,1,0},
			},
			{
				{0,1,0},
				{1,1,0},
				{0,1,0},
			}
		}
	},
	{
		.shape = Z,
		.color = RED,
		.size = 3,
		.rotates =
		{
			{
				{1,1,0},
				{0,1,1},
				{0,0,0},
			},
			{
				{0,0,1},
				{0,1,1},
				{0,1,0},
			},
			{
				{0,0,0},
				{1,1,0},
				{0,1,1},
			},
			{
				{0,1,0},
				{1,1,0},
				{1,0,0},
			}
		}
	},
};

void setBlock(Block* block, Color color, ShapeId shape, bool current) {
	block->color = color;
	block->shape = shape;
	block->current = current;
}

void resetBlock(Block* block) {
	block->color = BLACK;
	block->shape = EMPTY;
	block->current = false;
}

void printCanvas(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State *state) {
	printf("\033[0;0H\n");
	for (int i = 0; i < CANVAS_HEIGHT; i++) {
		printf("|");
		for (int j = 0; j < CANVAS_WIDTH; j++) {
			printf("\033[%dm\u3000", canvas[i][j].color);
		}
		printf("\033[0m");
		printf("|\n");
	}
}

bool move(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int originalX, int originalY, int originalRotate, int newX, int newY, int newRotate, ShapeId shapeId) {
	Shape shapeData = shapes[shapeId];
	int size = shapeData.size;

	// 判斷方塊有沒有不符合條件
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (shapeData.rotates[newRotate][i][j]) {
				// 判斷有沒有出去邊界
				if (newX + j < 0 || newX + j >= CANVAS_WIDTH || newY + i < 0 || newY + i >= CANVAS_HEIGHT) {
					return false;
				}
				// 判斷有沒有碰到別的方塊
				if(!canvas[newY + i][newX + j].current && canvas[newY + i][newX + j].shape != EMPTY) {
					return false;
				}	
			}
		}
	}
	
	// 移除方塊舊的位置
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (shapeData.rotates[originalRotate][i][j])
				resetBlock(&canvas[originalY + i][originalX + j]);
		}
	}

	// 移動方塊至新的位置
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (shapeData.rotates[newRotate][i][j])
				setBlock(&canvas[newY + i][newX + j], shapeData.color, shapeId, true);
		}
	}
}

void logic(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state) {
	if (move(canvas, state->x, state->y, state->rotate, state->x, state->y + 1, state->rotate, state->queue[0]))
		state->y++;

	return;
}

int main() {
	srand(time(NULL));

	State state = {
		.x = CANVAS_WIDTH / 2,
		.y = 0,
		.score = 0,
		.rotate = 0,
		.falltime = 0,
	};

	for (int i = 0; i < 4; i++) {
		state.queue[i] = rand() % 7;
	}

	Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
	for (int i = 0; i < CANVAS_HEIGHT; i++) {
		for (int j = 0; j < CANVAS_WIDTH; j++) {
			resetBlock(&canvas[i][j]);
		}
	}

	Shape shapeData = shapes[state.queue[0]];

	for (int i = 0; i < shapeData.size; i++) {
		for (int j = 0; j < shapeData.size; j++) {
			if (shapeData.rotates[state.rotate][i][j] == 1) {
				setBlock(&canvas[state.y + i][state.x + j], shapeData.color, state.queue[0], true);
			}
		}
	}

	while (1) {
		printCanvas(canvas, &state);
		logic(canvas, &state);
		Sleep(100);
	}

	return 0;
}


	return 0;
}

