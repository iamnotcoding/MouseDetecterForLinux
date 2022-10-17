#define _GNU_SOURCE

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct IsInputChangesArg_t
{
	char devPath[50];
	int mouseIndex;
	int mouseCount;
} IsInputChangesArg;

typedef struct mouse_data
{
	int8_t right;
	uint8_t left;
	uint8_t middle;
	int8_t x;
	int8_t y;
} MouseData;

pthread_t g_threads[50];

int g_deviceCount;
int g_dectectedMouseIndex = -1;

char g_deviceInfo[100][100][100];
char g_dectectedMousePath[50];
int g_mouseIndexesList[50];

static MouseData BinaryDataToMouseData(uint8_t mouseBinaryData[3])
{
	MouseData mouseData;

	mouseData.left = mouseBinaryData[0] & 0x1;
	mouseData.right = mouseBinaryData[0] & 0x2;
	mouseData.middle = mouseBinaryData[0] & 0x4;

	mouseData.x = (mouseBinaryData[0] & 0x10) ? mouseBinaryData[1] - 256
											  : mouseBinaryData[1];
	mouseData.y = (mouseBinaryData[0] & 0x20) ? -(mouseBinaryData[2] - 256)
											  : -mouseBinaryData[2];

	return mouseData;
}

MouseData GetAMouseData(void)
{
	uint8_t mouseBinaryData[3];
	FILE *mouse = fopen(g_dectectedMousePath, "rb");

	fread(mouseBinaryData, sizeof(uint8_t) * 3, 1, mouse);
	fclose(mouse);
	
	return BinaryDataToMouseData(mouseBinaryData);
}

void PrintMouseState(void)
{
	FILE *mouse = fopen(g_dectectedMousePath, "rb");
	char errorStr[100];

	if (mouse == NULL)
	{
		sprintf(errorStr, "reading %s failed", g_dectectedMousePath);
		perror(errorStr);
		putchar('\n');

		exit(EXIT_FAILURE);
	}

	// you'll have to press ^C to exit
	while (true)
	{
		MouseData mouseData = GetAMouseData();

		// prints current relative mouse coordinate and mouse button state
		fprintf(stderr,
				"x : "
				"%" PRId8 " y : "
				"%" PRId8 " left = %s right = %s middle = %s\n",
				mouseData.x, mouseData.y, mouseData.left ? "true" : "false",
				mouseData.right ? "true" : "false",
				mouseData.middle ? "true" : "false");
	}
}

static void *CleanThread(pthread_t thread[], int mouseCount,
						 int excluededThread)
{
	if (g_dectectedMouseIndex != -1)
	{
		for (int i = 0; i < mouseCount; i++)
		{
			if (i != excluededThread)
			{
				printf("killing thread %d...\n", i + 1);

				pthread_cancel(thread[i]);
			}
		}
	}

	return NULL;
}

static void *IsInputChanges(void *arg)
{
	IsInputChangesArg *args = arg;
	uint8_t temp;
	char errorStr[200];
	FILE *dev = fopen(args->devPath, "rb");

	if (dev == NULL)
	{
		sprintf(errorStr, "opening %s failed maybe you are not root?",
				args->devPath);
		perror(errorStr);

		putchar('\n');

		return NULL;
	}

	fread(&temp, sizeof temp, 1, dev);

	fclose(dev);

	printf("\nmouse number %d : %s detected!\n\n", args->mouseIndex + 1,
		   args->devPath);

	g_dectectedMouseIndex = args->mouseIndex;

	strcpy(g_dectectedMousePath, args->devPath);

	CleanThread(g_threads, args->mouseCount, args->mouseIndex);

	return NULL;
}

void DetectMouse(int mouseCount)
{
	char *token;
	char devPath[50];

	IsInputChangesArg *tempArg[50];

	puts("-------------------------");
	puts("Please move a mouse");

	sleep(1);

	for (int i = 0; i < mouseCount; i++)
	{
		for (int j = 0;
			 g_deviceInfo[g_mouseIndexesList[i]][j][0] != '\n' && j < 50; j++)
		{
			token = strtok(g_deviceInfo[g_mouseIndexesList[i]][j], "=");
			token = strtok(NULL, "=");

			if (token != NULL)
			{
				if (strstr(token, "mouse") || strstr(token, "mice"))
				{
					// make structure
					tempArg[i] = malloc(sizeof(IsInputChangesArg));
					tempArg[i]->mouseIndex = i;
					tempArg[i]->mouseCount = mouseCount;

					// make abosolute path
					token = strtok(token, " ");
					sprintf(devPath, "/dev/input/%s", token);
					strcpy(tempArg[i]->devPath, devPath);

					pthread_create(&g_threads[i], NULL, IsInputChanges,
								   tempArg[i]);
				}
			}
		}
	}

	for (int i = 0; i < mouseCount; i++)
	{
		pthread_join(g_threads[i], NULL);
	}

	for (int i = 0; i < mouseCount; i++)
	{
		free(tempArg[i]);
	}
}

void PrintMouseInfo(int mouseCount)
{
	printf("%d mice(mouse) found!\n\n", mouseCount);

	for (int i = 0; i < mouseCount; i++)
	{
		printf("number %d :\n", i + 1);

		for (int j = 0; g_deviceInfo[i][j][0] != '\n'; j++)
		{
			usleep(10000);

			printf("%s", g_deviceInfo[g_mouseIndexesList[i]][j]);
		}

		putchar('\n');
	}
}

int SeekMice(void)
{
	int mouseCount = 0;

	for (int i = 0; i < g_deviceCount; i++)
	{
		for (int j = 0; j < g_deviceCount; j++)
		{
			if (strstr(g_deviceInfo[i][j], "mouse") ||
				strstr(g_deviceInfo[i][j], "mice"))
			{
				g_mouseIndexesList[mouseCount++] = i;
			}
		}
	}

	return mouseCount;
}

void ParseDevicesList(FILE *inputDevicesListFile)
{
	for (int i = 0; i < 50 && !feof(inputDevicesListFile); i++)
	{
		for (int j = 0; j < 50 && !feof(inputDevicesListFile); j++)
		{
			fgets(g_deviceInfo[i][j], 50, inputDevicesListFile);

			if (g_deviceInfo[i][j][0] == '\n')
			{
				break;
			}
		}

		g_deviceCount = i;
	}
}

void PrintStartScreen(void)
{
	puts("***********************");
	puts("*** mouse detecter ***");
	puts("***********************");
	putchar('\n');
}
