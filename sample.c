#include <stdio.h>
#include <stdlib.h>

#include "source/MouseDetecter.h"

int main(void)
{
	int mouseCount;
	FILE *inputDevicesListFile = fopen("/proc/bus/input/devices", "rb");
	char yesOrNo;

	PrintStartScreen();

	sleep(1);

	if (inputDevicesListFile == NULL)
	{
		perror(
			"opening /proc/bus/input/devices failed! maybe you are not root?");
		putchar('\n');

		exit(EXIT_FAILURE);
	}

	ParseDevicesList(inputDevicesListFile);

	mouseCount = SeekMice();

	PrintMouseInfo(mouseCount);

	DetectMouse(mouseCount);

	puts("Do you want to see current mouse state?(y/n)"
		 "(mouse relative positions and button states)");

	scanf(" %c", &yesOrNo);

	if (yesOrNo == 'y' || yesOrNo == 'Y')
	{
		PrintMouseState();
	}

	fclose(inputDevicesListFile);
}

