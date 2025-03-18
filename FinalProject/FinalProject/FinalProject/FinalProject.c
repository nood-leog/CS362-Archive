//FinalProject.c
//Alex Boyce

//Determine the frequencies of title-cased words (proper nouns and sentence starters) in a large English text file, such as a book.
//Use mpiexec -n 4 .\FinalProject.exe book.txt
//Use mpiexec -n 4 .\FinalProject.exe grapesofwrath.txt


#define _CRT_SECURE_NO_WARNINGS 
#define MAX_WORD_LEN 100 //maximum length of a word

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>

//struct for a word and its count
typedef struct 
{
    char word[MAX_WORD_LEN];
    int count;
}WordCount;

//dynamic array for storing word counts
typedef struct 
{
    WordCount* data;
    int size;
    int capacity;
} WordCountArray;


//function prototypes 
void initWordCountArray(WordCountArray* arr);
void freeWordCountArray(WordCountArray* arr);
void addWord(WordCountArray* arr, const char* word);
int isTitleCase(const char* word);
void processBuffer(char* buffer, WordCountArray* wcArr);

int main(int argc, char* argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (argc < 2) //if no filename is provided then print usage and exit
    {
        if (rank == 0)
            fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

	char* filename = argv[1]; //get the filename from the command line arguments
	MPI_File fh; //file handle
	MPI_Offset file_size; //size of the file
	MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh); //open the file
	MPI_File_get_size(fh, &file_size); //get the size of the file

    //divide the file into chunks
    MPI_Offset local_size = file_size / size;
    MPI_Offset start = rank * local_size;
    //ensure the last process gets the remainder
    MPI_Offset end = (rank == size - 1) ? file_size : start + local_size;

    //allocate a buffer for the local chunk and add extra room to capture a word that might go over the boundary
    int extra = 100;
    int buf_size = (int)(end - start) + extra + 1;
	char* buffer = malloc(buf_size); //allocate memory for the buffer
	if (!buffer)  //check for memory allocation error
    {
        fprintf(stderr, "Memory allocation error\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
	memset(buffer, 0, buf_size); //initialize the buffer to zero


    //read the local chunk using MPI I/O
    MPI_File_read_at(fh, start, buffer, (int)(end - start), MPI_CHAR, MPI_STATUS_IGNORE);

    //for non-last processes, read extra characters to complete any cut-off word
    if (rank != size - 1)
    {
        MPI_File_read_at(fh, end, buffer + (end - start), extra, MPI_CHAR, MPI_STATUS_IGNORE);
    }
	MPI_File_close(&fh); //close the file

    //if not the first process then adjust start so that we begin at a word boundary
    if (rank != 0)
    {
        char* ptr = buffer;
        while (*ptr && !isspace((unsigned char)*ptr))
            ptr++;
        memmove(buffer, ptr, strlen(ptr) + 1);
    }

    //process the buffer to count title-cased words
    WordCountArray localCounts;
    initWordCountArray(&localCounts);
    processBuffer(buffer, &localCounts);

    //each process prints its results
    printf("Process %d found %d title-cased words:\n", rank, localCounts.size);
    for (int i = 0; i < localCounts.size; i++)
    {
        printf("  %s: %d\n", localCounts.data[i].word, localCounts.data[i].count);
    }

	free(buffer);//free the buffer
	freeWordCountArray(&localCounts); //free the word count array

	MPI_Finalize(); //finalize MPI
    return 0;
}


//initialize wordcountarray
void initWordCountArray(WordCountArray* arr) 
{
    arr->size = 0;
    arr->capacity = 100;
    arr->data = malloc(arr->capacity * sizeof(WordCount));
    if (!arr->data) 
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
}

//free memory used by wordcountarray
void freeWordCountArray(WordCountArray* arr) 
{
    free(arr->data);
}

//add a word to the array or update its count if it already exists
void addWord(WordCountArray* arr, const char* word) 
{
	for (int i = 0; i < arr->size; i++) //iterate through the array to find if the word already exists
    {
		if (strcmp(arr->data[i].word, word) == 0) //if the word exists then increment its count
        {
            arr->data[i].count++;
            return;
        }
    }
	if (arr->size == arr->capacity) //if the array is full then double its capacity
    {
		arr->capacity *= 2; //double the capacity
		arr->data = realloc(arr->data, arr->capacity * sizeof(WordCount)); //reallocate memory to the new size
		if (!arr->data) //check for memory allocation error
        {
            fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }
    }
	strncpy(arr->data[arr->size].word, word, MAX_WORD_LEN - 1); //copy the word to the array
	arr->data[arr->size].word[MAX_WORD_LEN - 1] = '\0'; //ensure null terminator is present
	arr->data[arr->size].count = 1; //set the count to 1
	arr->size++; //increment the size of the array
}

//check if a word has first letter uppercase rest of the letters cabn be lowercase
int isTitleCase(const char* word)
{
    if (!word || word[0] == '\0') //check if the word is empty
    {
        return 0;
    }
	if (!isupper((unsigned char)word[0])) //check if the first letter is uppercase
    {
    return 0;
    }

	for (int i = 1; word[i] != '\0'; i++) //iterate through the rest of the word
    {
		if (!islower((unsigned char)word[i])) //check if the rest of the letters are lowercase
        {
            return 0;
        } 
    }
    return 1;
}

//process a buffer by tokenizing on common delimiters
void processBuffer(char* buffer, WordCountArray* wcArr) 
{
    char* token = strtok(buffer, " \t\r\n,.;:!?\"()[]{}<>");
    while (token != NULL) 
    {
        if (isTitleCase(token)) 
        {
            addWord(wcArr, token);
        }
        token = strtok(NULL, " \t\r\n,.;:!?\"()[]{}<>");
    }
}