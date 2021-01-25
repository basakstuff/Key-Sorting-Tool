#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Json.h"

#define STUDENT_COUNT 10

enum TYPES { BIN, CHR, UTF };

#pragma warning(disable:4996) // for visual studio warning


// POD for Json file
typedef struct Info
{
	char* dataFileName;
	char* indexFileName;
	int recordLength;
	char* keyEncoding;
	int keyStart;
	int keyEnd;
	char* order;
}Info;

// POD for Student
typedef struct _Student 
{
	char ID[10];
	char name[22];
	char lastname[22];
	char phone[15];
}Student;

// POD for Index
typedef struct _Index 
{
	char name[22];
	int index;
	
}Index;

// Global Variables
Info info;
FILE* inFile = 0;
char* buffer = 0;
char name[22];

// It opens the json file and reads the given settings.
// Then it reads the whole index file in to memory and waits for the futher commands.
char* open(const char* fileName)
{
	long numbytes;

	inFile = fopen(fileName, "r");
 
	if(!inFile)
		return 1;
 
	fseek(inFile, 0L, SEEK_END);
	numbytes = ftell(inFile);
	fseek(inFile, 0L, SEEK_SET);	
	buffer = (char*)calloc(numbytes, sizeof(char));

	if(!buffer)
		return 1;

	fread(buffer, sizeof(char), numbytes, inFile);
}

// utility ordering function for ASC
int compare_studentsASC(const void* a, const void* b)
{ 
	return ((Index*)a)->index > ((Index*)b)->index; 
}

// utility ordering function for DESC
int compare_studentsDESC(const void* a, const void* b)
{ 
	return ((Index*)a)->index < ((Index*)b)->index; 
}

// utility string function
void generate_random_string(char* target, char* dictionary, int maxLength, int minLength)
{
	int length = minLength + rand() % (maxLength-minLength);
	int dictLength = strlen(dictionary);

	for(int i = 0; i < length; i++)
	{
		int dictIdx = rand() % dictLength;
		target[i] = toupper(dictionary[dictIdx]);
	}
		
	target[length] = '\0';
}

// generate new random data file
void generate_random_data_file()
{
	FILE* fp = fopen("dataa.dat", "wb");
	if(!fp)
	{
		printf("%s", "Could not open data file!");
		return;
	}

	Student student[STUDENT_COUNT];

	srand(time(NULL));
	for(int i = 0; i < STUDENT_COUNT - 1; i++)
	{
		generate_random_string(student[i].ID,"0123456789", 10, 9);
		generate_random_string(student[i].name, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 22, 5);
		generate_random_string(student[i].lastname, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 22, 5);
		generate_random_string(student[i].phone, "0123456789", 15, 10);
	}

	// to test binary search, we create an specific student has a name that AAAAAAAAA
	generate_random_string(student[STUDENT_COUNT-1].ID, "0123456789", 10, 9);
	generate_random_string(student[STUDENT_COUNT-1].name, "A", 10, 9);
	generate_random_string(student[STUDENT_COUNT-1].lastname, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 10, 9);
	generate_random_string(student[STUDENT_COUNT-1].phone, "0123456789", 10, 9);
				
	fseek(fp, 0, SEEK_SET);
	fwrite(student, sizeof(Student), STUDENT_COUNT, fp);
	fclose(fp);
}

// read data file and create index file
void create_index()
{
	FILE* fp = fopen("dataa.dat", "rb");
	if(!fp)
	{
		printf("%s", "Could not open data file!");
		return;
	}

	int i = 0;
    Index indexes[STUDENT_COUNT];
	Student student;
	fseek(fp, 0, SEEK_SET);
    fread(&student, sizeof(Student), 1, fp);

    while(!feof(fp)) 
	{
        strcpy(indexes[i].name, student.name);
		indexes[i].index = i;
		i++;
        fread(&student, sizeof(Student), 1, fp);
    }

	if (strcmp(info.order, "ASC") == 0)
		qsort(indexes, STUDENT_COUNT, sizeof(Index), compare_studentsASC);

	if (strcmp(info.order, "DEC") == 0)
		qsort(indexes, STUDENT_COUNT, sizeof(Index), compare_studentsDESC);

	FILE* fp1 = fopen(info.indexFileName, "wb");
	if(!fp1)
	{
		printf("%s", "Could not open the index file!");
		return;
	}

	fseek(fp1, 0, SEEK_SET);
	fwrite(indexes, sizeof(Index), STUDENT_COUNT, fp1);

	fclose(fp1);
	fclose(fp);
}

// binary searching by name
int search(FILE* fp, const char name1[22], int firstIdx, int lastIdx)
{
	Index first, last, middle;
	int middleIdx = (firstIdx + lastIdx) / 2;

	fseek(fp, firstIdx * (sizeof(Index)), SEEK_SET);
	fread(&first, sizeof(Index), 1, fp);

	if(strcmp(first.name, name1) == 0)
	{ 
		return first.index;
	}

	fseek(fp, lastIdx * sizeof(Index), SEEK_SET);
	fread(&last, sizeof(Index), 1, fp);
	if(strcmp(last.name, name1) == 0)
	{
		return last.index;
	}

	if(middleIdx == firstIdx || middleIdx == lastIdx) 
	{
		return -1;
	}

	fseek(fp, middleIdx * sizeof(Index), SEEK_SET);
	fread(&middle, sizeof(Index), 1, fp);
	if(strcmp(middle.name, name1) == 0)
	{
		return middle.index;
	} 
	else if (strcmp(middle.name, name1) > 0) 
	{
		return search(fp, name1, firstIdx + 1, middleIdx - 1);
	} 
	else 
	{
		return search(fp, name1, middleIdx + 1, lastIdx - 1);
	}
}

int find_student_by_name(char name1[22]) 
{
    FILE* inputFile = fopen(info.indexFileName, "rb");
	if (!inputFile)
	{
		printf("%s", "Could not open the index file.");
		return -1;
	}

    fseek(inputFile, STUDENT_COUNT * sizeof(Index), SEEK_SET);
    int lastIdx = (ftell(inputFile) / sizeof(Index)) - 1;
    int result = search(inputFile, name1, 0, lastIdx);

    fclose(inputFile);
    return result;
}

// print firstName by index
void firstName(int index)
{	
	FILE* inputFile = fopen(info.indexFileName, "rb");


	if (!inputFile)
	{
		printf("%s", "Could not open the index file.");
		return 0;
	}

	fseek(inputFile, 0, SEEK_SET);
	char name[22];

	for(int i = 0; i < STUDENT_COUNT; i++)
    {
		Index bufferPerson;
		fread(&bufferPerson,sizeof(Index),1,inputFile);
		if (index == bufferPerson.index)
			strcpy(name, bufferPerson.name);
	}

	printf("%s", name);
}

// print studentID by index
void studentID(int index)
{
	FILE* inputFile = fopen(info.indexFileName, "rb");
	fseek(inputFile, 0, SEEK_SET);
	char name[22];

	for(int i = 0; i < STUDENT_COUNT; i++)
    {
		Index bufferPerson;
		fread(&bufferPerson, sizeof(Index), 1, inputFile);
		if (index == bufferPerson.index)
			strcpy(name, bufferPerson.name);
		
	}
	fclose(inputFile);

	FILE* inputFile2 = fopen("dataa.dat","rb");
	fseek(inputFile2, 0, SEEK_SET);    
	for(int i = 0; i < STUDENT_COUNT; i++)
    {
		Student bufferPerson;
		fread(&bufferPerson, sizeof(Student), 1, inputFile);
		if (strcmp(name, bufferPerson.name) == 0)
		{
			printf("%s", bufferPerson.ID);
		}
	}

	fclose(inputFile2);
	return 0;
}

// print lastName by index
void lastName(int index)
{
	FILE* inputFile = fopen(info.indexFileName, "rb");
	fseek(inputFile, 0, SEEK_SET);
	char name[22];

	for(int i = 0; i < STUDENT_COUNT; i++)
    {
		Index bufferPerson;
		fread(&bufferPerson, sizeof(Index), 1, inputFile);
		if (index == bufferPerson.index)
			strcpy(name, bufferPerson.name);
		
	}
	fclose(inputFile);

	FILE* inputFile2 = fopen("dataa.dat","rb");
	fseek(inputFile2, 0, SEEK_SET);    
	for(int i = 0; i < STUDENT_COUNT; i++)
    {
		Student bufferPerson;
		fread(&bufferPerson, sizeof(Student), 1, inputFile);
		if (strcmp(name, bufferPerson.name) == 0)
		{
			//fclose(inputFile);
			printf("%s", bufferPerson.lastname);
		}
	}

	fclose(inputFile2);
	return 0;
}

// print phone by index
void phone(int index)
{
	FILE* inputFile = fopen(info.indexFileName, "rb");
	fseek(inputFile, 0, SEEK_SET);
	char name[22];

	for(int i = 0; i < STUDENT_COUNT; i++)
    {
		Index bufferPerson;
		fread(&bufferPerson, sizeof(Index), 1, inputFile);
		if (index == bufferPerson.index)
			strcpy(name, bufferPerson.name);
		
	}
	fclose(inputFile);

	FILE* inputFile2 = fopen("dataa.dat","rb");
	fseek(inputFile2, 0, SEEK_SET);    
	for(int i = 0; i < STUDENT_COUNT; i++)
    {
		Student bufferPerson;
		fread(&bufferPerson, sizeof(Student), 1, inputFile);
		if (strcmp(name, bufferPerson.name) == 0)
		{
			//fclose(inputFile);
			printf("%s", bufferPerson.phone);
		}
	}

	fclose(inputFile2);
	return 0;
}

// print data file to console
void print_data_file()
{
	printf("Student Number: %d\n", STUDENT_COUNT);
	FILE* inputFile = fopen("dataa.dat", "rb");
	if (!inputFile)
	{
		printf("%s", "Could not open the data file.");
		return;
	}

	fseek(inputFile, 0, SEEK_SET); 

	for(int i = 0; i < STUDENT_COUNT; i++)
    {
		Student tempStudent;
		fread(&tempStudent, sizeof(Student), 1, inputFile);

		printf("[datafile] id is %s, name is %s, lastname is %s, phone is:%s \n", tempStudent.ID, tempStudent.name, tempStudent.lastname, tempStudent.phone);
	}

	fclose(inputFile);
}

// print index file to console
void print_index_file()
{
	printf("Student Number: %d\n", STUDENT_COUNT);
	FILE* inputFile = fopen(info.indexFileName, "rb");
	if (!inputFile)
	{
		printf("%s", "Could not open the index file.");
		return;
	}

	fseek(inputFile, 0, SEEK_SET); 

	for(int i = 0 ; i < STUDENT_COUNT; i++)
    {
		Index tempStudent;
		fread(&tempStudent, sizeof(Index), 1, inputFile);
		printf("[indexfile] name is %s, index is %d \n", tempStudent.name, tempStudent.index);
		
	}

	fclose(inputFile);
}

// free the resources
void close()
{
	free(buffer);
	fclose(inFile);
}

int main()
{
	// open and load json file
	open("test.json");
	char* someJsonString = buffer;
	JSONObject *json = parseJSON(someJsonString);
	info.dataFileName = json->pairs[0].value->stringValue;
	info.indexFileName = json->pairs[1].value->stringValue;
	info.recordLength = atoi(json->pairs[2].value->stringValue);
	info.keyEncoding = json->pairs[3].value->stringValue;
	info.keyStart = atoi(json->pairs[4].value->stringValue);
	info.keyEnd = atoi(json->pairs[5].value->stringValue);
	info.order = json->pairs[6].value->stringValue;

	// print json file
	printf("%s", "Printing json data...\n");
	printf("%s \n", json->pairs[0].value->stringValue);
	printf("%s \n", json->pairs[1].value->stringValue);
	printf("%s \n", json->pairs[2].value->stringValue);
	printf("%s \n", json->pairs[3].value->stringValue);
	printf("%s \n", json->pairs[4].value->stringValue);
	printf("%s \n", json->pairs[5].value->stringValue);
	printf("%s \n", json->pairs[6].value->stringValue);
	printf("\n\n\n");
	
	generate_random_data_file();

	create_index();

	printf("%s", "Printing data file...\n");
	print_data_file();

	printf("\n\n\n");

	printf("%s", "Printing index file...\n");
	print_index_file();

	printf("\n\n\n");

	if (find_student_by_name("AAAAAAAAA") != -1)
		printf("index was found %d", find_student_by_name("AAAAAAAAA"));
	else
		printf("index not found!");
	
	printf("\n\n\n");

	firstName(5);

	printf("\n\n\n");

	lastName(5);

	printf("\n\n\n");

	phone(5);

	close();

	getchar();
}