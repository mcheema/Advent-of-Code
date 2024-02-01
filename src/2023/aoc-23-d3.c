/*  -*- mode: C -*- */
/* This file conforms to C99 */

/*
  Build Instructions:
  PATH=../../build/:${PATH}
  clang -std=c99 -Wall -Wextra aoc-23-d3.c  -O3 -g -o ../../build/aoc-23-d3
  clang -std=c99 -pedantic -Wvla -Wall -Wextra -fsanitize=address aoc-23-d3.c -g -o ../../build/aoc-23-d3

  Program written for the Advent of Code day 3 2023
  First example comes from the problem itself
  "../../data/aoc-23-d3-ex1.txt"
  Next two examples come from https://www.reddit.com/r/adventofcode/comments/189q9wv/2023_day_3_another_sample_grid_to_use/
  "../../Data/aoc-23-d3-ex2.txt" part 1: 413 part 2: 6756
  "../../data/aoc-23-d3-ex3.txt" part 1: 965 part 2: 6756

  example2 part I: echo $((12*4 + 34 + 78*2 + 23 + 90 + 2*2 + 56 + 1*2))
  example2 part2: echo $((78 *78 + 12 * 56)) 6756
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 256  /* based on: head -n 1 ../data/aoc-2023-d3.txt | wc | awk '{ print $3 - 1}' */
#define MAX_ROWS 256  /* Basec on:wc -l ../data/aoc-2023-d3.txt | awk  '{print $1}' */
#define MAX_DIGITS 8 /* Max digits in the schematic part number */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX_FNAME_LEN 128

/*
  primary data structure with a single array with all the non newline characters
  in row major order
*/
struct schematic {
    char *grid;
    int nrows;
    int ncols;
};

/*
  A window is a subset of the full schematic
  It incumbant on the programmer to make sure the window parameters
  properly overlap the schematic
 */
struct window {
    /* Location of window within the schematic */
    int from_row;
    int to_row;
    int from_col;
    int to_col;
    /* Absolute properties of the window  */
    int rows;   /* no of rows in window */
    int cols;   /* no of cols in window */
    bool *used; /* array of flags size of the window representing each cell in row major order */
};

/* Part numbers go left to right and have at most MAX_DIGITS - 1 */
struct part {
    int beg_col;
    int end_col;
    int row;
    int value;
    char partstr[MAX_DIGITS];
};

static struct schematic schematic_create(char const *const fname);
static void schematic_destroy(struct schematic s);
static int schematic_scan_and_sum_valid_parts(struct schematic *s);
static int schematic_scan_and_sum_gear_ratios(struct schematic const *const s);

int main(int argc, char *argv[])
{
    char fname[MAX_FNAME_LEN];
    if (argc == 2)
    {
        strcpy(fname, argv[1]);
    }
    else
    {
        fprintf(stderr, "USAGE: %s FILENAME\n", argv[1]);
        exit(EXIT_FAILURE);
    }    

    struct schematic s = schematic_create(fname);

    /* compute the sum of the values of the valid parts */
    int const cumsum = schematic_scan_and_sum_valid_parts(&s);
    printf("The value of the sum of the valid part numbers is: %i\n", cumsum);

    int const gearsum = schematic_scan_and_sum_gear_ratios(&s);
    printf("The value of the sum of the valid part numbers is: %i\n", gearsum);

    schematic_destroy(s);
    exit(EXIT_SUCCESS);
}

/* Helper for schematic create */
static int schematic_width(char const *const fname)
{
    FILE *f = fopen(fname, "r");
    char first_line[MAX_ROWS];
    fscanf(f, "%[^\n]", first_line);
    fclose(f);
    return strlen(first_line);
}

/* Helper for schematic create */
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

/* Helper for schematic create */
static int schematic_fill(struct schematic *const s, char const *const fname)
{
    char *const sch = s->grid;
    int const ncols = s->ncols;

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
    return 0;
}

#ifdef TEST
/* Helper for schematic create */
static void schematic_print(struct schematic const *const s)
{
    char const *const sch = s->grid;
    int const nrows = s->nrows;
    int const ncols = s->ncols;

    for (int i = 0; i < nrows; i++)
    {
        for (int j = 0; j < ncols; j++)
        {
            printf("%c", sch[i * ncols +i]);            
        }
        printf("\n");
    }    
}
#endif

/* Helper for schematic create */
static struct schematic schematic_create(char const *const fname)
{
    struct schematic sch;
    sch.nrows = schematic_length(fname);
    sch.ncols = schematic_width(fname);
    sch.grid = malloc(sizeof(char) * sch.nrows * sch.ncols);
    schematic_fill(&sch, fname);
#ifdef TEST    
    schematic_print(&sch);
#endif
    return sch;
}

static void schematic_destroy(struct schematic s)
{
    if (s.grid)
    {
        free(s.grid);
    }
}

/* Helper function for schematic_scan_and_sum_valid_parts */
static bool is_valid_symbol(char const c)
{
    return (ispunct(c) && (c != '.')) ? true : false;
}

/* Create a window around a row segment (symbol | part) within the schematic  */
static struct window window_create_around_row_seg(struct schematic const *const s, int const row, int const beg_col, int const end_col)
{
    int const nrows = s->nrows;
    int const ncols = s->ncols;
    struct window w;
    /* Location of window within associated schematic */
    w.from_row = MAX(row - 1, 0);
    w.to_row = MIN(row + 1, nrows - 1);
    w.from_col = MAX(beg_col - 1, 0);
    w.to_col = MIN(end_col + 1, ncols - 1);

    /* Absolute references to the window */
    w.rows = w.to_row - w.from_row + 1; /* no of rows in window */
    w.cols = w.to_col - w.from_col + 1; /* no of cols in window */
    w.used = calloc(w.rows * w.cols, sizeof(bool)); /* set to false */

    /* mark location of segment within window in used array */
    int const seg_size = end_col - beg_col + 1;
    int const seg_row = (row == 0) ? 0 : 1;
    int const seg_beg_col = (beg_col == 0) ? 0 : 1;
    for (int i = 0; i < seg_size; i++)
    {
        w.used[seg_row * w.cols + seg_beg_col + i] = true; 
    }

    return w;
}

static void window_print(struct window const w, struct schematic const *const s)
{
    char const *const sch = s->grid;
    int const ncols = s->ncols;
    
    for( int i = w.from_row; i <= w.to_row; i++)
    {
        for (int j = w.from_col; j <= w.to_col; j++)
        {
            printf("%c", sch[i * ncols + j]);
        }
        printf("\n");
    }
}


static void window_destroy(struct window w)
{
    if (w.used) free(w.used);
}

/* Specifiy the location within the schema of the number by giving it's row number and beg and end column number*/
static bool is_symbol_adjacent(struct schematic const *const s, int const beg_col, int const end_col, int const row)
{
    char const *const sch = s->grid;
    int const ncols = s->ncols;

    struct window w = window_create_around_row_seg(s, row, beg_col, end_col);

    /* Look for symbol */
    for (int i = w.from_row; i <= w.to_row; i++)
    {
        for (int j = w.from_col; j <= w.to_col; j++)
        {
            char c = sch[i * ncols + j];
            if (is_valid_symbol(c))
            {
                return true;
            }
        }
    }
    window_destroy(w);
    return false;
}

/* Helper function for schematic_scan_and_sum_valid_parts */
static int schematic_part_value(struct schematic const *const s, int const beg, int const end, int const row)
{
    char const *const sch = s->grid;
    int const ncols = s->ncols;

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
    int part = atoi(partstr);
    return part;
}

static int schematic_scan_and_sum_valid_parts(struct schematic *s)
{
    char const *const sch = s->grid;
    int const nrows = s->nrows;
    int const ncols = s->ncols;


    bool digit_found = false; /* are we currently scanning a part nummber? */
    int beg_part_col = 0;
    int end_part_col = 0;
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
                    end_part_col = j;
                }
                else
                { /* We have completed scanning a part number or end of the line */
                    if (j == (ncols - 1)) /* end of the line*/
                    {
                        end_part_col = j;
                    }
                    if (is_symbol_adjacent(s, beg_part_col, end_part_col, i))
                    {
                        /* Note part number value is 0 if the part number has already been seen */
                        cumsum += schematic_part_value(s, beg_part_col, end_part_col, i);
                    }
                    digit_found = false;
                    beg_part_col = j; /* reset the beginning digit column */
                    end_part_col = j; /* reset the end digit column */
                }                            /* we have completed scanning a part number */
            }
            else
            { /* we are not scanning a part number */
                if (isdigit(c))
                {
                    /* new potential part number */
                    digit_found = true;
                    beg_part_col = j;
                    end_part_col = j;
                }
                else
                {
                    continue;
                } /* continue scanning the row for the next part number */
            }     /* we are not scanning for a part number */
        }         /* Scan row for part numbers */
        /* after processing each row do the following: */
        beg_part_col = 0; /* reset the beginning digit column for the next row */
        end_part_col = 0; /* reset the end for the next row */
        digit_found = false;     /* reset the search for a part number for the next row */
    }                            /* Scan each row for part numbers */

    return cumsum;
} /* End of scan and sum */ 

/* Helper function for schematic_calc_gear_ratio */
static void update_used(struct window w, struct part const p)
{
    /* find the overlap of the window and the part in the schematic */
    int const from = MAX(p.beg_col, w.from_col);
    int const to = MIN(p.end_col, w.to_col);
    int const size = to - from + 1; /* size of the overlap of part and window */
    int const offset = MAX(0, p.beg_col - w.from_col); /* where the part begins within the window */
    int const r = p.row - w.from_row;
    for (int i = 0; i < size; i++)
    {
        int idx = r * w.cols + i + offset;
        w.used[idx] = true;
    }    
}

/* Helper function for schematic_calc_gear_ratio */
/* Given there is a digit at sch[row * ncols + col] return the corresponding part number */
static struct part schematic_find_part(struct schematic const *const s, int const row, int const col)
{
    char const *const sch = s->grid;
    int const ncols = s->ncols;

    int idx = row * ncols + col;
    struct part p;

    p.row = row;

    p.beg_col = col;
    while (isdigit(sch[idx]) && p.beg_col > 0)
    {       
        p.beg_col--;
        idx = row * ncols + p.beg_col;        
    }
    /* edge case when the part number begins at col 0 */
    if (isdigit(sch[idx])) p.beg_col++;

    idx = row * ncols + col;
    p.end_col = col;
    while (isdigit(sch[idx]) && p.end_col < ncols)
    {       
        p.end_col++;
        idx = row * ncols + p.end_col;        
    }
    if (isdigit(sch[idx])) p.end_col--;

    int len = 0;
    for (int i = p.beg_col; i <= p.end_col; i++)
    {
        char c = sch[row * ncols + i];
        p.partstr[len] = c;
        len++;
    }
    p.partstr[len] = '\0';
    
    p.value =atoi(p.partstr);

    return p;
}

/* Helper function for schematic_scan_and_sum_gear_ratios*/
static int schematic_calc_gear_ratio(struct schematic const *const s, int const row, int const col)
{
    char const *const sch = s->grid;
    int const ncols = s->ncols;

    struct window w = window_create_around_row_seg(s, row, col, col);
//    window_print(w, s);
    struct part part[2]; /* 2 and only 2 parts can be adjacent to the symbol for a valid gear ratio */
    int n_part = 0;

    /* Look for part numbers overlapping the window */
    for (int i = w.from_row; i <= w.to_row; i++)
    {
        for (int j = w.from_col; j <= w.to_col; j++)
        {
            int idx = i * ncols + j;
            int used_idx = (i - w.from_row) * w.cols + (j - w.from_col) ;
            if (n_part > 1)
            {
                break;
            }
            if (isdigit(sch[idx]) || !w.used[used_idx])
            {
                part[n_part] = schematic_find_part(s, i, j);
                update_used(w, part[n_part]);
                n_part++;
            }
            else
            {
                continue;
            }
        }
    }
    
    window_destroy(w);
    if (n_part < 1 || n_part > 1)
    {
        return 0;
    }
   return part[0].value * part[1].value;
}

static int schematic_scan_and_sum_gear_ratios(struct schematic const *const s)
{
    char const *const sch = s->grid;
    int const nrows = s->nrows;
    int const ncols = s->ncols;

    int cumsum = 0;
    for (int i = 0; i < nrows; i++)
    {
        for (int j = 0; j < ncols; j++)
        {
            if (is_valid_symbol(sch[i * ncols + j]))
            {
                cumsum += schematic_calc_gear_ratio(s, i, j);
            }
        }
    }
    return cumsum;
}
