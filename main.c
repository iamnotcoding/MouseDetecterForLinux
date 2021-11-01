#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_ARR_SIZE 100

struct arg_struct1
{
	char devpath[MAX_ARR_SIZE];
	size_t mouseNum;
	size_t mouseID;
};

struct mouse_data
{
	uint8_t right;
	uint8_t left;
	uint8_t middle;
	uint8_t x;
	uint8_t y;
};

pthread_t g_threads[MAX_ARR_SIZE];

size_t g_devicesNum;
size_t g_dectectedmouseNum;

char g_devicesInfo[MAX_ARR_SIZE][MAX_ARR_SIZE][MAX_ARR_SIZE];
char g_dectectedmousePath[MAX_ARR_SIZE];
int g_mousnumList[MAX_ARR_SIZE];

struct mouse_data BinaryDataToMouseData(uint8_t mouse_binary_data[3])
{
	struct mouse_data mouse_data;

	mouse_data.left = mouse_binary_data[0] & 0x1;
	mouse_data.right = mouse_binary_data[0] & 0x2;
	mouse_data.middle = mouse_binary_data[0] & 0x4;

	mouse_data.x = mouse_binary_data[1];
	mouse_data.y = mouse_binary_data[2];

	return mouse_data;
}

void PrintMouseState(void)
{
	struct mouse_data mouse_data;
	uint8_t mouse_binary_data[3];
	char errorStr[200];

	printf("reading data form %s... please move the mouse(touch pad may not "
		   "work properly)\n\n",
		   g_dectectedmousePath);

	FILE *mouse = fopen(g_dectectedmousePath, "rb");

	if (mouse == NULL)
	{
		sprintf(errorStr, "reading %s failed", g_dectectedmousePath);
		perror(errorStr);
		putchar('\n');

		exit(EXIT_FAILURE);
	}

	while (!feof(mouse))
	{
		fread(mouse_binary_data, sizeof(uint8_t) * 3, 1, mouse);
		mouse_data = BinaryDataToMouseData(mouse_binary_data);

		// prints current mouse coordinate and mouse button state
		fprintf(stderr,
				"x : "
				"%" PRIu8 " y : "
				"%" PRIu8 " left = %s right = %s middle = %s\n",
				mouse_data.x, mouse_data.x, mouse_data.left ? "true" : "false",
				mouse_data.right ? "true" : "false",
				mouse_data.middle ? "true" : "false");
	}
}

void *CleanThread(pthread_t thread[], size_t mouseNum, size_t excluededThread)
{
	if (g_dectectedmouseNum != SIZE_MAX)
	{
		for (size_t i = 0; i < mouseNum; i++)
		{
			if (i != excluededThread)
			{
				printf("killing thread %zu...\n", i);

				pthread_cancel(thread[i]);
			}
		}
	}

	return NULL;
}

void *IsInputChanges(void *args)
{
	const char *devpath = (*(struct arg_struct1 *)args).devpath;
	size_t mouseNum = (*(struct arg_struct1 *)args).mouseNum;
	size_t mouseID = (*(struct arg_struct1 *)args).mouseID;
	uint8_t temp;
	char errorStr[200];
	FILE *dev = fopen(devpath, "rb");

	if (dev == NULL)
	{ 
		sprintf(errorStr, "opening %s failed maybe you are not root?", devpath);
		perror(errorStr);
		putchar('\n');

		pthread_exit(NULL);
	}

	fread(&temp, sizeof temp, 1, dev);

	fclose(dev);

	printf("\nmouse %zu : %s detected!\n\n", mouseID, devpath);

	g_dectectedmouseNum = mouseNum;

	strcpy(g_dectectedmousePath, devpath);

	CleanThread(g_threads, mouseNum, mouseID);

	pthread_exit(NULL);
}

void DetectMouse(size_t mouseNum)
{
	char *token;
	char devpath[MAX_ARR_SIZE];

	struct arg_struct1 temp_arg1_struct;

	puts("-------------------------");
	puts("Please move a mouse");

	sleep(1);

	for (size_t i = 0; i < mouseNum; i++)
	{
		for (size_t j = 0;
			 g_devicesInfo[g_mousnumList[i]][j][0] != '\n' && j < MAX_ARR_SIZE;
			 j++)
		{
			token = strtok(g_devicesInfo[g_mousnumList[i]][j], "=");
			token = strtok(NULL, "=");

			if (token != NULL)
			{
				if (strstr(token, "mouse") || strstr(token, "mice"))
				{
					// make structure
					temp_arg1_struct.mouseNum = mouseNum;
					temp_arg1_struct.mouseID = i;

					// make abosolute path
					token = strtok(token, " ");
					sprintf(devpath, "/dev/input/%s", token);
					strcpy(temp_arg1_struct.devpath, devpath);

					/* creates multiple thread for each input streams.
					if you don't use multithread, you cannot detect input
					streams changes simultaneously. So if you don't use
					multithread, it probably not be able to detect mice
					properly.*/

					pthread_create(&g_threads[i], NULL, IsInputChanges,  &temp_arg1_struct);
				}
			}
		}
	}

	for (size_t i = 0; i < mouseNum; i++)
	{
		pthread_join(g_threads[i], NULL);
	}
}

void PrintMouseInfo(size_t mouseNum)
{
	printf("%zu mice(mouse) found!\n\n", mouseNum);

	for (size_t i = 0; i < mouseNum; i++)
	{
		printf("number %zu :\n", i + 1);

		for (size_t j = 0; g_devicesInfo[i][j][0] != '\n'; j++)
		{
			usleep(10000);

			printf("%s", g_devicesInfo[g_mousnumList[i]][j]);
		}

		putchar('\n');
	}
}

size_t SeekMouse(void)
{
	size_t k = 0;

	for (size_t i = 0; i < g_devicesNum; i++)
	{
		for (size_t j = 0; j < g_devicesNum; j++)
		{
			if (strstr(g_devicesInfo[i][j], "mouse") ||
				strstr(g_devicesInfo[i][j], "mice"))
			{
				g_mousnumList[k++] = i;
			}
		}
	}

	return k;
}

void ParseDevicesList(FILE *inputdevicelistFile)
{
	for (size_t i = 0; i < MAX_ARR_SIZE && !feof(inputdevicelistFile); i++)
	{
		for (size_t j = 0; j < MAX_ARR_SIZE && !feof(inputdevicelistFile); j++)
		{
			fgets(g_devicesInfo[i][j], MAX_ARR_SIZE, inputdevicelistFile);

			if (g_devicesInfo[i][j][0] == '\n')
			{
				break;
			}
		}

		g_devicesNum = i;
	}
}

void PrintStartScreen(void)
{
	puts("***********************");
	puts("*** mouse detecter ***");
	puts("***********************");
	putchar('\n');
}

int main(void)
{
	size_t mouseNum;
	FILE *inputdevicelistFile = fopen("/proc/bus/input/devices", "rb");
	char yesOrNo;

	PrintStartScreen();

	sleep(1);

	if (inputdevicelistFile == NULL)
	{
		perror("opening /proc/bus/input/devices failed! maybe you are not root?");
		putchar('\n');
		
		exit(EXIT_FAILURE);
	}

	ParseDevicesList(inputdevicelistFile);

	mouseNum = SeekMouse();

	PrintMouseInfo(mouseNum);

	DetectMouse(mouseNum);

	puts("Do you want to see current mouse state?(y/n)"
		 "(mouse coordinate and button state)");

	scanf(" %c", &yesOrNo);

	if (yesOrNo == 'y' || yesOrNo == 'Y')
	{
		PrintMouseState();
	}

	fclose(inputdevicelistFile);
}
