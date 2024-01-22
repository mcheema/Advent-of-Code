/*  -*- mode: C -*- */
/* This file conforms to C99 */

/*
  Build Instructions:
  PATH=../build/:${PATH}
  clang -std=c99 -Wall -Wextra aoc-23-d1.c  -O3 -g -o ../build/aoc-23-d1

  Program written for the Advent of Code day 1 2023
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#define VALUE(digit) ((digit) - (int) '0') /* Convert the digit ascii code to the digit's value  */

#ifdef TEST
char const fname[] = "../../data/tmp.dat";
#else
char const fname[] = "../../data/aoc-23-d1.txt";
#endif

char *numbers[] = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
static bool match_str_from_file_maybe(FILE *f, char const *str);
static int match_num_maybe(FILE *f, int c);
static bool is_first_char_of_digit_name(char c);

int main(void)
{
    FILE *f = fopen(fname, "r");
    int *p = malloc(sizeof(int));
    long sum = 0;

    /* the scan state variables: must be reset to these values at the beginning of each line */
    int first = 0;
    int last = 0;
    bool isFirst = true;

    /* scan line to deterine the first and last integer */
    do
    {
        while ((*p = fgetc(f)) != '\n') /* keep scanning until end of line reached */
        {
            if (*p == EOF) /* Check for end of file and terminate this loop */
            {            
                break;
            }
            else if (isalpha(*p))
            {
                *p = match_num_maybe(f, (char) *p); /* try to match a number in word form */                
            }

            if (isdigit(*p)) /* at this point p is either a number or the next character of a non number word */
            {
                if (isFirst)
                {
                    first = *p - '0';
                    last = first;
                    isFirst = false;
                }
                else
                    {
                        last = *p - '0';    
                    }
            }
        } /* Keep scanning characters until newline reached. EOF is handled specially within the loop */

        /* Compute the running total for all lines scanned so far */
        sum += (first * 10 + last);

        if (*p == EOF) /* Check for EOF and terminate this loop */
        {
            break;            
        }

        /* reset the scan state variables */
        isFirst = true;
        first = 0;
        last = 0;
    }
    while (true); /* Keep scanning lines until end of file reached */

    /* Print the result */
    printf("The sum is %li\n", sum);
    if (p) free(p);
    return EXIT_SUCCESS;
}

/*
  Try match string str, with the corresponding character's in the stream.
  If the match fails rewind the stream to remain unchanged from when the
  function was invoked.
*/
static bool match_str_from_file_maybe(FILE *f, char const *str)
{
    int const len = strlen(str);
    char history[6] = "";
    char c;
    for (int i = 0; i < len; i++) { /* match str with characters from stream f */
        c = fgetc(f);
        history[i] = c;
        if (isalpha(c))
        {
            c = tolower(c);
        }

        if (c != str[i])
        { /* case: no match found */
            for (int j = i; j >= 0; j--) /* rewind */
            {
                ungetc(history[j], f);
            }
            return false;
        } /* no match */
    } /* match characters loop */
    /* match found */
    if (is_first_char_of_digit_name(c)) 
    {
        ungetc(c, f); /* Could be the start of digit name */
    }

    return true; /* case: match found */
}

/* Match a number written in characters and return the corresponding character code, 
   or return the next character from the file
 */

static int match_num_maybe(FILE *f, int c)
{
    c = tolower(c);
    switch (c)
    {
        case 'o':
        if (match_str_from_file_maybe(f, "ne"))
        {
            return '1';
        }
        break;
        case 't':
        if (match_str_from_file_maybe(f, "wo"))
        {
            return '2';
        }
        else if (match_str_from_file_maybe(f, "hree"))
        {
            return '3';
        }
        break;
        case 'f':
        if (match_str_from_file_maybe(f, "our"))
        {
            return '4';
        }
        if (match_str_from_file_maybe(f, "ive"))
        {
            return '5';
        }
        break;
        case 's':
        if (match_str_from_file_maybe(f, "ix"))
        {
            return '6';
        }
        else if (match_str_from_file_maybe(f, "even"))
        {
            return '7';
        }
        break;
        case 'e':
        if (match_str_from_file_maybe(f, "ight"))
        {
            return '8';
        }
        break;
        case 'n':
        if (match_str_from_file_maybe(f, "ine"))
        {
            return '9';
        }
        break;
        default:
        return c;
    }
    return c;
}

static bool is_first_char_of_digit_name(char c)
{
    switch(tolower(c))
    {
        case 'e':
        case 'f':
        case 'o':
        case 'n':
        case 's':
        case 't':
        return true;
        break;
        default:
        break;
    }
    return false;
}
