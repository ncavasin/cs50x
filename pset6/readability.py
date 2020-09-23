from cs50 import get_string
from sys import exit


def main():
    text = (get_string("Ingrese texto: "))

    # Create a list to store the results
    result = [0, 1, 0]

    # Parse the input text
    parse(text, result)

    # Process the results
    grade = compute(result)

    # Inform grade
    if (grade >= 16):
        print("Grade 16+")
    elif (grade < 1):
        print("Before Grade 1")
    else:
        print(f"Grade {grade}")
    exit(0)


def parse(txt, res):
    """
    Parse the received string. Sum every letter, word and sentence.
    """
    for char in txt:
        if (char.isalpha()):
            res[0] += 1
        if (char.isspace()):
            res[1] += 1
        if char in (".", "!", "?"):
            res[2] += 1
    return


def compute(res):
    """
    Receives a list of lettters, words and sencenteces
    and applies Coleman-Liau formulae.
    """

    # Avg of letters per 100 words
    L = res[0] * 100 / res[1]
    # Avg of sentences per 100 words
    S = res[2] * 100 / res[1]
    # Coleman-Liau formulae
    grade = 0.0588 * L - 0.296 * S - 15.8
    # Round the grade to the closest integer
    return round(grade)


main()