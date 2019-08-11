#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "buff.h"
#include "key_values.h"

char *initLine(){
	gbuf.line = (char *)malloc(COLS);
	return gbuf.line;
}

char *copyLine(){
	char *temp;
	temp = gbuf.current;
	while(1){
		if(*temp == '\n' || temp == gbuf.buffer)
			break;
		temp--;
	}
	temp++;
	if(temp >= gbuf.gapstart && temp < gbuf.gapend)
		temp = gbuf.gapend;
	int i = 0;
	char *cpLine = initLine();
	//cpLine = (char*)malloc(COLS);
	//char line[COLS];
	while(1){
		if(*temp == '\n' || temp == gbuf.bufend)
			break;
		cpLine[i] = *temp;
		temp++;
		if(temp >= gbuf.gapstart && temp < gbuf.gapend)
			temp = gbuf.gapend;
		i++;
	}
	return cpLine;
	
}

int initBuffer(int size){
	gbuf.buffer = (char *)malloc(size);
	if(gbuf.buffer == NULL){
        	return 0;
    	}
    	gbuf.current = gbuf.buffer;
    	gbuf.gapstart = gbuf.buffer;

    	gbuf.gapend = gbuf.buffer + size;     
    	gbuf.bufend = gbuf.gapend;
    	return 1;
}

void moveGapToEnd(){
	char *temp;
	temp = gbuf.current;
	gbuf.current = gbuf.bufend;
	moveGapToCurr();
	gbuf.current = temp;	
}


void movePointToBegin(){
	gbuf.current = gbuf.buffer;
}

int transferGap(char *temp1, char *temp2, int length){
    //printw("inside coppybytes")
    if((temp1 == temp2) || (length == 0))
        return 1;
    if(temp2 > temp1){
        if((temp2 + length) >= gbuf.bufend)
            return 0;
        while(length > 0){ 
	//printw("inside loop 1");
            *(temp1++) = *(temp2++);
        	length--;
        }
    }//printw("HI");
    else{
        temp2 = temp2 + length;
        temp1 = temp1 + length;
        while(length > 0){
        	*(--temp1) = *(--temp2); 
        	length--;
		//printw("inside loop 2");        
        }
    }
    return 1;
}

void emptyGap(){
	
}
void moveGapLeft(){
    	--gbuf.current;
        moveGapToCurr();
}

void adjustGap(){
	if(gbuf.current >= gbuf.gapstart && gbuf.current < gbuf.gapend){
        	gbuf.current = gbuf.gapend;
       	}
       	moveGapToCurr();
}
void moveGapUp(int row, int col){
	if(row){
		char *temp, *prev;
		temp = gbuf.current;
		temp = temp - col;
		temp--;
		prev = temp;
		int count = 0;

		while(1){
			temp--;
			if(*temp == '\n' || temp == gbuf.buffer)
				break;
			count++;
			
		}
		
		if(count < col){
			move(row - 1, count);
			gbuf.current = prev;
		}
		else{
			move(row - 1, col);
			temp++;
			temp = temp + col;
			gbuf.current = temp;
		}
		refresh();
		moveGapToCurr();
	}
}

void moveGapRight(){
        ++gbuf.current;
       	moveGapToCurr();
}

int moveGapDown(int x){
	char *temp;
	int flag = 0;
	temp = gbuf.current;
	if(temp == gbuf.gapstart)
			temp = gbuf.gapend;
	while(1){
		if(*temp == '\n')
			break;
		if(temp == gbuf.bufend){
			flag = -1;
			return flag;
		}
		temp++;
	}
	temp++;
	while(1){
		if(*temp == '\n' || temp == gbuf.bufend)
			break;
		flag++;	
		temp++;
	}
	if(x <= flag){
		char *count;
		count = gbuf.current; 
		if(count == gbuf.gapstart)
			count = gbuf.gapend;
		while(1){
			if(*count == '\n')
				break;
			count++;
		}
		gbuf.current = count + x;
		moveGapToCurr();
	}else{
		gbuf.current = temp;
		moveGapToCurr();
	}
	return flag;
}

int saveBuff(FILE *file, int bytes){
    if(!bytes)
        return 1;
    if(gbuf.current == gbuf.gapstart)
        gbuf.current = gbuf.gapend;
    if((gbuf.gapstart > gbuf.current) && (gbuf.gapstart < (gbuf.current + bytes)) && (gbuf.gapstart != gbuf.gapend)){
        if(gbuf.gapstart - gbuf.current != fwrite(gbuf.current, 1, gbuf.gapstart-gbuf.current, file)){
            return 0;
        }
        if((bytes - (gbuf.gapstart - gbuf.current)) != fwrite(gbuf.gapend, 1, bytes-(gbuf.gapstart - gbuf.current), file)){
            return 1;
        }
        return 1;
    }else {
        return bytes == fwrite(gbuf.current, 1, bytes, file);
    }
}

void writeString(char *string, int len){
	moveGapToCurr();
	int s = sizeOfGap();
	if(len > s){
        	expandGap(len);
    	}
    	do{	//printw("1");
		writeKey(*(string++));
	  }while(len--);         
}

void writeBuffer(){
	char *temp = gbuf.buffer;
	while(temp < gbuf.bufend){
        	if((temp >= gbuf.gapstart) && (temp < gbuf.gapend)){
            		temp++;
        	}else
           		printw("%c", *(temp++));
	refresh();
    }
}

int sizeOfGap(){
	return gbuf.gapend - gbuf.gapstart;
}

int bufferSize(){
	return (gbuf.bufend - gbuf.buffer) - (gbuf.gapend - gbuf.gapstart);
}


void deleteKey(){
	--gbuf.gapstart;
	--gbuf.current;
}

void initEmptyGapBuffer(){
	initBuffer(DEF_SIZE);
	
}

int initGapBuffer(FILE *file){
	struct stat buf;
	fstat(fileno(file), &buf);
	long fsize = buf.st_size;
	
	initBuffer(fsize + (DEF_SIZE));    
	moveGapToCurr();
	expandGap((int)fsize);

	int total;
	total = fread(gbuf.gapstart, 1, fsize, file);
	gbuf.gapstart += total;
	return 0;
}


void expandGap(int size){
	if(size > sizeOfGap()){
        size += gbuf.GAP_SIZE;	
        expandBuffer(size);
       	transferGap(gbuf.gapend+size, gbuf.gapend, gbuf.bufend - gbuf.gapend);
		gbuf.gapend += size;
       	gbuf.bufend += size;
    }
}

void expandBuffer(int size){   
    if(((gbuf.bufend - gbuf.buffer) + size) > bufferSize()){
        char *origbuffer = gbuf.buffer;
      
        int NewBufferSize = (gbuf.bufend - gbuf.buffer) + size  + gbuf.GAP_SIZE;
        gbuf.buffer = (char *) realloc(gbuf.buffer, NewBufferSize);

        gbuf.current += gbuf.buffer - origbuffer;
        gbuf.bufend += gbuf.buffer - origbuffer;
      
        gbuf.gapstart += gbuf.buffer - origbuffer;
        gbuf.gapend += gbuf.buffer - origbuffer;
    }

}

void moveGapToCurr(){
	//printw("inside MoveGap");
	if(gbuf.current == gbuf.gapstart)
	        return;
	if(gbuf.current == gbuf.gapend){
        	gbuf.current = gbuf.gapstart;
        	return;
    	}
	if(gbuf.current < gbuf.gapstart){
	//printw("point is behind the gap");
        transferGap(gbuf.current + (gbuf.gapend-gbuf.gapstart), gbuf.current, gbuf.gapstart - gbuf.current);
        gbuf.gapend -= (gbuf.gapstart - gbuf.current);
        gbuf.gapstart =gbuf.current;
   	}
   	else{
   	//printw("point is after the gap");
        transferGap(gbuf.gapstart, gbuf.gapend, gbuf.current - gbuf.gapend);
        gbuf.gapstart += gbuf.current - gbuf.gapend;
        gbuf.gapend = gbuf.current;
        gbuf.current = gbuf.gapstart;
    }
}

void writeKey(char ch){
    if(gbuf.current != gbuf.gapstart)
		moveGapToCurr();
	if(gbuf.gapstart == gbuf.gapend)
        	expandGap(1);
	*(gbuf.gapstart++) = ch;
	*gbuf.current++;
}

char nextKey(){   
        if(gbuf.current == gbuf.gapstart){
        gbuf.current = gbuf.gapend;
        return *gbuf.current;
    }
    int ch = *(gbuf.current + 1);
    return ch;
}

