#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <cs50.h>

int parse(string);


int main(void)
{

    string input = get_string("Text:");

    int grade = parse(input);


    if (grade > 16)
    {
        printf("Grade 16+\n");
    }
    else if (grade < 1)
    {
        printf("Before Grade 1\n");
    }
    else
    {
        printf("Grade %i\n", grade);
    }

    return 0;
}

// Parse the text and return a round number based on the given formula
int parse(string str)
{

    int i = 0;          // input's index

    int letters = 0;    //     #
    int words = 1;      // Counters
    int sentences = 0;  //     #

    while (str[i] != '\0')    // While not EOF
    {
        if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z'))
        {
            letters++;
        }
        else if (str[i] == ' ')    // End of word
        {
            words++;
        }
        else if (str[i] == '!' || str[i] == '?' || str[i] == '.')    // End of sentence
        {
            sentences++;
        }

        i++;
    }

    float L = (float) letters * 100 / (float) words;

    float S = (float) sentences * 100 / (float) words;

    float index = 0.0588 * L - 0.296 * S - 15.8;

    return round(index);
}