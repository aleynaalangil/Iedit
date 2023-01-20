#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfx.h"

// gcc main.c gfx.c -o Iedit -lX11 -lm
#define WIDTH 128
#define HEIGHT 128
#define MAX_OPS 100
int counter=0;
struct pixel
{
    int red;
    int green;
    int blue;
};

struct pixel orig[WIDTH][HEIGHT]; // original image
struct pixel eb[WIDTH][HEIGHT];   // editing buffer

struct operation
{
    int op; // 1-copy/paste, 2-blur, 3-grayscale, 4-brighten
    int x;  // destination for copy/paste
    int y;
};

struct operation stack[MAX_OPS];   // max. 100 operations
struct operation *sptr = stack;     // stack pointer
struct operation *max_sptr = stack; // maximum stack pointer for redo

void edit(char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("Error: file not found.\n");
        exit(1);
    }

	fseek(fp, 54, SEEK_SET); 

	for (int i = HEIGHT-1; i >= 0; i--)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            fread(&orig[j][i].blue, 1, 1, fp);
			fread(&orig[j][i].green, 1, 1, fp);
			fread(&orig[j][i].red, 1, 1, fp);
        }
    }
	fclose(fp);
	memcpy(eb, orig, sizeof(struct pixel) * WIDTH * HEIGHT);
    gfx_open(WIDTH, HEIGHT, "Iedit");
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            gfx_color(eb[i][j].red, eb[i][j].green, eb[i][j].blue);
            gfx_point(i, j);
        }
    }
    gfx_flush();
}

void filter(int op, int x, int y)
{
    sptr->op = op;
    sptr->x = x;
    sptr->y = y;
    sptr++;
    if (sptr > max_sptr)
    {
        max_sptr = sptr;
    }
}
void copy_paste(int x, int y)
{
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            eb[x + i][y + j] = orig[i][j];
        }
    }
}
void blur()
{
    struct pixel temp[WIDTH][HEIGHT];

    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            temp[i][j].red = eb[i][j].red;
			temp[i][j].green = eb[i][j].green;
			temp[i][j].blue = eb[i][j].blue;
			int pixelCount = 1;
			if(i > 0){ 
				temp[i][j].red += eb[i - 1][j].red;
				temp[i][j].green += eb[i - 1][j].green;
				temp[i][j].blue += eb[i - 1][j].blue;
				pixelCount++;
			}
			if(i < (WIDTH - 1)){ 
				temp[i][j].red += eb[i + 1][j].red;
				temp[i][j].green += eb[i + 1][j].green;
				temp[i][j].blue += eb[i + 1][j].blue;
				pixelCount++;
			}
			if(j > 0){ 
				temp[i][j].red += eb[i][j - 1].red;
				temp[i][j].green += eb[i][j - 1].green;
				temp[i][j].blue += eb[i][j - 1].blue;
				pixelCount++;
			}
			if(j < (HEIGHT - 1)){ 
				temp[i][j].red += eb[i][j + 1].red;
				temp[i][j].green += eb[i][j + 1].green;
				temp[i][j].blue += eb[i][j + 1].blue;
				pixelCount++;
			}
			eb[i][j].red = temp[i][j].red / pixelCount;
			eb[i][j].green = temp[i][j].green / pixelCount;
			eb[i][j].blue = temp[i][j].blue / pixelCount;
        }
    }
}
void undo()
{
    if (sptr == stack)
    {
        printf("Error: no more operations to undo.\n");
        return;
    }

    sptr--;
}
void redo()
{
    if (sptr == max_sptr)
    {
        printf("Error: no more operations to redo.\n");
        return;
    }

    sptr++;
}
void grayscale()
{
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            int gray = (eb[i][j].red + eb[i][j].green + eb[i][j].blue) / 3;
            eb[i][j].red = gray;
            eb[i][j].green = gray;
            eb[i][j].blue = gray;
        }
    }
}
void brighten()
{
	int brightness=125;
	for(int i=0; i< WIDTH; i++)
	{
		for(int j=0; j< HEIGHT; j++)
		{
			eb[i][j].red=eb[i][j].red+brightness;
			if(eb[i][j].red>255) {eb[i][j].red=255;}
			if(eb[i][j].red<0) {eb[i][j].red=0;}
			
			eb[i][j].green=eb[i][j].green+brightness;
			if(eb[i][j].green>255) {eb[i][j].green=255;}
			if(eb[i][j].green<0) {eb[i][j].green=0;}
			
			eb[i][j].blue=eb[i][j].blue+brightness;
			if(eb[i][j].blue>255) {eb[i][j].blue=255;}
			if(eb[i][j].blue<0) {eb[i][j].blue=0;}
		}
	}
}
void view()
{
	memcpy(eb, orig, sizeof(struct pixel) * WIDTH * HEIGHT);
    struct operation *op = stack;
    while (op < sptr)
    {
        switch (op->op)
        {
        case 1:
            copy_paste(op->x, op->y);
            break;
        case 2:
            blur();
            break;
        case 3:
            grayscale();
            break;
        case 4:
            brighten();
            break;
        }
        op++;
    }
    gfx_open(WIDTH, HEIGHT, "Iedit");
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            gfx_color(eb[i][j].red, eb[i][j].green, eb[i][j].blue);
            gfx_point(i, j);
        }
    }
    gfx_flush();
}
void save(char *filename)
{
    for (struct operation *op = stack; op < sptr; op++)
    {
        switch (op->op)
        {
        case 1:
            copy_paste(op->x, op->y);
            break;
        case 2:
            blur();
            break;
        case 3:
            grayscale();
            break;
        case 4:
            brighten();
            break;
        }
        op++;
    }

	while(sptr!=stack)
    {
		sptr->x=0;
		sptr->y=0;
		sptr->op=0;
        sptr--;
    }
	
	FILE *fp = fopen(filename, "r+");
	fseek(fp, 54, SEEK_SET);
	for (int i = HEIGHT-1; i >= 0; i--)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            fwrite(&eb[j][i].blue, 1, 1, fp);
			fwrite(&eb[j][i].green, 1, 1, fp);
			fwrite(&eb[j][i].red, 1, 1, fp);
        }
    }
	fclose(fp);
    counter=0;
}
int main(int argc, char *argv[])
{
	char command[20];
	edit(argv[1]);
	while(1)
	{
		fgets(command,20,stdin);
		if(command[0]=='X') 
		{
			break;
		}
		
		switch(command[0])
		{
		case 'F':
			if(command[2]=='b' && command[3]=='r')
			{
				if(counter<MAX_OPS) {filter(4,0,0);}
				else if(counter >=MAX_OPS ) {save(argv[1]);}
				counter++;
			}
			else if(command[2]=='b' && command[3]=='l')
			{
				if(counter<MAX_OPS) {filter(2,0,0);}
				else if(counter >=MAX_OPS ) {save(argv[1]);}
				counter++;
			}
			else if(command[2]=='g' && command[3]=='r')
			{
				if(counter<MAX_OPS) {filter(3,0,0);}
				else if(counter >=MAX_OPS ) {save(argv[1]);}
				counter++;
			}
			else if(command[2]=='c' && command[3]=='p')
			{	
				int i=5;
				int xp, yp;
				xp=(command[5]-'0')*10+(command[6]-'0');
				yp=(command[8]-'0')*10+(command[9]-'0');
				if(counter<MAX_OPS) {filter(1,xp,yp);}
				else if(counter >=MAX_OPS ) {save(argv[1]);}
				counter++;
			}
			break;
        case 'U':
            undo();
            break;
        case 'R':
            redo();
            break;
        case 'V':
            view();
            break;
		case 'S':
			save(argv[1]);
			break;
	}
	}
	
	
	if(command[0]=='X')
	{
		exit(1);
	}
	
	
	return 0;
}