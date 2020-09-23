#include "helpers.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_COLOR_VALUE 255
#define KERNEL_SIZE 3

// Receive a pointer to pixel and return the average value of its colors
int get_avg(RGBTRIPLE *px, int accounted)
{

    // Check division by zero
    if (accounted == 0)
    {
        return -1;
    }

    int result = (int) round((float)(px->rgbtRed + px->rgbtGreen + px->rgbtBlue) / (float) accounted);

    return result;
}

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE *pixel;
    int avg;

    // For every row
    for (int i = 0; i < height; i++)
    {
        // For every column
        for (int j = 0; j < width; j++)
        {
            // Get the pixel's address
            pixel = &image[i][j];

            // Get the average of its colors
            avg = get_avg(pixel, 3);

            // Redefine the pixel's gray value
            pixel->rgbtRed = avg;
            pixel->rgbtGreen = avg;
            pixel->rgbtBlue = avg;
        }
    }
    return;
}

// Swap two pixels
void swap(RGBTRIPLE *px, int row, int src_col, int dst_col, int h, int w, RGBTRIPLE img[h][w])
{
    // Not a pointer but a struct to hold a copy of a pixel
    RGBTRIPLE aux;

    // Hold a copy
    aux = img[row][dst_col];

    // Reflect the actual pixel
    img[row][dst_col] = *px;

    // Restore the copy but in reflected position
    img[row][src_col] = aux;
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE *pixel;
    int half;

    /**
     * Two posible cases:
     * 1. Width is even:
     *      Half does not need to be rounded.
     *      Every column will swap.
     * 2. Width is odd:
     *      Half + 1 must be kept as it is.
     *      Half must be rounded to the nearest previous integer.
     *      That rounding can be done using round()-1 or just trunking.
     */
    for (int row = 0; row < height; row++)
    {
        // Intentionally trunk the division by 2 to the previous integer
        for (int col = 0; col < (width / 2); col++)
        {
            // Get the pixel address
            pixel = &image[row][col];

            // Reflect it
            swap(pixel, row, col, (width - col - 1), height, width, image);
        }
    }
    return;
}

/** Return the average color of a pixel's neighbor
 *  Receive:
 *      int color       => the color accumulator
 *      int accounted   => the number of processed neighbors
 * Return:
 *      int with the result of color / accounted properly done.
 */
int get_blur_avg(int color, int accounted)
{
    // Check division by zero
    if (accounted == 0)
    {
        return -1;
    }

    int result = (int) round((float) color / accounted);

    return ((result > MAX_COLOR_VALUE) ? MAX_COLOR_VALUE : result);
}


void grid_set_lower_limit(int *to_set, int actual_pos, int bound)
{
    *to_set = ((actual_pos - 1) < bound) ? bound : (actual_pos - 1);
    return;
}

void grid_set_upper_limit(int *to_set, int actual_pos, int bound)
{
    *to_set = ((actual_pos + 1) >= bound) ? bound - 1 : (actual_pos + 1);
    return;
}


// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    // A copy for the already processed and blurred pixels
    RGBTRIPLE copy[height][width];
    // Reference to original pixel and the one processed
    RGBTRIPLE *original, *grid;
    // Used for the blurring process
    int r, g, b, accounted;
    // Used to determine the size of neighbors' grid
    int grid_first_row, grid_first_col, grid_last_row, grid_last_col;

    // For every row
    for (int row = 0; row < height; row++)
    {
        // For every col
        for (int col = 0; col < width; col++)
        {
            // Get the pixel to process
            original = &image[row][col];

            // Set grid boundaries contemplating edge cases
            grid_set_lower_limit(&grid_first_row, row, 0);
            grid_set_upper_limit(&grid_last_row, row, height);
            grid_set_lower_limit(&grid_first_col, col, 0);
            grid_set_upper_limit(&grid_last_col, col, width);

            /**
             * Just to be clear with myself about edge cases:
             * Situation: width = 400, col = 399, grid_last_col = col + 1.
             *
             * As the grid values are relative, i must be sure that adding one to
             * grid_last_col never is >= than 400, otherwise i'll be out of the image's bounds.
             *
             * At the same time, i must take care of processing that last column (399). So, in
             * the iterations below i'll compare i and j to upper bounds using '<=' because i
             * need to process that last col/row.
             */

            // Initialize accums
            r = g = b = 0;
            accounted = 0;

            // Walk through the grid
            for (int i = grid_first_row; i <= grid_last_row; i++)
            {
                for (int j = grid_first_col; j <= grid_last_col; j++)
                {
                    // Get the pixel from the original
                    grid = &image[i][j];

                    // Store values
                    r += grid->rgbtRed;
                    g += grid->rgbtGreen;
                    b += grid->rgbtBlue;
                    accounted++;
                }
            }

            // Modify the colors of the processed pixel in the blurred copy
            copy[row][col].rgbtRed = get_blur_avg(r, accounted);
            copy[row][col].rgbtGreen = get_blur_avg(g, accounted);
            copy[row][col].rgbtBlue = get_blur_avg(b, accounted);
        }
    }

    // Replace the original values with the blurred copy ones
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = copy[i][j];
        }
    }
    return;
}

int cap_max(int color)
{
    return (color > MAX_COLOR_VALUE ? MAX_COLOR_VALUE : (int) color);
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE *original, *grid;
    RGBTRIPLE copy[height][width];
    RGBTRIPLE dummy = {.rgbtRed = 0, .rgbtGreen = 0, .rgbtBlue = 0};
    int conv_matrix[KERNEL_SIZE][KERNEL_SIZE] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};

    int x_accum_red, x_accum_green, x_accum_blue;
    int y_accum_red, y_accum_green, y_accum_blue;
    int abs_row, abs_col;

    // Walk through the image
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            // Initialize accums
            x_accum_red = x_accum_green = x_accum_blue = 0;
            y_accum_red = y_accum_green = y_accum_blue = 0;

            // Walk through the grid
            for (int r = 0; r < KERNEL_SIZE; r++)
            {
                for (int c = 0; c < KERNEL_SIZE; c++)
                {
                    // Calculate absolute positions on the image
                    abs_row = (row + r) - 1;
                    abs_col = (col + c) - 1;

                    /**
                     *
                     */

                    // If the location doesn't exist, make it a black pixel
                    if (abs_row < 0 || abs_row >= height || abs_col < 0 || abs_col >= width)
                    {
                        // Get a full black dummy pixel
                        grid = &dummy;
                    }
                    else
                    {
                        // Get a real pixel from the image
                        grid = &image[abs_row][abs_col];
                    }

                    // Do the math for the current pixel
                    x_accum_red += grid->rgbtRed * conv_matrix[r][c];
                    y_accum_red += grid->rgbtRed *conv_matrix[c][r];

                    x_accum_green += grid->rgbtGreen * conv_matrix[r][c];
                    y_accum_green += grid->rgbtGreen * conv_matrix[c][r];

                    x_accum_blue += grid->rgbtBlue * conv_matrix[r][c];
                    y_accum_blue += grid->rgbtBlue * conv_matrix[c][r];
                }
            }

            // Normalize values
            x_accum_red = pow(x_accum_red, 2);
            y_accum_red = pow(y_accum_red, 2);

            x_accum_green = pow(x_accum_green, 2);
            y_accum_green = pow(y_accum_green, 2);

            x_accum_blue = pow(x_accum_blue, 2);
            y_accum_blue = pow(y_accum_blue, 2);

            copy[row][col].rgbtRed = cap_max(round(sqrt(x_accum_red + y_accum_red)));
            copy[row][col].rgbtGreen = cap_max(round(sqrt(x_accum_green + y_accum_green)));
            copy[row][col].rgbtBlue = cap_max(round(sqrt(x_accum_blue + y_accum_blue)));
        }
    }

    // Modify the image
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            image[row][col] = copy[row][col];
        }
    }
    return;
}
