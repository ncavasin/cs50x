#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }
    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    // Walk through the candidates
    for (int candidate_index = 0; candidate_index < candidate_count; candidate_index++)
    {
        // If name is a valid candidate
        if (strcmp(name, candidates[candidate_index]) == 0)
        {
            // Register the vote
            ranks[rank] = candidate_index;
            return true;
        }
    }
    return false;
}

// Update preferences given each voter ranks
void record_preferences(int ranks[])
{
    // Walk through preferences[][] rows
    for (int i = 0; i < candidate_count; i++)
    {
        // For every column on preferences[][]
        for (int j = i + 1; j < candidate_count; j++)
        {
            // Augment by one the preference over the remaining candidates.
            preferences[ranks[i]][ranks[j]]++;
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    // Walk through every row of preferences[][]
    for (int i = 0; i < candidate_count; i++)
    {
        // Walk only through half of matrix, as the other half is checked shifting i and j
        for (int j = i + 1; j < candidate_count; j++)
        {
            // Compare and discover the winner and the loser
            if (preferences[i][j] != preferences[j][i])
            {
                // Asume a winner
                pairs[pair_count].winner = i;
                pairs[pair_count].loser = j;

                // Correct if wrong
                if (preferences[i][j] < preferences[j][i])
                {
                    pairs[pair_count].winner = j;
                    pairs[pair_count].loser = i;
                }
                pair_count++;
            }
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    int max_pos;
    pair aux;
    for (int i = 0; i < pair_count - 1; i++)
    {
        max_pos = i;
        for (int j = i + 1; j < pair_count; j++)
        {
            // If there's a winner pair by a bigger difference than the actual pair i'm looking at...
            if (i != j && preferences[pairs[j].winner][pairs[j].loser] > preferences[pairs[max_pos].winner][pairs[max_pos].loser])
            {
                // Store its position
                max_pos = j;
            }
        }
        // Once a round is completed, swap pushing max to the ith place of the array
        aux = pairs[i];
        pairs[i] = pairs[max_pos];
        pairs[max_pos] = aux;
    }
    return;
}

// Recursive check of candidate visit
// Receives the index of a candidate and:
//      1 - Verifies if it was visited
//      2 - Verifies if any of his descendants were visited
bool check_visited(int index, bool visited[])
{
    // If the node has been visited, there's a cycle
    if (visited[index])
    {
        return true;
    }
    // If not visited, mark it as visited
    visited[index] = true;
    // Then check all of the other candidates it points to using locked array
    for (int i = 0; i < candidate_count; i++)
    {
        // If it points to a candidate, verify if the new edge creates a cycle between the ones existing
        if (locked[index][i] && check_visited(i, visited))
        {
            return true;
        }
    }
    // If the execution arrives at this point, there's no cycle
    return false;
}

// Defines if the locked edge creates a cycle.
// Receives the index of the added candidate and
// creates a boolean array of candidates
// to simulate candidate visits
bool creates_cycle(int candidate_index)
{
    // This array stores the visited candidates
    bool visited[candidate_count];

    // Populate the array
    for (int i = 0; i < candidate_count; i ++)
    {
        visited[i] = false;
    }
    // Verify if it makes a cycle
    return check_visited(candidate_index, visited);
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    // Walk through the pairs array
    for (int i = 0; i < pair_count; i++)
    {
        // Lock all pairs just as they are
        locked[pairs[i].winner][pairs[i].loser] = true;

        // Verify if cycles are created after lock
        if (creates_cycle(pairs[i].winner))
        {
            // Remove the added edge, it creates a cycle
            locked[pairs[i].winner][pairs[i].loser] = false;
        }
    }
    return;
}

// Print the winner of the election
void print_winner(void)
{
    int count;
    int max = 0;
    int max_pos = 0;

    for (int i = 0; i < MAX; i++)
    {
        count = 0;
        for (int j = 0; j < MAX; j++)
        {
            // If nobody points the candidate
            if (! locked[j][i])
            {
                count++;
            }
        }
        // If there's a new max of unpointed candidate, store its value and position
        if (count > max)
        {
            max_pos = i;
            max = count;
        }
    }
    // The candidate with less arrows pointing to him (more zeroes in its column) is the source and winner
    printf("%s\n", candidates[max_pos]);
    return;
}

