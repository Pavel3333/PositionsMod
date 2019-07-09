#pragma once
#include "iostream"
#include <fstream>
#include <vector>
#include <windows.h>

#define NET_BUFFER_SIZE (16384)
#define MARKERS_SIZE 12

#define ID_SIZE 4
#define DWNLD_TOKEN_SIZE 252

#define FIRING (0)
#define LIGHTING (1)
#define LFD_S (2)

typedef struct {
	uint8_t GET_POSITIONS_TOKEN   = 0;
	uint8_t GET_POSITIONS_MODPACK = 1;
} EVENT_ID;

typedef struct {
	uint8_t TRJ_FREE         = 0;
	uint8_t POS_FREE         = 1;
	uint8_t TRJ_WE           = 2;
	uint8_t POS_FREE_MODPACK = 3;
	uint8_t TRJ_FREE_DEMO    = 4;
} MODS_ID;

typedef struct {
	uint8_t sections_count = NULL; //now it is 6 anywhere but I do that field for future updates of structure
	uint8_t minimap_count = NULL;

	//types of positions sections

	std::vector<float*> firing;
	std::vector<float*> lighting;
	std::vector<float*> LFD;
} map;

extern unsigned char response_buffer[NET_BUFFER_SIZE + 1];
extern size_t response_size;

uint8_t curl_init();

void parse_config(map*, bool, bool, bool);

bool file_exists(const char*);

uint8_t send_token(uint32_t, uint8_t, uint8_t);