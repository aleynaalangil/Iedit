# Iedit
Image editor in C using gfx graphics tool. 
The editor performs the following functions:
- E (edit): Opens the specified bitmap file and a window on the screen. If the file exists, its contents are drawn on the window.
- F (filter): Applies the following operations on the image:
    * cp - copy/paste
    * bl - blur
    * gr - grayscale
    * br - brighten
- U (undo): Discards the last operation.
- R (redo): Repeats the last operation.
- V (view): Views the image on the window.
- S (save): Saves the file
- X (exit)

The editor keeps the original image in a memory buffer and edit it in an editing buffer to display. Using the following data structures:
```c
struct pixel {
int red;
int green;
int blue;
}
struct pixel orig[128][128]; // original image (max. image size 128x128)
struct pixel eb[128][128]; // editing buffer
```
For undo/redo, you need to keep all operations performed on the image. Use the following stack structure:
 ```c
struct operation {
int op; // 1-copy/paste, 2-blur, 3-grayscale, 4-brighten
int x; // destination for copy/paste
int y;
}
struct operation stack[100]; // max. 100 operations
struct operation *sptr; // stack pointer
struct operation *max_sptr; // maximum stack pointer for redo
```
Editor commands:
```shell
F bl → pushes blur operation on the stack
F gr → pushes grayscale operation on the stack
U → updates stack pointer
V → applies operations on the stack to the image and displays
R → updates stack pointer
V → applies operations on the stack to the image and displays
R → gives an error
F cp 50 50 → pushes copy operation on the stack (copy (0,0)-(20,20) to (50,50)-(70,70))
S → saves the changes
X → quit
```
To compile:
```shell
gcc main.c gfx.c -o Iedit -lX11 -lm
```
To run:
```shell
./Iedit file.bmp
```
