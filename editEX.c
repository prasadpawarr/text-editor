#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "buff.h"
#include "key_values.h"

#define tab_size 4

void displayStatusbar(){
	attron(A_REVERSE);
	mvaddstr(LINES - 3, 0, "^R REPLACE     ^F FIND 		^X CUT		^V PASTE	  ^C COPY");
	mvaddstr(LINES - 2, 0, "^W QUIT        ^U SAVE		^H HELP		^H HELP/RULES");	
	attroff(A_REVERSE);
	move(0,0);
	refresh();
}

void clearLine(int x, int y){
	move(x, y);
	clrtoeol();
	refresh();
}

void startCursor(){
	move(0,0);
	refresh();
}

void createWin(){
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	displayStatusbar();
}	

void moveRightInsert(int line_x, int column_y){
	if(column_y == COLS-1 ){
		++line_x;
		column_y = 0;
		move(line_x, column_y);
	}
	else{	
		++column_y;					
		move(line_x, column_y);
	}
	refresh();
}

void moveRight(int line_x, int column_y){
	if(nextKey() == '\n'){
		++line_x;
		column_y = 0;
		move(line_x, column_y);
	}else	
		if(column_y == COLS-1 ){
			++line_x;
			column_y = 0;
			move(line_x, column_y);
		}
		else{	
			++column_y;					
			move(line_x, column_y);
		}
	refresh();
}

void moveLeft(int line_x, int column_y){
	if(column_y == 0 && line_x != 0){
		move(line_x, column_y);
	}else{
		--column_y;
		move(line_x, column_y);
	}
	refresh();
}

void moveDown(int line_x, int column_y, int flag){
	if(flag <= column_y){
		line_x++;
		move(line_x, flag);
	}
	else{
		if(line_x < (LINES - 5)){
		line_x++;
		move(line_x, column_y);
		}	
	}
		refresh();
}

void newline(int line_x){
	if(line_x < (LINES - 5)){
		line_x++;
		move(line_x, 0);
	}	
	refresh();
}	
void moveRightOffset(int line_x, int column_y, int offset){	
	if(nextKey() == '\n'){
		++line_x;
		column_y = 0;
		move(line_x, column_y);
	}else
		if(column_y == COLS-1 ){
			++line_x;
			column_y = 0;
			move(line_x, column_y);
		}else{	
			column_y = column_y + offset;					
			move(line_x, column_y);
			}
	refresh();	
}

int findWord(char *string){
	char *temp, *ptr, *p;
	int count = 0;
	int val;
	int x1, y1;
	getyx(stdscr, x1, y1);
	if(gbuf.current == gbuf.gapstart)
		gbuf.current = gbuf.gapend;

	temp = gbuf.current;
	ptr = strstr(temp, string);
	if(ptr){
		val = 2;
		p = gbuf.current;
		while(p != ptr){
			p++;
			count++;
			gbuf.current = p;
			moveRight(x1, y1);
			getyx(stdscr, x1, y1);
			}
		if(x1 != 0)
			y1 = y1 - 2;
		move(x1, y1);
		gbuf.current = ptr;
		moveGapToCurr();
	}
	else{
		val = 1;
		move(LINES - 4, 0);
		clrtoeol();
		noecho();
		mvprintw(LINES - 4, 0, "Word (%s) not found", string);
	}
	return val;
}
	
int replaceWord(char *replacewith, char *toreplace, int x, int y){
	int len1 = strlen(replacewith);
	int len2 = strlen(toreplace);
	int l = len2;
	gbuf.current = gbuf.gapend;
	while(len2){
		gbuf.current++;
		len2--;
	}
	moveGapToCurr();
	while(l){
		deleteKey();
		l--;
	}
	writeString(replacewith, len1);
	refresh();
	return 0;
}

int linesInFile(FILE *fp){			//calculates no of lines in the file
	char ch = '\0';
	int count = 0;
	while((ch = fgetc(fp)) != EOF)
		if(ch == '\n')
			count++;
    	fseek(fp, 0, SEEK_SET); 
	return count;
}
	
void cutLine(){
	char *temp;
	if(gbuf.current == gbuf.gapstart)
		gbuf.current = gbuf.gapend;
	temp = gbuf.current;
	while(1){
		if(*temp == '\n' || temp == gbuf.bufend)
			break;
		temp++;
	}
	gbuf.current = temp;
	moveGapToCurr();
	while(1){
		if(*gbuf.current == '\n' || gbuf.current == gbuf.buffer)
			break;
		deleteKey();
	}
	moveGapToCurr();
}

int createSaveFile(char *filename){			//creates a file in write mode for saving.
	FILE *fp;
	fp = fopen(filename, "w");
	if(fp == NULL)
		return 0;
	saveBuff(fp, bufferSize());
	fclose(fp);
	return 1;
}

void printHelpOption(){
print: 	
	move(0, 0);
	clrtobot();
	displayStatusbar();
	move(1, (COLS - 40)/2);
	printw("HELP MENU");
	printw("\n BASIC FUNCTIONS :\n");
	printw("\n CTRL-R searches a given word and replaces it.");
	printw("\n CTRL-F searches for the given word from the current cursor posiiton.");
	printw("\n CTRL-C copies the current line.");
	printw("\n CTRL-V pastes the line copied.");
	printw("\n CTRL-X cuts the current line.");
	printw("\n CTRL-W closes the editor without saving the file.");
	printw("\n CTRL-U saves the file with given filename.");
	move(LINES - 4, 0);
	clrtoeol();
	curs_set(0);
	mvaddstr(LINES - 4, 0, "Press 'q' to quit help menu");
	char ch = getch();
	if(ch == 'q' || ch == 'Q'){
		move(0, 0);
		clrtobot();		
		writeBuffer();
		displayStatusbar();
	}else
		goto print;
	curs_set(1);
}

void key_strokes(int scr_x, int scr_y){					//Handles the key input strokes.
	int key = 1;
	char savefile[32];
	char flag;
	char word[32], to_replace[32], replace_with[32];
	int d; // for down
	char *temp;
	char *string;
	int len;
	while(1){
		key = getch();
		switch(key){
			case BACKSPACE:
				getyx(stdscr, scr_x, scr_y);
				if(scr_y == 0 && scr_x == 0 ){ 
					//nothing
					}
				
				if(scr_y != 0){ 
					deleteKey();
	 				move(0,0);
					writeBuffer();
					displayStatusbar();
					moveLeft(scr_x, scr_y);
					}				
				break;
				
			case TAB:
				noecho();
				for(int i = 0; i < tab_size; i++)
					writeKey(' ');
				getyx(stdscr, scr_x, scr_y);
				move(0,0);	
				writeBuffer();
				displayStatusbar();
				moveRightOffset(scr_x, scr_y, tab_size);
				break;
			
			case ENTER:
				noecho();
				writeKey('\n');
				getyx(stdscr, scr_x, scr_y);
				move(0,0);	
				writeBuffer();
				displayStatusbar();
				newline(scr_x);
				break;
			
			case CTRLH:							//Printing a help menu
				getyx(stdscr, scr_x, scr_y);
				printHelpOption();
				move(scr_x, scr_y);
				break;
			
			case CTRLC:						//Copying a line.
				noecho();
				string = copyLine();
				len = strlen(string);
				break;
			
			case CTRLV:						//Pasting a line
				noecho();
				writeString(string, len);
				getyx(stdscr, scr_x, scr_y);
				move(0, 0);
				writeBuffer();
				displayStatusbar();
				moveRightOffset(scr_x, scr_y, len);
				moveGapToCurr();
				break;
			
			case CTRLX:							//Cut a line
				noecho();
				getyx(stdscr, scr_x, scr_y);
				noecho();
				string = copyLine();
				len = strlen(string);
				cutLine();
				clear();
				displayStatusbar();	
				move(0, 0);
				writeBuffer();
				displayStatusbar();
				refresh();
				d = moveGapDown(scr_x);
				move(scr_x, 0);
				break;

			case CTRLF:								//Finding a word from cursor position.
				getyx(stdscr, scr_x, scr_y);
				move(LINES - 4, 0);
				mvaddstr(LINES - 4, 0, "Enter the word to search : ");
				echo();
				mvgetstr(LINES - 4, 27, word);
				move(scr_x, scr_y);
				noecho();
				int val = findWord(word);
				if(val == 1)
					move(scr_x, scr_y);
				break;
			
			case CTRLR:								//Replacing a word
				getyx(stdscr, scr_x, scr_y);
				move(LINES - 4, 0);
				clrtoeol();
				move(LINES - 4, 0);
				clrtoeol();
				echo();
				mvaddstr(LINES - 4, 0, "Search (to replace): ");
				mvgetstr(LINES - 4, 21, to_replace);
				move(scr_x, scr_y);
				val = findWord(to_replace);
				if(val == 1){
					move(scr_x, scr_y);
					break;
				}
				move(LINES - 4, 0);
				clrtoeol();
				mvaddstr(LINES - 4, 0, "Replace with: ");
				mvgetstr(LINES - 4, 14, replace_with);
				noecho();
				move(scr_x, scr_y);
				replaceWord(replace_with, to_replace, scr_x, scr_y);
				move(0, 0);
				writeBuffer();
				displayStatusbar();
				refresh();
				gbuf.current = gbuf.buffer;
				moveGapToCurr();
				move(0, 0);
				break;
			
			case CTRLU:							//Saving the file.
				temp = gbuf.current;
				gbuf.current = gbuf.buffer;
				moveGapToCurr();
				getyx(stdscr, scr_x, scr_y);
				move(LINES - 4, 0);
				clrtoeol();
				mvaddstr(LINES - 4, 0, "Do you want to save the file ? (Y/N): ");
				echo();
				flag = mvgetch(LINES - 4, 37);
				
				if(flag == 'Y' || flag == 'y'){
					move(LINES - 4, 0);
					clrtoeol();
					mvaddstr(LINES - 4, 0, "Enter the name of file: ");
					mvgetstr(LINES - 4, 24, savefile);
					if(createSaveFile(savefile)){
						move(LINES - 4, 0);
						clrtoeol();
						mvaddstr(LINES - 4, 0, "File saved successfully");
					}else{
						move(LINES - 4, 0);
						clrtoeol();
						mvaddstr(LINES - 4, 0, "File couldn't be created");	
					}
				}else{
					move(LINES - 4, 0);
					clrtoeol();
				}
				move(scr_x, scr_y);
				noecho();
				gbuf.current = temp;
				break;	


			case CTRLW:								//Closing the window.
				endwin();
				exit(1);
				break;
	
			case SPACE:
				writeKey(' ');
				getyx(stdscr, scr_x, scr_y);
				move(0,0);
				writeBuffer();
				displayStatusbar();
				moveRightInsert(scr_x, scr_y);
				break;
			
			case LEFT:
				getyx(stdscr, scr_x, scr_y);
				moveLeft(scr_x, scr_y);
				moveGapLeft();
				moveGapToCurr();
				break;		
		
			case RIGHT:
				getyx(stdscr, scr_x, scr_y);
				moveRight(scr_x, scr_y);
				moveGapRight();
				break;
		
			case UP:
				getyx(stdscr, scr_x, scr_y);
				moveGapUp(scr_x, scr_y);
				break;
	
			case DOWN:
				getyx(stdscr, scr_x, scr_y);
				d = moveGapDown(scr_y + 1);
				getyx(stdscr, scr_x, scr_y);
				if(d)
					moveDown(scr_x, scr_y, d);
				moveGapToCurr();
				break;
			
			default:											//For any printable character.	
				if(isprint(key)){
					getyx(stdscr, scr_x, scr_y);
					move(LINES - 4, 0);
					clrtoeol();
					move(scr_x, scr_y);
					moveGapToCurr();
					noecho();
					writeKey(key);
					getyx(stdscr, scr_x, scr_y);
					move(0,0);	
					writeBuffer();
					displayStatusbar();
					moveRightInsert(scr_x, scr_y);
					moveGapToCurr();
					
				}
					
			}
		
		}

	}

int open_file(char *filename){			//opens the file and initializes the buffer
	FILE *fp;
	fp = fopen(filename, "r+");
	if(fp == NULL) {
		return 1;
		}
	initGapBuffer(fp);
	fclose(fp);
	return 0;
}

int main(int argc, char *argv[]){
	if(argc > 1){
		int val = open_file(argv[1]);
		if(val){
			printf("Could not open the file\n");
			exit(1);
		}
	}else
		initEmptyGapBuffer();
	
	createWin(); 				
	displayStatusbar();	
	writeBuffer();	
	displayStatusbar();	
	move(0, 0);
	int curr_x, curr_y;			//screen cursors
	getyx(stdscr, curr_x, curr_y);			
	key_strokes(curr_x, curr_y);	
	refresh();
	return 0;
}
