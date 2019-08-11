#include <ncurses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#define DEF_SIZE 5

typedef struct ebuff{
	char *current;		// position of cursor in the buffer		
	char *buffer;		// start of buffer [array]
	char *bufend;		// end of the array
	char *gapstart;		// position of start of gap in buffer
	char *gapend;		// end of gap
	char *line;			// for copying a line
	int GAP_SIZE;
}ebuff;

struct ebuff gbuf;

int initBuffer(int size);
int initGapBuffer(FILE *file);
char *initLine();
void initEmptyGapBuffer();

void movePointToBegin();

void moveGapLeft();
void moveGapRight();
int moveGapDown(int x);
void moveGapUp(int x, int y);			
void expandGap(int size);
void moveGapToCurr();

int tranferGap(char *temp1, char *temp2, int len);
void expandBuffer(int size);
int bufferSize();
int sizeOfGap();    

char nextKey();
char* copyLine();
void writeKey(char ch);
void deleteKey();

int saveBuff(FILE *file, int byte);
void writeString(char *string, int len);
void writeBuffer();

