#include <stdio.h>
#include <cs50.h>


long get_card_number(void);
int  checksum(long);
void identify_provider(int, long);

int main(void)
{
    long card = get_card_number();

    int len = checksum(card);

    identify_provider(len, card);

}

// Asks for a number and validates it's > 0 and not a string
long get_card_number(void)
{
    long n;

    do
    {
        n = get_long("Card number:\n");
    }
    while (n < 1);

    return n;
}

// Verifies if the input number is a valid credit card number or not
int checksum(long n)
{

    // To store the additions
    int mult_accum = 0;
    int add_accum = 0;

    // Number to process
    int num = 0;

    // To calculate the length of the number
    int counter = 0;

    // Save the original value
    long copy = n;

    // While there's at least one number to process
    do
    {
        // Get the right-most number
        num = (copy % 10);

        // If counter is odd execute step #2
        if (counter % 2 == 0)
        {

            // Accumulate the digit (Step #2 of algorithm)
            add_accum += num;
        }
        // Counter is even, execute the step #1
        else
            // Multiply by 2 and accumulate the result (Step #1 of algorithm)
            num = num * 2;

            // If the result is a two-digit number, accumulate both digits and not the number per se.
            if (num > 9)
            {
                mult_accum += num % 10;

                num = num / 10;
            }

            mult_accum += num;
        }

        // "Remove" the processed number
        copy = copy / 10;
        // Increment counter
        counter++;
    }
    while (copy > 0);

    // If the number is valid, return its length
    if ((mult_accum + add_accum) % 10 == 0)
    {
        return counter;
    }
    else
    {
        return -1;
    }
}

// Defines the provider of the credit card number
void identify_provider(int length, long card)
{

    if (length == -1)
    {
        printf("INVALID\n");
        return;
    }

    // Store the copy
    long copy = card;

    for (int i = 0; i < (length - 2); i++) // Trim the number in order to keep the two left-most digits
    {
        copy = copy / 10;
    }

    if ((copy / 10) % 10 == 4 && (length == 13 || length == 16))
    {
        printf("VISA\n");
    }
    else if (length == 15 && ((copy == 34) || (copy == 37)))
    {
        printf("AMEX\n");
    }
    else if (((copy >= 51) && (copy <= 55)) && length == 16)
    {
        printf("MASTERCARD\n");
    }
    else
    {
        printf("INVALID\n");
    }
}