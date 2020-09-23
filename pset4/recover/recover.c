#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define BYTE1 0xff
#define BYTE2 0Xd8
#define BYTE4 0xe0
#define MASK  0xf0

#define JPEG_HDRS 4

#define BLOCK_SIZE 512
#define IMG_FILENAME_LEN 8

#define true 1
#define false 0

typedef uint8_t BYTE;

int is_jpeg(BYTE *, BYTE []);
FILE *create_jpeg(char *, int *);
int write_block(BYTE *, int, FILE *, char *);
void scan_memcard(FILE *);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./recover image\n");
        return 1;
    }

    // Remember filename
    char *infile = argv[1];

    // Open file
    FILE *memcard = fopen(infile, "r");
    if (memcard == NULL)
    {
        fprintf(stderr, "%s can't open '%s'.\n", argv[0], argv[1]);
        return 1;
    }

    scan_memcard(memcard);
    fclose(memcard);

    return 0;
}

// Return true if the buffer contains a jpeg file
int is_jpeg(BYTE *buf, BYTE *h)
{
    int res = true;

    for (int i = 0; i < (JPEG_HDRS - 1) && res; i++)
    {
        res = (h[i] == buf[i]) ? true : false;
    }

    return (res) ? (h[3] == (buf[3] & MASK)) : false;
}

// Encapsulate the creation of a new jpeg file to simplify process
FILE *create_jpeg(char *fname, int *fcounter)
{
    // Open new file
    sprintf(fname, "%03i.jpg", *fcounter);

    FILE *fptr = fopen(fname, "w");

    if (! fptr)
    {
        return NULL;
    }

    // Increment filename id counter
    *fcounter += 1;
    return fptr;
}

// Encapsulate write of block to avoid repetition
int write_block(BYTE *buf, int nread, FILE *fptr, char *fname)
{
    return (fwrite(buf, sizeof(BYTE), nread, fptr) == nread);
}

void scan_memcard(FILE *memcard)
{
    // Pointer to the img file
    FILE *img_ptr = NULL;
    // Pointer to the name of the img file
    char *img_name;
    // Filename id counter
    int fcounter = 0;
    // Array for storing each read block
    BYTE *buffer;
    // Array of expected jpeg headers
    BYTE headers[JPEG_HDRS] = {BYTE1, BYTE2, BYTE1, BYTE4};
    // Store the read bytes
    int nread;

    // Allocate the memory required
    buffer = (BYTE *) malloc(sizeof(BYTE) * BLOCK_SIZE);
    img_name = (char *) malloc(sizeof(char) * IMG_FILENAME_LEN);

    // Process the file
    while ((nread = fread(buffer, sizeof(BYTE), BLOCK_SIZE, memcard)) == BLOCK_SIZE)
    {
        // Has the read block a jpeg header?
        if (is_jpeg(buffer, headers))
        {
            // If there was an open file, close it
            if (img_ptr != NULL)
            {
                fclose(img_ptr);
            }

            // Try to open new file
            if ((img_ptr = create_jpeg(img_name, &fcounter)))
            {
                // Write jpeg's body
                if (! write_block(&buffer[0], nread, img_ptr, img_name))
                {
                    fprintf(stderr, "Error writing %i bytes to %s. Aborting...", nread, img_name);
                }
            }
            else
            {
                fprintf(stderr, "Error creating %s. Aborting...\n", img_name);
                break;
            }
        }
        // Block doesn't have a jpeg header, but it can be body
        // If there's an open fdescriptor, it is body
        else if (img_ptr)
        {
            // Write jpeg's body
            if (! write_block(&buffer[0], nread, img_ptr, img_name))
            {
                fprintf(stderr, "Error writing %i bytes to %s file. Aborting...", nread, img_name);
            }
        }
    }

    // Free allocated memory
    free(buffer);
    free(img_name);
    return;
}