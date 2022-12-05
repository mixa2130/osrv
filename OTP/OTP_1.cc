#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <fstream>
#include <fcntl.h>

#define SUCCESS 0
#define ERROR_CREATE_THREAD -11
#define ERROR_BARRIER -12
#define ERROR_BARRIER_DESTROY -13
#define ERROR_WAIT_BARRIER -14
#define ERROR_JOIN_THREAD -15
#define ERROR_FILE_OPEN -16
#define ERROR_FILE -17

//pthread_barrier_t barrier;

struct programParam{
	size_t a;
	size_t c;
	size_t m;
	size_t seed;
	char* inputFilePath;
	char* outputFilePath;
};

struct keygenParam{
	size_t a;
	size_t c;
	size_t m;
	size_t seed;
	size_t sizeKey;
};

struct cryptParam
{
	char* msg;
	char* key;
	char* outputText;
	size_t size;
	size_t downIndex;
	size_t topIndex;
	pthread_barrier_t* barrier;
};

void* keyGenerate(void* params){
	keygenParam *parametrs = reinterpret_cast<keygenParam *>(params);
	size_t a = parametrs->a;
	size_t m = parametrs->m;
	size_t c = parametrs->c;
	size_t sizeKey = parametrs->sizeKey;
	char* key = new char[sizeKey];
	int* buff = new int[sizeKey/sizeof(int) + 1];
	buff[0] = parametrs->seed;

	for(size_t i = 1; i < sizeKey/sizeof(int) + 1 ; i++){
		buff[i]= (a * buff[i-1] + c) % m;
	}
	memcpy(key,buff,sizeKey);
	return key;
};

void* crypt(void * cryptParametrs)
{
	int status = 0;

	cryptParam* param = reinterpret_cast<cryptParam*>(cryptParametrs);
	size_t topIndex = param->topIndex;
	size_t downIndex = param->downIndex;

	while(downIndex < topIndex)
		{
			param->outputText[downIndex] = param->key[downIndex] ^ param->msg[downIndex];
			downIndex++;
		}

	status = pthread_barrier_wait(param->barrier);
		if(status != PTHREAD_BARRIER_SERIAL_THREAD && status != 0)
			{
			std::cout << "problem with pthread_barrier_wait";
			exit(ERROR_WAIT_BARRIER);
			}
	}

int main (int argc, char **argv) {
	int c;
	programParam progParam;
	while ((c = getopt(argc, argv, "i:o:a:c:x:m:")) != -1) {
		switch (c) {
		case 'i':
			printf ("option i with value '%s'\n", optarg);
			progParam.inputFilePath = optarg;
			break;
		case 'o':
			printf ("option o with value '%s'\n", optarg);
			progParam.outputFilePath = optarg;
			break;
		case 'a':
			printf ("option a with value '%s'\n", optarg);
			progParam.a = atoi(optarg);
			break;
		case 'c':
			printf ("option c with value '%s'\n", optarg);
			progParam.c = atoi(optarg);
			break;
		case 'm':
			printf ("option m with value '%s'\n", optarg);
			progParam.m = atoi(optarg);
			break;
		case 'x':
			printf ("option x with value '%s'\n", optarg);
			progParam.seed = atoi(optarg);
			break;
		case '?':
			break;
		default:
			printf ("?? getopt returned character code 0%o ??\n", c);
		}
	}
	if (optind < argc) {
		printf ("non-option ARGV-elements: ");
		while (optind < argc)
			printf ("%s ", argv[optind++]);
		printf ("\n");
	}

	int num_thread = sysconf(_SC_NPROCESSORS_ONLN)+1;
	int inputFile = open(progParam.inputFilePath, O_RDONLY);

	if (inputFile == -1)
	{
		std::cerr << "error with file 123 ";
		exit(ERROR_FILE);
	}

	int inputSize = lseek(inputFile, 0, SEEK_END);
	std::cout<<"input size = "<<inputSize<<std::endl;

	if(inputSize == -1)
	{
		std::cout << "error with file ";
		exit(ERROR_FILE);
	}

	char* key = new char[inputSize];
	char* outputText = new char[inputSize];
	char* msg = new char[inputSize];

	if(lseek(inputFile, 0, SEEK_SET) == -1)
	{
		std::cout << "error with file ";
		exit(ERROR_FILE);
	}

	inputSize = read(inputFile, msg, inputSize);

	if(inputSize == -1)
	{
		std::cout << "error with file ";
		exit(ERROR_FILE);
	}

	keygenParam keyParam;
	keyParam.sizeKey = inputSize ;
	keyParam.a=progParam.a;
	keyParam.c=progParam.c;
	keyParam.m=progParam.m;
	keyParam.seed=progParam.seed;

	pthread_t keyGenThread;
	pthread_t cryptThread[num_thread];
	int status = 0;

	if(pthread_create(&keyGenThread, NULL, keyGenerate, &keyParam) != 0)
	{
		std::cout << "error with pthread_create()";
		exit(ERROR_CREATE_THREAD);
	}
	if(pthread_join(keyGenThread, (void**)&key) != 0)
	{
		std::cout << "error with pthread_join()";
		exit(ERROR_JOIN_THREAD);
	}

	pthread_barrier_t barrier;

	status = pthread_barrier_init(&barrier, NULL, num_thread);

	if(status != 0)
	{
		std::cout << "error with pthread_barrier_init()";
		exit(ERROR_BARRIER);
	}


	for(int i = 0; i < num_thread - 1; i++)
	{
		cryptParam* cryptParametrs = new cryptParam;

		cryptParametrs->key = key;
		cryptParametrs->size = inputSize;
		cryptParametrs->outputText = outputText;
		cryptParametrs->msg = msg;
		cryptParametrs->barrier = &barrier;
		if(i == 0)
			cryptParametrs->downIndex = 0;
		else
			cryptParametrs->downIndex = inputSize / ((num_thread - 1) * i);
		if(i == (num_thread - 2))
			cryptParametrs->topIndex = inputSize;
		else
			cryptParametrs->topIndex = inputSize / ((num_thread - 1) * (i + 1));

		pthread_create(&cryptThread[i], NULL, crypt, cryptParametrs);
	}
	status = pthread_barrier_wait(&barrier);



	int output;
	if ((output=open(progParam.outputFilePath, O_WRONLY))==-1) {
		printf ("Cannot open file.\n");
		exit(1);
	}
	if(write(output, outputText, inputSize) !=inputSize)
		printf("Write Error");
	close(output);

	delete[] key;
	delete[] outputText;
	delete[] msg;

	return SUCCESS;
}
