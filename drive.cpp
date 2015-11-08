#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

int main()
{
if (getch() == '\033') { // if the first value is esc
    getch(); // skip the [
    switch(getch()) { // the real value
        case 'A'
	    printf("Up");
            break;
        case 'B':
	    printf("Down");
            break;
        case 'C':
            printf("Right");
            break;
        case 'D'
            printf("Left");
            break;
    }
}
}


