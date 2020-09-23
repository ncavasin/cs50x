import csv
from sys import argv, exit
from cs50 import SQL


def main():

    # Get the count of the received arguments
    argc = len(argv)
    if argc != 2:
        print("Usage: python3 import.py filename.extension")
        exit(1)

    # Connect to the database
    db = SQL("sqlite:///students.db")

    # Import data to database
    csv_to_db(argv[1], db)

    exit(0)


def csv_to_db(filepath, db):

    # Preset query
    query = "INSERT INTO students\
            (first, middle, last, house, birth)\
            VALUES (?, ?, ?, ?, ?)"

    # Open the csv file
    with open(filepath, "r") as csvfile:

        # Read a row and store it in a dict fashion
        row = csv.DictReader(csvfile)

        # For every key in the row
        for key in row:

            # Remove all the whitespaces in "name"
            name = key["name"].split(" ")

            # Store every value
            first = name[0]
            # Assume the person doesn't have middle-name
            middle = None
            last = name[1]
            house = key["house"]
            birth = key["birth"]

            # If the person has middle-name
            if len(name) == 3:
                middle = name[1]
                last = name[2]

            # Insert the row in the database
            db.execute(query, first, middle, last, house, birth)
    return


main()

