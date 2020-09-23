from cs50 import get_int
from cs50 import get_string
from sys import exit


def main():
    card_num = get_card_num()
    card_len = checksum(card_num)
    if (card_len == -1):
        print("INVALID")
        exit(1)
    print(id_provider(card_num, card_len))
    exit(0)


# Ask the user for the card number
def get_card_num():
    while True:
        num = get_int("Card number: ")
        # Only allow positive integers
        if num >= 1:
            break
    return num


def checksum(number):

    copy = number
    step_1_accum = 0
    step_2_accum = 0
    index = 0

    # Walk through the whole number
    while(copy > 0):

        # Get the last digit
        digit = copy % 10
        # Now, the action depends on the position of every digit
        # If 'index' is odd -> execute step #1
        # If 'index' is even -> execute step #2
        if ((index % 2) == 1):
            # Raise by two the digit and accum the result
            doubled = digit * 2
            # If the digit converts to a double digit number, add both digits
            while(doubled > 9):
                # Get the last digit and accum it
                step_1_accum += doubled % 10
                # Remove the processed digit
                doubled = doubled // 10

            # Accumulate the digit
            step_1_accum += doubled
        else:
            # Accum the digits as they are
            step_2_accum += digit

        # Remove the processed digit
        copy = copy // 10
        # Increment index
        index += 1

    # Finally, step #3
    # Add both accums and verify if last digit is 0
    # If valid return the number of digits the number has, else -1
    result = step_1_accum + step_2_accum
    if ((result % 10) == 0):
        return index
    else:
        return -1


def id_provider(number, number_len):

    # Keep only the two left-most digits
    for i in range(number_len - 2):
        number = number // 10

    provider = "INVALID"

    if ((number // 10) == 4 and (number_len == 13 or number_len == 16)):
        provider = "VISA"

    if (number_len == 15 and (number == 34 or number == 37)):
        provider = "AMEX"

    if (number_len == 16 and (number >= 51 and number <= 55)):
        provider = "MASTERCARD"

    return provider


main()

