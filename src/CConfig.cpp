#include "CConfig.h"

// Инициализация конфига

char* Config::ids       = MOD_NAME;
char* Config::author    = "by Pavel3333 & RAINN VOD";
char* Config::version   = MOD_VERSION;
char* Config::patch     = PATCH;

uint16_t Config::version_id = 105;

buttons_c Config::buttons = buttons_c();
data_c    Config::data    = data_c();
i18n_c    Config::i18n    = i18n_c();
