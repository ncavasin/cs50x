#include <stdio.h>
#include <cs50.h>
#include <string.h>


int get_height(void);
void draw(int);



int main(void)
{
    draw(get_height());
}





// Ask and validate input
int get_height(void)
{
    int n;

    do
    {
        n = get_int("Integer between 1 and 8: ");
    }
    while (n < 1 || n > 8);
    return n;
}

// Draw the pyramid using h as height
void draw(int h)
{

    // This for counts rows
    for (int i = 0; i < h; i++)
    {
        // This prints left-side spaces according to the row number
        for (int j = i; (h - (j + 1)  != 0); j++)
        {
            printf(" ");
        }

        // This prints the left #s that follow the spaces
        for (int k = 0; k <= i; k++)
        {
            printf("#");
        }

        // The space between each side
        printf("  ");

        // This mirrors the left side
        for (int l = 0; l <= i; l++)
        {
            printf("#");
        }

        printf("\n");
    }
}