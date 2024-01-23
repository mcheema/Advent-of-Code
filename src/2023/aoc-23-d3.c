/*  -*- mode: C -*- */
/* This file conforms to C99 */

/*
  Build Instructions:
  PATH=../../build/:${PATH}
  clang -std=c99 -Wall -Wextra aoc-23-d3.c  -O3 -g -o ../../build/aoc-23-d3
  clang -std=c99 -Wall -Wextra aoc-23-d3.c  -DTEST -g -o ../../build/aoc-23-d3

  Program written for the Advent of Code day 3 2023

  First example comes from the problem itself
  "../../data/aoc-23-d3-ex1.txt"
  Next two examples come from https://www.reddit.com/r/adventofcode/comments/189q9wv/2023_day_3_another_sample_grid_to_use/
  "../../data/aoc-23-d3-ex2.txt" part 1: 413 part 2: 6756
  "../../data/aoc-23-d3-ex3.txt" part 1: 965 part 2: 6756

  example2: echo $((12 + 34 + 78 + 23 + 90 + 2 + 56 + 1))
  example2: echo $((12*4 + 34 + 78*2 + 23 + 90 + 2*2 + 56 + 1*2))


 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef TEST
#define FILENAME "../../data/aoc-23-d3-ex.txt" /* Example from the problem set */
#else
#define FILENAME "../../data/aoc-23-d3.txt"
#endif

#define MAX_COLS 256  /* based on: head -n 1 ../data/aoc-2023-d3.txt | wc | awk '{ print $3 - 1}' */
#define MAX_ROWS 256  /* Basec on:wc -l ../data/aoc-2023-d3.txt | awk  '{print $1}' */
#define MAX_DIGITS 32 /* Max digits in the schematic part number */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX_FNAME_LEN 512


static int schematic_length(char const *const fname);
static int schematic_width(char const *const fname);
static int schematic_fill(char *const sch, int const nrows, int const ncols, char const *const fname);
static void schematic_print_row(char const *const sch, int const row, int const ncols);
static bool is_valid_symbol(char const c);
static bool is_symbol_adjacent(char const *const sch, int const beg_col, int const end_col, int const row, int const nrows,
                               int const ncols);
static int schematic_part_number_value(char const *const sch, int const beg, int const end, int const row, int const ncols);
static int schematic_scan_and_sum_valid_part_numbers(char const *const sch, int const nrows, int const ncols);
static void schematic_check_chars(char const *const sch, int const nrows, int const ncols);
static bool is_new_part_number(int part_number);

int main(int argc, char *argv[])
{
    char fname[MAX_FNAME_LEN];
    if (argc == 2)
    {
        strcpy(fname, argv[1]);
    }
    else
    {
        strcpy(fname, FILENAME);
    }
    int const ncols = schematic_width(fname);
    int const nrows = schematic_length(fname);
    char *const schematic = malloc(sizeof(char) * nrows * ncols);
    schematic_fill(schematic, nrows, ncols, fname);
    for (int i = 0; i < nrows; i++)
    {
        schematic_print_row(schematic, i, ncols);
    }
    schematic_check_chars(schematic, nrows, ncols);
    /* compute the sum of the values of the valid parts */
    int const cumsum = schematic_scan_and_sum_valid_part_numbers(schematic, nrows, ncols);
    printf("The value of the sum of the valid part numbers is: %i\n", cumsum);
    free(schematic);
    return EXIT_SUCCESS;
}

static int schematic_width(char const *const fname)
{
    FILE *f = fopen(fname, "r");
    char first_line[MAX_ROWS];
    fscanf(f, "%[^\n]", first_line);
    fclose(f);
    return strlen(first_line);
}

static int schematic_length(char const *const fname)
{
    FILE *f = fopen(fname, "r");    
    int row_count = 0;
    char c;
    while ((c = fgetc(f)) != EOF)
    {
        if (c == '\n')
        {
            row_count++;            
        }
    }
    
    fclose(f);
    return row_count;
}

static int schematic_fill(char *const sch, int const nrows, int const ncols, char const *const fname)
{
    FILE *f = fopen(fname, "r");    
    int cur_row = 0;
    int cur_col = 0;
    char c;
    while ((c = fgetc(f)) != EOF)
    {
        if (c == '\n') 
        {
            cur_row++;
            cur_col = 0;
        }
        else
        {
            sch[cur_row * ncols + cur_col] = c;
            cur_col++;
        }
    }
    fclose(f);
    if (cur_row != nrows)
    {
        fprintf(stderr, "Warning: unexpected size of schematic");        
    }
    return 0;
}

static int schematic_scan_and_sum_valid_part_numbers(char const *const sch, int const nrows, int const ncols)
{
    bool digit_found = false; /* are we currently scanning a part nummber? */
    int beg_part_number_col = 0;
    int end_part_number_col = 0;
    int cumsum = 0;
    for (int i = 0; i < nrows; i++)
    {
        for (int j = 0; j < ncols; j++)
        {
            char c = sch[i * ncols + j];
            if (digit_found)
            { /* we are scanning a part number */
                if (isdigit(c) && (j != (ncols - 1)))
                {
                    end_part_number_col = j;
                }
                else
                { /* We have completed scanning a part number */
                    if (j == (ncols - 1))
                    {
                        end_part_number_col = j;
                    }
                    if (is_symbol_adjacent(sch, beg_part_number_col, end_part_number_col, i, nrows, ncols))
                    {
                        /* Note part number value is 0 if the part number has already been seen */
                        cumsum += schematic_part_number_value(sch, beg_part_number_col, end_part_number_col, i, ncols);
                    }
                    digit_found = false;
                    beg_part_number_col = j; /* reset the beginning digit column */
                    end_part_number_col = j; /* reset the end digit column */
                }                            /* we have completed scanning a part number */
            }
            else
            { /* we are not scanning a part number */
                if (isdigit(c))
                {
                    /* new potential part number */
                    digit_found = true;
                    beg_part_number_col = j;
                    end_part_number_col = j;
                }
                else
                {
                    continue;
                } /* continue scanning the row for the next part number */
            }     /* we are not scanning for a part number */
        }         /* Scan row for part numbers */
        /* after processing each row do the following: */
        beg_part_number_col = 0; /* reset the beginning digit column for the next row */
        end_part_number_col = 0; /* reset the end for the next row */
        digit_found = false;     /* reset the search for a part number for the next row */
    }                            /* Scan each row for part numbers */

    return cumsum;
} /* End of scan and sum */ 

static void schematic_print_row(char const *const sch, int const row, int const ncols)
{
    char const *const row_ptr = sch + row * ncols;
    printf("%3i ", row);
    for (int i = 0; i < ncols; i++)
    {
        printf("%c", row_ptr[i]);
    }    
    printf("\n");
}

static bool is_valid_symbol(char const c)
{
    return (ispunct(c) && (c != '.')) ? true : false;
}

/* Specifiy the location within the schema of the number by giving it's row number and beg and end column number*/
static bool is_symbol_adjacent(char const *const sch, int const beg_col, int const end_col, int const row, int const nrows,
                               int const ncols)
{
    /* The rectangle within which to search for the adjacent symbol */
    int from_row = MAX(row - 1, 0);
    int to_row = MIN(row + 1, nrows - 1);
    int from_col = MAX(beg_col - 1, 0);
    int to_col = MIN(end_col + 1, ncols - 1);

    /* Look for symbol */
    for (int i = from_row; i <= to_row; i++)
    {
        for (int j = from_col; j <= to_col; j++)
        {
            char c = sch[i * ncols + j];
            if (is_valid_symbol(c))
            {
                return true;
            }
        }
    }
    return false;
}

static int schematic_part_number_value(char const *const sch, int const beg, int const end, int const row, int const ncols)
{
    char partstr[MAX_DIGITS] = "";
    int len = 0;
    for (int i = beg; i <= end; i++)
    {
        char c = sch[row * ncols + i];
        partstr[len] = c;
        len++;
    }
    partstr[len] = '\0';
    /* To check the sum of these in the output use the following AWK command*/
    /* awk 'BEGIN { FS=OFS=" "; cumsum = 0; } //{ if (NF == 5) cumsum += $5} END {print cumsum}'*/
    int part_number = atoi(partstr);
    /* if (is_new_part_number(part_number)) */
    /* { */
    /*     return part_number; */
    /* } */
    /* else */
    /* { */
    /*     return 0; */
    /* } */
    FILE *f = fopen("./numbers.dat", "a");
    /* awk 'BEGIN { cumsum = 0 } { cumsum += $1; } END { print cumsum}' numbers.dat */
    fprintf(f, "%i\n", part_number);
    fclose(f);
    return part_number;
}

/* Diagnostic function to test each character whether it is a digit or punctuation symbol */
static void schematic_check_chars(char const *const sch, int const nrows, int const ncols)
{
    for (int i = 0; i < nrows; i++)
    {
        for (int j = 0; j < ncols; j++)
        {
            char c = sch[i * ncols + j];
            if (ispunct(c) || isdigit(c))
            {
                continue;
            }
            else
            {
                printf("Char %c does not fit any criteria!\n", c);
            }
        }
    }
}

/* bool is_new_part_number(int part_number) */
/* { */
/*     static bool part_number_found[1000] = { 0 }; /\* intialize to not found for each number *\/ */
/*     if (!part_number_found[part_number]) */
/*     { */
/*         FILE *f = fopen("./numbers.dat", "a"); */
/*         /\* cat numbers.dat | uniq | sort | awk 'BEGIN { cumsum = 0 } { cumsum += $1; } END { print cumsum}'*\/ */
/*         fprintf(f, "%i\n", part_number); */
/*         fclose(f); */
/*         part_number_found[part_number] = true; */
/*         return true; */
/*     } */
/*     else */
/*     { */
/*         return false; */
/*     } */
/* } */
