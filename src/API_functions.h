#pragma once

#include "pch.h"

#define NET_BUFFER_SIZE (16384)
#define MARKERS_SIZE 12

#define ID_SIZE 4
#define DWNLD_TOKEN_SIZE 252

#define FIRING (0)
#define LIGHTING (1)
#define LFD_S (2)

enum EVENT_ID {
	GET_POSITIONS_TOKEN = 0,
	GET_POSITIONS_MODPACK
};

enum MOD_ID {
	TRJ_FREE = 0,
	POS_FREE,
	TRJ_WE,
	POS_FREE_MODPACK,
	TRJ_FREE_DEMO
};

typedef struct _map {
	uint8_t sections_count = 0; //now it is 6 anywhere but I do that field for future updates of structure
	uint8_t minimap_count  = 0;

	//types of positions sections

	std::vector<float*> firing;
	std::vector<float*> lighting;
	std::vector<float*> LFD;

	size_t getTotalCount() {
		return firing.size() + lighting.size() + LFD.size();
	}
} map;

extern unsigned char response_buffer[NET_BUFFER_SIZE + 1];
extern size_t response_size;

uint8_t curl_init();

void parse_config(map*, bool, bool, bool);

bool file_exists(const char*);

uint8_t send_token(uint32_t, uint8_t, uint8_t);