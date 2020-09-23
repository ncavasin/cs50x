from sys import argv, exit
import csv


def main():

    # Verify received arguments
    if (len(argv) != 3):
        print("Usage: python filename.py <input1.csv> <input2.txt>")
        exit(1)

    # Store a copy of the input file path
    in_csv_path = argv[1]
    in_txt_path = argv[2]

    # Read csv to the database list
    database = read_csv(in_csv_path)

    # Read the sequence of DNAs
    sequence = read_txt(in_txt_path)

    # Parse the sequence
    results = parse_sequence(sequence, database)

    # Check if the results match to any person on the database
    person = check_matches(database, results)

    # Inform result
    if person:
        print(person)
    else:
        print("No match")
    exit(0)


def read_csv(path):
    """
    Reading .csv as python docs suggests:
    https://docs.python.org/3/library/csv.html

    This reads every row and stores it as a list inside of the list
    called database.
    """
    # Initialize database as a mutable list.
    # It will be a list of lists, where every inner list will represent data
    # of a different person.
    db = []

    # Open csv file
    with open(path, newline='') as csv_file:

        # Get an iterator to iterate over every csv row
        iterable = csv.reader(csv_file)
        # Iterate over every csv row
        for row in iterable:
            db.append(row)

    # 'with' method automatically closes the opened file once indentation is broken
    # so just return to main.
    return db


def read_txt(path):
    """
    Reading .txt as python docs suggests:
    https://docs.python.org/3/tutorial/inputoutput.html#reading-and-writing-files
    """
    # Open txt file
    with open(path, "r") as txt_file:
        # Read only one line, as the sequence is stored in a one-line fashion
        seq = txt_file.readline()

    return seq


def parse_sequence(seq, db):
    """
    Parses the list 'seq' looking for any STR repetition.

    This method uses a dict to store the results:
    * Every STR works as key, based on the .csv header.
    * The max repetition of every STR is stored as value.


    Returns the dict.
    """

    # Create the dict
    res = {}

    # Get the possible DNA chains, avoid col index 0 as it contains 'name'
    chains = [c for c in db[0][1:]]

    # For every STR chain found in the csv
    for chain in chains:

        # Insert the chain as key and initialize its value to 0
        res[chain] = 0

        # Get the chain's length
        j = len(chain)

        # Initialize the current chain consecutive occurrences accum
        cur_max = 0

        # For every character in the DNA sequence
        for i, char in enumerate(seq):

            # Get a substring of current chain's length
            substring = seq[i:i+j]

            # Initialize temp_max
            temp_max = 0

            # Remember we're counting how many CONSECUTIVES OCURRENCIES
            # are in the sequence, NOT how many ocurrences. So, if there's a
            # match: launch the sub-stringing accumulate and advance
            while(substring == chain):

                # Advance a chain-length in the DNAs sequence
                i = i + j

                # Adjust substring properly
                substring = seq[i:i+j]

                # Increment the consecutive ocurrence
                temp_max = temp_max + 1

            # If there were more consecutive occurrences than before
            if temp_max > cur_max:
                cur_max = temp_max

        # Store the max consecutive occurences of current chain in the dict
        res[chain] = cur_max

    return res


def check_matches(db, res):
    """
    Iterates through the database and checks if any person matches the
    processed DNA sequence

    If found returns 'name', else 'None'
    """

    # For every row in the database
    for row in db[1:]:

        # For every value in a row
        # Get a column index of every element inside the row
        for col, value in enumerate(row[1:]):

            # Use the column index to get the chain occurrences from dict
            # (the chains key are in db[0])
            chain_occurrences = res.get(db[0][col+1])

            # If they aren't equal break
            if int(value) != chain_occurrences:
                break

            # If they're equal and we've processed the whole row
            # Substrac 2 to row's len: 1 for header and 1 for counting  from 0
            if int(value) == chain_occurrences and col == (len(row)-2):
                # Return person's name
                return row[0]
            # If not, keep processing until one of both happens

        # Advance to the next row

    return None


main()

