#ifndef MOUSE_DETECTER_H
#define MOUSE_DETECTER_H

#include <inttypes.h>
#include <unistd.h>

typedef struct mouse_data
{
	int8_t right;
	uint8_t left;
	uint8_t middle;
	int8_t x;
	int8_t y;
} MouseData;

MouseData BinaryDataToMouseData(uint8_t mouseBinaryData[3]);
MouseData GetAMouseData(FILE *mouse);
void PrintMouseState(void);
void DetectMouse(int mouseCount);
void ParseDevicesList(FILE *inputDevicesListFile);
void PrintMouseInfo(int mouseCount);
void PrintStartScreen(void);
int SeekMice(void);

#endif