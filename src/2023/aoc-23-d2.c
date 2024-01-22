/*  -*- mode: C -*- */
/* This file conforms to C99 */

/*
  Build Instructions:
  PATH=../build/:${PATH}
  clang -std=c99 -Wall -Wextra aoc-23-d2.c  -O3 -g -o ../build/aoc-23-d2

  Program written for the Advent of Code day 2 2023
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* No of Cubes per specification */
#define NO_OF_RED 12 
#define NO_OF_GREEN 13  
#define NO_OF_BLUE 14
#define RESULT_FILENAME "../../data/aoc-d2.dat"
#define MAX_SETS 7 /* use: awk -F\;  '{ print NF }' ../data/aoc-d2.dat| sort | tail -n 1 */
#define MAX_LINE_LENGTH 200 /* awk -F\\n  '{ print length }' ../data/aoc-d2.dat | uniq | sort */

typedef struct
{
    int red;
    int green;
    int blue;
} GameSet;

typedef struct
{
    int id;
    GameSet results[MAX_SETS];
} GameRecord;

typedef struct GameElem
{    
    GameRecord *rec;
    struct GameElem *next;
} GameElem;

static int scan_line(FILE *f, GameRecord *rec);
static GameSet set_create_from_string(char const *setstr);
static void set_print(GameSet set);
static void game_list_print(GameElem *list);
static GameSet game_minimal_set(GameRecord const * const rec);
static int set_power(GameSet set);

int main(void)
{
    char const fname[] = RESULT_FILENAME;
    FILE *f = fopen(fname, "r");
    int res;
    GameElem *game_list = NULL;
    GameElem *iter = NULL;
    bool first = true;
    do
    {
        GameRecord *rec = malloc(sizeof(GameRecord)); /* allocate record */
        res = scan_line(f, rec); /* Fill record from the database file */
        if (res != EOF) /* add record to the game list */
        {
            GameElem *el = malloc(sizeof(GameElem));
            if (first)
            {
                game_list = el;
                first = false;
                iter = game_list; /* create and initialise list iterator */
                el->rec = rec;
                el->next = NULL;
            }
            else
                {
                    el->rec = rec;
                    el->next = NULL;
                    iter->next = el;
                    iter = iter->next;
                }
        }
        else
            {
                break;
            }
    }
    while (true); /* The loop is terminated from within the body on reaching EOF  */

    /* Print records */
    // game_list_print(game_list);    

    /* Iterate over game_list and see if the minimal set of the game is within the specification bounds */
    GameSet minimal;
    int cumsum = 0;
    int powersum = 0;
    iter = game_list;
    while (iter) /* while more games in gamelist */
    {
        minimal = game_minimal_set(iter->rec);
        if (minimal.red <= NO_OF_RED && minimal.green <= NO_OF_GREEN && minimal.blue <= NO_OF_BLUE)
        {
            cumsum += iter->rec->id;
        }
        powersum += set_power(minimal);
        iter = iter->next;
    }
    printf("The sum of the possible game ids is: %i\n", cumsum);
    printf("The cumulative power of the minimal games is %i\n", powersum);
    /* Destroy records */
    iter = game_list;
    while(iter) /* while more elements in game list */
    {
        if (iter->rec)
        {
            free(iter->rec); /* Destroy the element's record */
        }
        GameElem *prev_iter = iter;
        iter = iter->next; /* move to the next container */
        free(prev_iter); /* Destroy the current list element */
    }

    return EXIT_SUCCESS;
}

static int scan_line(FILE *f, GameRecord *rec )
{
    char line[MAX_LINE_LENGTH];
    char garbage[12];
    int res = fscanf(f, "%s %i:%[^\n]\n", garbage, &(rec->id), line);
    if (res == EOF) return res;    
    int i =  0;
    char sep[2] = ";";
    char *setstr, *brkt;
    for (setstr = strtok_r(line, sep, &brkt); setstr; setstr = strtok_r(NULL, sep, &brkt))
    {
        rec->results[i] = set_create_from_string(setstr);
        i++;
    }
    return res;
}

static GameSet set_create_from_string(char const *setstr)
{
    GameSet set = {.red = 0, .blue = 0, .green = 0};
    char str[MAX_LINE_LENGTH];
    strcpy(str, setstr);
    int count = 0;
    char col[6]; /* red, green, or blue */
    char sep[2] = ",";
    char *brkt;
    for (char *setstr2  = strtok_r(str, sep, &brkt); setstr2; setstr2 = strtok_r(NULL, sep, &brkt))
    {
        sscanf(setstr2, "%i %s,", &count, col); 
        if (!strcmp("red", col))
        {
            if (count > set.red)
            {
                set.red = count;
            }
        }
        else if (!strcmp("blue", col))
        {
            if (count > set.blue)
            {
                set.blue = count;
            }            
        }
        else if (!strcmp("green", col))
        {
            if (count > set.green)
            {
                set.green = count;
            }            
        }
    }
    return set;
}
static void set_print(GameSet set)
{
    if (set.red)
    {
        printf("%i red", set.red);
        if (set.blue || set.green)
        {
            printf("%c ", ',');
            
        }
    }

    if (set.green)
    {
        printf("%i green", set.green);
        if (set.blue)
        {
            printf("%c ", ',');
        }
    }
    if (set.blue)
    {
        printf("%i blue", set.blue);
    }
}

static void game_print(GameRecord *rec)
{
    bool first = true;
    printf("Game %i: ", rec->id);
    for (int i = 0; i < MAX_SETS; i++)
    {
        GameSet set = rec->results[i];
        if (set.red > 0 || set.blue > 0 || set.red > 0)
        {
            if (!first)
            {
                printf("; "); /* semicolon only printed at beginning of second through last set */
            }
            else
                {
                    first = false;
                }
            set_print(set);   
        }
    }
    printf("\n");
}

static void game_list_print (GameElem *list)
{
    GameElem *iter = list;
    while (iter != NULL)
    {
        game_print(iter->rec);
        iter = iter->next;
    }
}

/* return a set representing the maximum value of each colour of ball seen in any set of that game */
static GameSet game_minimal_set(GameRecord const * const rec)
{
    GameSet max_set = {0, 0, 0};
    for (int i = 0; i < MAX_SETS; i++)
    {
        if (max_set.red  < rec->results[i].red)
        {
            max_set.red = rec->results[i].red;
        }
        if (max_set.green  < rec->results[i].green)
        {
            max_set.green = rec->results[i].green;
        }
        if (max_set.blue  < rec->results[i].blue)
        {
            max_set.blue = rec->results[i].blue;
        }
    }
    return max_set;
}

static int set_power(GameSet set)
{
    return set.red * set.blue * set.green;
}
