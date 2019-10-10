#pragma once

#include <cstdint>

#if !PATCH_9_22
#define PATCH       "1.6.1.0"
#define MOD_VERSION "v1.0.1.2 (" __TIMESTAMP__ ") - patch " PATCH
#else
#define PATCH       "0.9.22.0.1"
#define MOD_VERSION "v1.0.1.1 (" __TIMESTAMP__ ") - patch " PATCH
#endif

struct data_c {
	uint16_t version = 0; //version_id
	bool enabled             = true;
	bool showOnStartBattle   = true;
	bool showBattleGreetings = true;
	bool createMarkers       = true;
	bool createLighting      = true;
	bool createFiring        = true;
	bool createLFD           = true;
	bool playAnimation       = true;
	bool newModels           = true;
	bool hideMarkersInBattle = false;
	uint16_t buttonShow = 0;    //buttons.buttonShow
	uint16_t buttonMinimap = 0; //buttons.buttonMinimap
};

struct buttons_c {
	uint8_t buttonShow    = 24U; //Keys.KEY_O
	uint8_t buttonMinimap = 38U; //Keys.KEY_L
};

struct i18n_c {
	uint16_t version = 0;
};


class Config {
public:
	static char* ids;
	static char* author;
	static char* version;
	static char* patch;
	static uint16_t version_id;
	static buttons_c buttons;
	static data_c data;
	static i18n_c i18n;
};
