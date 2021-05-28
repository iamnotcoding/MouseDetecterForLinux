#include "stdheaders.h"

#define MAX_ARR_SIZE 100

struct arg_struct1
{
	char devpath[MAX_ARR_SIZE];
	size_t mouseNum;
};

struct mouse_data
{
	uint8_t right;
	uint8_t left;
	uint8_t middle;
	uint8_t x;
	uint8_t y;
};

char g_devicesInfo[MAX_ARR_SIZE][MAX_ARR_SIZE][MAX_ARR_SIZE] = {0};
int g_mousnumList[MAX_ARR_SIZE] = {0};
size_t g_devicesNum;
size_t g_dectectedmouseNum = -1;
char g_dectectedmousePath[MAX_ARR_SIZE];
pthread_t g_threads[MAX_ARR_SIZE];

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

	printf("reading data form %s... please move the mouse\n\n", g_dectectedmousePath);

	FILE *mouse = fopen(g_dectectedmousePath, "rb");

	if (mouse == NULL)
	{
		fprintf(stderr, "reading %s failed", g_dectectedmousePath);
		exit(EXIT_FAILURE);
	}

	sleep(1);

	while (!feof(mouse))
	{
		fread(mouse_binary_data, sizeof(uint8_t) * 3, 1, mouse);
		mouse_data = BinaryDataToMouseData(mouse_binary_data);

		fprintf(stderr, "x : "
						"%" PRIu8 " y : "
						"%" PRIu8 " left = %s right = %s middle = %s\n",
				mouse_data.x, mouse_data.x, mouse_data.left ? "true" : "false", mouse_data.right ? "true" : "false", mouse_data.middle ? "true" : "false");
	}
}

void *CleanThread(pthread_t thread[], size_t mouseNum)
{
	while (true)
	{
		sleep(1);

		if (g_dectectedmouseNum != -1)
		{
			for (size_t i = 0; i < mouseNum; ++i)
			{
				pthread_cancel(thread[i]);
			}
		}
	}
}

void *IsInputChanges(void *args)
{
	const char *devpath = (*(struct arg_struct1 *)args).devpath;
	size_t mouseNum = (*(struct arg_struct1 *)args).mouseNum;
	uint8_t temp;
	FILE *dev = fopen(devpath, "rb");

	if (dev == NULL)
	{
		fprintf(stderr, "opening %s failed maybe you are not root?\n", devpath);
		pthread_exit(NULL);
	}

	fread(&temp, sizeof temp, 1, dev);
	fclose(dev);

	if (temp != 0)
	{
		printf("mouse : %s detected!\n", devpath);

		g_dectectedmouseNum = mouseNum;
		strcpy(g_dectectedmousePath, devpath);
		CleanThread(g_threads, mouseNum);
	}

	pthread_exit(NULL);
}

size_t DetectMouse(size_t mouseNum)
{
	int status, *pstatus;
	char *token;
	char devpath[MAX_ARR_SIZE];

	struct arg_struct1 *temp_arg1_structs[MAX_ARR_SIZE];

	puts("-------------------------");
	puts("Please move a mouse");

	for (size_t i = 0; i < mouseNum; ++i)
	{
		for (size_t j = 0; g_devicesInfo[g_mousnumList[i]][j][0] != '\n' && j < MAX_ARR_SIZE; ++j)
		{
			token = strtok(g_devicesInfo[g_mousnumList[i]][j], "=");
			token = strtok(NULL, "=");

			if (token != NULL)
			{
				if (strstr(token, "mouse") || strstr(token, "mice"))
				{
					pstatus = &status;
					temp_arg1_structs[i] = malloc(sizeof(struct arg_struct1));
					temp_arg1_structs[i]->mouseNum = mouseNum;

					token = strtok(token, " ");
					sprintf(devpath, "/dev/input/%s", token);
					strcpy(temp_arg1_structs[i]->devpath, devpath);

					pthread_create(&g_threads[i], NULL, IsInputChanges, temp_arg1_structs[i]);
				}
			}
		}
	}

	for (size_t i = 0; i < mouseNum; ++i)
	{
		pthread_join(g_threads[i], (void **)&pstatus);
	}

	for(size_t i = 0; i < mouseNum; ++i)
			free(temp_arg1_structs[i]);
}

size_t PrintMouseInfo(size_t mouseNum)
{
	printf("%zu mice found!\n", mouseNum);

	for (size_t i = 0; i < mouseNum; ++i)
	{
		printf("number %zu :\n", i + 1);
		for (size_t j = 0; g_devicesInfo[i][j][0] != '\n'; ++j)
		{
			printf("%s", g_devicesInfo[g_mousnumList[i]][j]);
		}
	}
}

size_t SeekMouse(FILE *inputdevicelistFile)
{
	size_t k = 0;

	for (size_t i = 0; i < g_devicesNum; ++i)
	{
		for (size_t j = 0; j < g_devicesNum; ++j)
		{
			if (strstr(g_devicesInfo[i][j], "mouse") || strstr(g_devicesInfo[i][j], "mice"))
			{
				g_mousnumList[k++] = i;
			}
		}
	}

	return k;
}

void ParseDevicesList(FILE *inputdevicelistFile)
{
	for (size_t i = 0; i < MAX_ARR_SIZE && !feof(inputdevicelistFile); ++i)
	{
		for (size_t j = 0; j < MAX_ARR_SIZE && !feof(inputdevicelistFile); ++j)
		{
			fgets(g_devicesInfo[i][j], MAX_ARR_SIZE, inputdevicelistFile);

			if (g_devicesInfo[i][j][0] == '\n')
				break;
		}
		g_devicesNum = i;
	}
}

int main(void)
{
	size_t mouseNum;
	FILE *inputdevicelistFile = fopen("/proc/bus/input/devices", "rb");

	if (inputdevicelistFile == NULL)
	{
		fprintf(stderr, "opening /proc/bus/input/devices failed! maybe you are not root?");
		exit(EXIT_FAILURE);
	}

	ParseDevicesList(inputdevicelistFile);
	mouseNum = SeekMouse(inputdevicelistFile);
	PrintMouseInfo(mouseNum);
	DetectMouse(mouseNum);
	PrintMouseState();
	
	fclose(inputdevicelistFile);
}
