// Implements a dictionary's functionality

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "dictionary.h"

// Used for hash function
#define BASE_POLYNOMIAL 7

// Represents a node in a hash table
typedef struct node
{
    char word[LENGTH + 1];
    struct node *next;
}
node;

// Number of buckets in hash table
const unsigned int N = 77777;

// Hash table
node *table[N];

// Global words counter
unsigned int word_counter;


// Buffer to read every word
char word_buffer[LENGTH];

char *normalize(const char *word)
{
    int len = strlen(word);

    // Allocate just the needed space + 1 byte.
    // The additional byte is for concatening the '\0'.
    char *norm = (char *) calloc(1, sizeof(char) * (len + 1));

    for (int i = 0; i < len; i++)
    {
        norm[i] = tolower(word[i]);
    }

    // Add the end-of-string delimiter
    strcat(norm, "\0");
    return norm;
}

// Returns true if word is in dictionary else false
bool check(const char *word)
{
    // Normalize the word in order to get a correct hash index
    char *normalized_word = normalize(word);

    node *n = table[hash(normalized_word)];

    // Traverse the linked list
    while (n)
    {
        // Compare the word with the original word
        if ((strcasecmp(n->word, word)) == 0)
        {
            free(normalized_word);
            return true;
        }
        // Advance to the next node
        n = n->next;
    }
    free(normalized_word);
    // The word isn't in the table
    return false;
}

/**
 * Hashes word to a number
 * Source of hash function:
 * http://cseweb.ucsd.edu/~kube/cls/100/Lectures/lec16/lec16-14.html
 */
unsigned int hash(const char *word)
{
    /**
     * No need to normalize the word to lowercase, as it comes from
     * load method where the dictionary is already in lowercase or
     * it comes from check method where it's properly normalized.
     */

    unsigned int accum = 0;
    int len = strlen(word);

    // Multiply and accum BASE_POLYNOMIAL to the ascii value of every char
    for (int i = 0; i < len; i++)
    {
        accum += BASE_POLYNOMIAL * (int) word[i];
    }
    return (accum % N);
}

// Adds a node to the collisions list
bool add_collision(char *word)
{
    // Point to the address of the indexed node of the table
    node *table_node = table[hash(word)];

    // Allocate memory for a new node and connect it to the list
    node *allocated_node = (node *) malloc(sizeof(node));

    // Store the word in the allocated node
    strcpy(allocated_node->word, word);

    // Mark it as last one
    allocated_node->next = NULL;

    // Store the old first collision node
    node *aux = table_node->next;

    // Connect allocated node with the first collision node of the list
    allocated_node->next = aux;

    // Connect the table node with the allocated node
    table_node->next = allocated_node;
    return true;
}

bool init_table()
{
    // Allocate space
    for (int i = 0; i < N; i++)
    {
        table[i] = (node *) calloc(1, sizeof(node));
        if (! table[i])
        {
            return false;
        }
    }
    return true;
}

// Loads dictionary into memory, returning true if successful else false
bool load(const char *dictionary)
{

    // Stores the result of the hash function
    unsigned long index;

    // Initialize global word counter
    word_counter = 0;

    if (! init_table())
    {
        return false;
    }

    // Try to open the file
    FILE *infile = fopen(dictionary, "r");
    if (! infile)
    {
        return false;
    }

    // Read file line by line
    while (fscanf(infile, "%s", word_buffer) != EOF)
    {
        // Get the index
        index = hash(word_buffer);

        // If it's an empty bucket
        if (! table[index])
        {
            // Copy the content in the allocated table node
            strcpy(table[index]->word, word_buffer);
        }
        // If the bucket wasn't empty, it's a collision
        else
        {
            // Add the collision
            if (! add_collision(word_buffer))
            {
                return false;
            }
        }
        // Increment the global words counter
        word_counter++;
    }

    fclose(infile);
    return true;
}

// Returns number of words in dictionary if loaded else 0 if not yet loaded
unsigned int size(void)
{
    return word_counter;
}

// Unloads dictionary from memory, returning true if successful else false
bool unload(void)
{
    node *table_node, *cursor;

    // Walk through the whole table
    for (int i = 0; i < N; i++)
    {
        table_node = table[i];

        // For every node in this linked list
        while (table_node)
        {
            // A copy of the pointer
            cursor = table_node;
            // Advance to the next
            table_node = table_node->next;
            // Free current
            free(cursor);
        }

        // Free the node of the hash table
        free(table_node);
    }
    word_counter = 0;
    return true;
}
