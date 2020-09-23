from sys import argv, exit
from cs50 import SQL


def main():

    # Get the length of the received arguments
    argc = len(argv)
    if argc != 2:
        print("Usage: python roster.py name_of_house")
        exit(1)

    # Connect to the database
    db = SQL("sqlite:///students.db")

    # Preset execution query
    query = "SELECT first\
                    , middle\
                    , last\
                    , birth\
            FROM students\
            WHERE students.house = ?\
            ORDER BY students.last\
                    , students.first"

    # Execute the query and show the results
    query_res = db.execute(query, argv[1])

    # Print results
    for row in query_res:
        first = row["first"]
        middle = ""
        last = row["last"]
        birth = row["birth"]
        if row["middle"] != None:
            middle = row["middle"]
            print(first, middle, last + f", born {birth}")
        else:
            print(first, last + f", born {birth}")

    exit(0)


main()