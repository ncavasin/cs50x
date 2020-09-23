#include <cs50.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

int is_valid(string);
string encipher(string, string);
string adjust_sensitivity();


#define KEY_LENGTH 26

int main(int argc, string argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./substitution key");
        return 1;
    }

    if (strlen(argv[1]) != KEY_LENGTH)
    {
        printf("Key must contain %i characters.", KEY_LENGTH);
        return 1;
    }

    if (! is_valid(argv[1]))
    {
        printf("Duplicates and non-alphabetic chars are not allowed.");
        return 1;
    }

    string key = argv[1];
    string input = get_string("plaintext: ");

    string enciphered = encipher(input, key);


    printf("ciphertext: %s\n", enciphered);

    return 0;
}

// Validate the cipher key
int is_valid(string key)
{
    int i = 0;
    int j = 0;
    int valid = 1;

    while (key[i] != '\0' && valid)
    {
        if (! isalpha(key[i]))    // If not an alfabetic char, key is invalid
        {
            valid = 0;
        }

        j = i + 1;

        while (j < KEY_LENGTH - i && valid)   // Do not admit duplicated chars
        {
            if (key[i] == key[j])
            {
                valid = 0;
            }
            j++;
        }
        i++;
    }
    return valid;   // Key is valid
}

// Encipher using the key
string encipher(string input, string key)
{
    int i = 0;
    int ascii_value;
    int case_sens;
    int mapping;
    string output = input;

    while (input[i] != '\0')
    {

        if (isalpha(input[i]))  // If it's an alphabetic char
        {
            ascii_value = (int) input[i];  // Get char's ascii number
            case_sens = (int) 'a';    // Assume the char is in lowercase

            if (isupper(input[i]))
            {
                case_sens = (int) 'A';  // Adjust if it's uppercase
            }

            mapping = ascii_value - case_sens;    // Map input char with key cipher replacement
            output[i] = key[mapping];   // Assume replacement and key have the same case-sensitivity

            if (isupper(case_sens) > isupper(key[mapping]))     // Adjust if not
            {
                output[i] = toupper(key[mapping]);
            }
            else if (islower(case_sens) > islower(key[mapping]))
            {
                output[i] = tolower(key[mapping]);
            }
        }
        else    // Else let it be
        {
            output[i] = input[i];
        }
        i++;
    }

    output[i + 1] = '\0'; // It's over

    return output;
}

