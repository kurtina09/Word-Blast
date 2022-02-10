/**************************************************************
* Class:  CSC-415-02 Fall 2021
* Name: Edel Jhon Cenario
* Student ID : 921121224
* GitHub ID : kurtina09
* Project: Assignment 4 – Word Blast
*
* File: cenario_edeljhon_HW4_main.c
*
* Description: This assignment is about creating a program that will read words that are 6 or more characters long from a file. We will be using threads, and each thread will take a chunk of file and process it. 
**************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//Defining my Variables and Structures here --

pthread_mutex_t mutex; //creating my MUTEX here.

typedef struct wordStruct
{                            //Word Structure with the word and frequency.
    char *word;              //The word from txt file
    int frequency;           //Words frequency
} wordStruct, *wordStruct_p; //End of word struct

typedef struct chunkStruct
{
    long start; //Start of file
    long size;  //Size of file
    char *filename;
    int id;
} chunkStruct, *chunkStruct_p;

volatile wordStruct_p wordArray = NULL; //This is my array of words
volatile int wordArrayFreq = 0;         //This is my array of word frequencies
int wordArraySize = 0;                  //Size of my array

//First function -- I am implementing a function that will add word into my word array. I will be using mutex lokc here.
void addWord(char *word)
{ //START OF addWord -----------------------------------------------------------------------------------

    for (int i = 0; i < wordArrayFreq; i++)
    { //Check if the word is already on the list -- If it is, then index + 1.
        if (strcasecmp(wordArray[i].word, word) == 0)
        {                                                        //Check if thee is a difference between comparison, if not then == 0, otherwise != 0.
            pthread_mutex_lock(&mutex);                          //Start of Mutex LOCK -- I am using a mutex lock here because we are dealing with a global array.
            wordArray[i].frequency = wordArray[i].frequency + 1; //Incrementing for the next arg.
            pthread_mutex_unlock(&mutex);                        //End of Mutex UNLOCK.
            return;
        } // End of if -- if there is different - then add word to the list, and set the count to 1.
    }     // End of for -- if the word is not found, then add it to the list.

    pthread_mutex_lock(&mutex); //START OF Mutex LOCK --
    if (wordArrayFreq >= wordArraySize)
    { //Check if there is still room in the array -- if not then reallocate.
        wordArraySize = wordArraySize + 2500;

        if (wordArray == NULL)
        { //If its the first time, then do malloc.

            wordArray = malloc(sizeof(wordStruct) * wordArraySize);

            if (wordArray == NULL)
            { //Check the malloc here -- if it fails then send an error message.
                printf("ERROR: Malloc of WordList has failed. You are trying to allocate %lu bytes.\n", sizeof(wordStruct) * wordArraySize);
                exit(-2);
            }
        }
        else
        { //REALLOC Here
            wordStruct_p reallocRet = realloc(wordArray, sizeof(wordStruct) * wordArraySize);
            if (reallocRet == NULL)
            { //Check the malloc here -- if it fails then send an error message.
                printf("ERROR: Malloc of WordList has failed. You are trying to allocate %lu bytes.\n", sizeof(wordStruct) * wordArraySize);
                exit(-2);
            }
            wordArray = reallocRet;
        }
    }
    wordArray[wordArrayFreq].word = malloc(strlen(word) + 2); //I am adding word to the list here -- Also make sure to check if malloc didn't fail.
    strcpy(wordArray[wordArrayFreq].word, word);
    wordArray[wordArrayFreq].frequency = 1;
    wordArrayFreq++;

    pthread_mutex_unlock(&mutex); //START OF Mutex UNLOCK --

} //End of AddWord ---------------------------------------------------------------------------------------------------------------------------

char *delim = "\"\'."
              "''?:;-,-*($%)! \t\n\x0A\r"; //You may find this Useful

//Second function -- I am implementing a function that will allow threads to process specific chunks of file.
void *processChunk(void *param) //Start of processChunk ------------------------------------------------------------------------------------------
{
    //Defining my Structs and Variables
    chunkStruct_p info = (chunkStruct_p)param; //Here I am casting a pointer to become a Chunk info pointer.
    char *buffer;
    char *word;
    char *tempPtr;
    int fileDesc;

    buffer = malloc(info->size + 4);
    if (buffer == NULL)
    {
        printf("ERROR allocating a buffer in thread %d (%ld bytes) \n", info->id, info->size + 4);
        return NULL;
    }

    fileDesc = open(info->filename, O_RDONLY);
    if (fileDesc == 0)
    { //It is very important part to deallocate the buffer to prevent future issues.
        printf("ERROR: Cannot open %s in thread %d\n", info->filename, info->id);
        free(buffer);
        return NULL;
    }

    lseek(fileDesc, info->start, SEEK_SET);        //Here I am reading from the starting point.
    long res = read(fileDesc, buffer, info->size); //I am only reading a specific amount of bytes given by my variable.
    close(fileDesc);

    buffer[info->size] = 0;     //Notes that my data is being parsed as a large string -- Null termination can be important.
    buffer[info->size + 1] = 0; //By doing this, I am essentially protecting my buffer.

    word = strtok_r(buffer, delim, &tempPtr); //I am getting the first word here. -- NOTE that I used strtok_r instead of strtok. I think that strtok_r is more safer.

    while (word != NULL)
    {
        if (strlen(word) >= 6)
        {
            addWord(word);
        }
        word = strtok_r(NULL, delim, &tempPtr); //Here I am getting all the subsequent words.
    }

    free(buffer);
    return NULL;
} //End of processChunk ------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    //***TO DO***  Look at arguments, open file, divide by threads
    //             Allocate and Initialize and storage structures
    //Defining my variables here.
    chunkStruct *infoArray;
    int processIndex;
    int fileDesc;
    char *filename;
    pthread_t *pt;
    filename = "WarAndPeace.txt"; //Defining here the default file name, unless otherwise specified.

    processIndex = 1;

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (argc > 2)
    {
        filename = argv[1];
        processIndex = atoi(argv[2]); //In here I am getting the number of threads that we need to create. I am using atoi to convert it from CString to Int.
    }

    fileDesc = open(filename, O_RDONLY);

    lseek(fileDesc, 0, SEEK_END);
    long fileSize = lseek(fileDesc, 0, SEEK_CUR); // I am getting the file size here.
    lseek(fileDesc, 0, SEEK_SET);                 // needed for next read from beginning of file
    close(fileDesc);

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************
    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish

    infoArray = malloc(sizeof(chunkStruct) * processIndex); //I am allocating Chunk struct depending on how many processes we have.
    pt = malloc(sizeof(pthread_t) * processIndex);          //I am creating an array of thread process IDs here.

    for (int i = 0; i < processIndex; i++)
    { //For the process here -- First we initialize our parameters.

        infoArray[i].filename = filename;
        infoArray[i].id = i + 1;
        infoArray[i].start = (fileSize / processIndex) * i;
        //NOTE that some will have their own version of the structure. -- Instead of everyone having the same structure that is being overwritten.

        if (i == processIndex - 1) //Make sure that all bytes are accounter for. 
        {
            infoArray[i].size = fileSize - infoArray[i].start;
        }
        else
        {
            infoArray[i].size = (fileSize / processIndex);
        }

        pthread_create(&(pt[i]), NULL, processChunk, &(infoArray[i])); //I am creating threads here. 
    }

    for (int i = 0; i < processIndex; i++)
    {
        pthread_join(pt[i], NULL); //Per the requirements -- We need to wait for the threads to finish. 
    }

    // ***TO DO *** Process TOP 10 and display
    wordStruct top10[10]; //I am finding the top 10 here. 
    for (int j = 0; j < 10; j++)
    {
        top10[j].frequency = 0;
    }

    for (int j = 0; j < wordArrayFreq; j++)
    {
        if (wordArray[j].frequency > top10[10 - 1].frequency)
        { 
            top10[10 - 1].word = wordArray[j].word;
            top10[10 - 1].frequency = wordArray[j].frequency;
            for (int k = 10 - 2; k >= 0; k--)
            {
                if (wordArray[j].frequency > top10[k].frequency)
                {
                    top10[k + 1].word = top10[k].word;
                    top10[k + 1].frequency = top10[k].frequency;
                    top10[k].word = wordArray[j].word;
                    top10[k].frequency = wordArray[j].frequency;
                }
            }
        }
    }

    printf("\n\nWord Frequency Count on %s with %d threads\n", filename, processIndex); //Printing headers
    printf("Printing top %d words %d characters or more. \n", 10, 6);
    for (int k = 0; k < 10; k++)
    {
        printf("Number %d is %s with a count of %d\n", k + 1, top10[k].word, top10[k].frequency);
    }

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
    {
        --sec;
        n_sec = n_sec + 1000000000L;
    }

    printf("Total Time Was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************

    //MOST IMPORTANT PART OF CODE -- CLEAN AFTER YOURSELF. 
    for (int j = 0; j < wordArrayFreq; j++)
    {
        free(wordArray[j].word);
        wordArray[j].word = NULL;
    }
    //I AM FREEING EVERYTHING HERE
    free(wordArray); 
    wordArray = NULL; //Precaution to set the array to null after deleting its content. 
    pthread_mutex_destroy(&mutex);
}
