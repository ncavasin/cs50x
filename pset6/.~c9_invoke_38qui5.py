from cs50 import get_int
from cs50 import get_string

def main():
    card_num = get_card_num()
    checksum(card_num)
    return 0


# Ask the user for the card number
def get_card_num():
    # Only
    while True:
        num = get_int("Card number:")
        if num > 1:
            break
    return num

# Verifies tha
def checksum(cardnum):

    return True


main()