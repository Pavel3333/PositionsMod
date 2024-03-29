﻿#include "API_functions.h"

#include "CConfig.h"

#include "MyLogger.h"

#include <io.h>
#include <sstream>
#include "curl/curl.h"

unsigned char response_buffer[NET_BUFFER_SIZE + 1];
size_t response_size = 0;

static CURL* curl_handle = nullptr;

//---------------------------------------------API functions--------------------------------------------------------

bool file_exists(const char *fname) { 
	return _access(fname, 0) != -1;
}

//generate random bytes
void generate_random_bytes(unsigned char* out, size_t length) { 
	unsigned char ret = 0;
	for (size_t i = 0; i < length; i++) {
		do {
			ret = rand() % 256;
		} while (ret == '"' || ret == '\t' || ret == '\n');

		out[i] = ret;
	}
}

//writing response from server into array ptr and return size of response
static size_t write_data(char *ptr, size_t size, size_t nmemb, char* data){ 
	if (!data || response_size + size * nmemb > NET_BUFFER_SIZE)
		return 0; // Error if out of buffer

	memcpy(&data[response_size], ptr, size*nmemb);// appending data into the end
	response_size += size * nmemb;  // changing position
	return size * nmemb;
}

//get license key hash by current path
static bool get_key(unsigned char* out, uint32_t id) {
	char* token_name = "res_mods/mods/xfw_packages/shared/token";
	char* token_id_name = "res_mods/mods/xfw_packages/shared/token_%d";

	std::streamoff size;

	std::ifstream file(token_name, std::ios::binary);

	char* file_renamed_id = new char[64];

	sprintf_s(file_renamed_id, 64, token_id_name, id ^ 0xAC4B2F7C);

	if (file.is_open()) {  //путь token есть
		std::ifstream token_old(file_renamed_id, std::ios::binary);

		if (token_old.is_open()) {  //есть переименованный файл
			token_old.close();

			std::ofstream token_old_rewrite(file_renamed_id, std::ios::binary);

			//считываем размер файла с токеном

			file.seekg(0, std::ios::end);
			size = file.tellg(); // getting file size
			file.seekg(0);

			if (size != DWNLD_TOKEN_SIZE) {  
				delete[] file_renamed_id;

				token_old_rewrite.close();
				file.close();

				return false;
			}

			file.read((char*)out, DWNLD_TOKEN_SIZE); //читаем

			token_old_rewrite.write((char*)out, DWNLD_TOKEN_SIZE); //перезаписываем

			token_old_rewrite.close();
			file.close();

			std::remove(token_name); //удаляем token

			delete[] file_renamed_id;

			return true;
		}
		else {                      //нет переименованного файла
			token_old.close();
			file.close();

			uint32_t rename_result = rename(token_name, file_renamed_id); // переименовываем

			if (rename_result) { 
				delete[] file_renamed_id;

				return false;
			}
		}
	}
	else { 
		file.close();
	}

	std::ifstream file_renamed(file_renamed_id, std::ios::binary);

	memset(file_renamed_id, 0, 63);

	delete[] file_renamed_id;

	if (file_renamed.is_open()) {  // путь token_ID есть
		file_renamed.seekg(0, std::ios::end);
		size = file_renamed.tellg(); // getting file size
		file_renamed.seekg(0);

		if (size != DWNLD_TOKEN_SIZE) { 
			file_renamed.close();

			return false;
		}
		file_renamed.read((char*)out, DWNLD_TOKEN_SIZE); // читаем
		file_renamed.close();

		return true;
	}

	memset(out, '5', DWNLD_TOKEN_SIZE); //на случай, если юзер вообще без токена

	return true;
}

//---------------------------------------------------------------------------------------------------------------

//------------------------------------------CLIENT-SERVER PART---------------------------------------------------

//Vigenere encoding/decoding
static void vigenere(unsigned char* input, unsigned char* output, size_t size, unsigned char* key, size_t key_size, bool encoding) {
	for (size_t i = 0; i < size; i++) {
		output[i] = encoding 
			? input[i] + key[i % key_size]
			: input[i] - key[i % key_size];
	}
}

uint8_t curl_init() {
	//инициализация curl

	if (curl_global_init(CURL_GLOBAL_ALL)) {
		return 1;
	}

	curl_handle = curl_easy_init();

	if (!curl_handle) {
		return 2;
	}

	return 0;
}

//sending responce
static uint8_t send_to_server(std::string_view request)
{
	if (!curl_handle)
		return 1;

	memset(response_buffer, 0, NET_BUFFER_SIZE);
	response_size = 0;

	// Build an HTTP form with a single field named "request"
	curl_mime*     mime = curl_mime_init(curl_handle);
	curl_mimepart* part = curl_mime_addpart(mime);
	curl_mime_data(part, request.data(), request.size());
	curl_mime_name(part, "request");

	// Post and send it. */
	curl_easy_setopt(curl_handle, CURLOPT_MIMEPOST, mime);

	// setting url
	curl_easy_setopt(curl_handle, CURLOPT_URL, "https://api.pavel3333.ru/downloaded_2/index.php");

	// setting user agent
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, MOD_NAME);

	// setting CA cert path
	curl_easy_setopt(curl_handle, CURLOPT_CAPATH, "/");

	// setting CA cert info
	curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "./res_mods/mods/xfw_packages/" MOD_NAME "/certs/cacert.pem");

	// setting function for write data
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

	// setting buffer
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, response_buffer);

	// requesting
	CURLcode res = curl_easy_perform(curl_handle);

	// always cleanup
	////curl_easy_cleanup(curl_handle);
	curl_mime_free(mime);

	return static_cast<uint8_t>(res);
}

void parse_config(map* curr_map, bool createLighting, bool createFiring, bool createLFD) { 

	//подчищаем память от старых карт

	curr_map->firing.clear();
	curr_map->lightning.clear();
	curr_map->LFD.clear();

	//-------------------------------

	if (response_size < 3) { 
		return ;
	}

	uint32_t offset = 0;
	uint16_t length = 0;

	memcpy(&length, response_buffer, 2);
	if (length != response_size) { 
		return ;
	}

	offset += 2;

	char* key = "DIO4Gb941mfOiHox6jLntKn6kqfgopFX1xaCu1JWlb3ag";

	vigenere(response_buffer + offset, response_buffer + offset, length - 2, (unsigned char*)key, 45, false);

	curr_map->sections_count = response_buffer[offset];

	uint8_t firing_count   = response_buffer[offset + 1];
	uint8_t lighting_count = response_buffer[offset + 2];
	uint8_t LFD_count      = response_buffer[offset + 3];

	offset += 4;

	if (firing_count && createFiring) { 
		curr_map->firing.resize(firing_count);

		for (uint8_t i = 0; i < firing_count; i++) {
			curr_map->firing[i] = new float[3];

			for (uint8_t j = 0; j < 3; j++) {
				curr_map->firing[i][j] = 0.0;
			}
		}
	}

	if (lighting_count && createLighting) { 
		curr_map->lightning.resize(lighting_count);

		for (uint8_t i = 0; i < lighting_count; i++) {
			curr_map->lightning[i] = new float[3];

			for (uint8_t j = 0; j < 3; j++) {
				curr_map->lightning[i][j] = 0.0;
			}
		}
	}

	if (LFD_count && createLFD) { 
		curr_map->LFD.resize(LFD_count);

		for (uint8_t i = 0; i < LFD_count; i++) {
			curr_map->LFD[i] = new float[3];

			for (uint8_t j = 0; j < 3; j++) {
				curr_map->LFD[i][j] = 0.0;
			}
		}
	}

	uint8_t section_num = 0;

	for (uint8_t i = 0; i < curr_map->sections_count; i++) { 
		section_num = response_buffer[offset];
		offset++;

		if (section_num == FIRING) { 
			for (auto &it : curr_map->firing) {
				for (uint8_t k = 0; k < 3; k++) {
					if (createFiring)
						memcpy(&(it[k]), response_buffer + offset, 4);

					offset += 4;
				}
			}
		}
		else if (section_num == LIGHTING) { 
			for (auto &it : curr_map->lightning) {
				for (uint8_t k = 0; k < 3; k++) {
					if (createLighting)
						memcpy(&(it[k]), response_buffer + offset, 4);

					offset += 4;
				}
			}
		}
		else if (section_num == LFD_S) { 
			for (auto &it : curr_map->LFD) {
				for (uint8_t k = 0; k < 3; k++) {
					if (createLFD)
						memcpy(&(it[k]), response_buffer + offset, 4);

					offset += 4;
				}
			}
		}
	}
}

uint8_t send_token(uint32_t id, uint8_t map_id, EVENT_ID event_id) {
	unsigned char* token = nullptr;

	uint16_t size = 0;

	//Код наполнения токена по типу события

	if (event_id == EVENT_ID::GET_POSITIONS_MODPACK) {
		size = 273;

		char key[17] = "Piranhas ModPack";

		//-----------------------Generating rubbish section---------------------------------------------

		const uint8_t  rubbish_size[7] = { 141, 6, 74, 15, 11, 4, 3 };
		unsigned char* rubbish[7] = { 0 };

		for (size_t i = 0; i < 7; i++) {
			rubbish[i] = new unsigned char[rubbish_size[i] + 1];
			generate_random_bytes(rubbish[i], rubbish_size[i]);
		}

		//-----------------------------------------------------------------------------------------------

		token = new unsigned char[size + 1];

		token[0] = MOD_ID::POS_FREE_MODPACK; //mod

		token[1] = map_id;                  //map ID

		memcpy(&token[2], rubbish[0], rubbish_size[0]);
		delete[] rubbish[0];

		memcpy(&token[143], rubbish[1], rubbish_size[1]);
		delete[] rubbish[1];

		memcpy(&token[149], rubbish[2], rubbish_size[2]);
		delete[] rubbish[2];

		memcpy(&token[223], rubbish[3], rubbish_size[3]);
		delete[] rubbish[3];

		memcpy(&token[238], rubbish[4], rubbish_size[4]);
		delete[] rubbish[4];

		memcpy(&token[249], rubbish[5], rubbish_size[5]);
		delete[] rubbish[5];

		memcpy(&token[253], key, 16);

		memcpy(&token[269], rubbish[6], rubbish_size[6]);
		delete[] rubbish[6];

		//-----------------------------------------------------------------------------------------------
	}
	else if (event_id == EVENT_ID::GET_POSITIONS_TOKEN) {
		size = 512;

		unsigned char* key_ = new unsigned char[DWNLD_TOKEN_SIZE + 1];

		if (!get_key(key_, id)) {
			delete[] key_;

			return 2;
		}

		unsigned char* key = new unsigned char[DWNLD_TOKEN_SIZE + 4 + 1];
		memcpy(key, key_, 41);
		memcpy(&key[41], &id, 4);
		memcpy(&key[41 + 4], &key_[41], 211);

		for (uint16_t i = 0; i < DWNLD_TOKEN_SIZE + 4; i++) {
			key[i] ^= 0xeb;
		}

		memset(key_, 0, DWNLD_TOKEN_SIZE);
		delete[] key_;

		//-----------------------Generating rubbish section---------------------------------------------

		unsigned char* rubbish0 = new unsigned char[142];
		unsigned char* rubbish1 = new unsigned char[7];
		unsigned char* rubbish2 = new unsigned char[75];
		unsigned char* rubbish3 = new unsigned char[16];
		unsigned char* rubbish4 = new unsigned char[12];
		unsigned char* rubbish5 = new unsigned char[4];
		unsigned char* rubbish6 = new unsigned char[4];

		generate_random_bytes(rubbish0, 141);
		generate_random_bytes(rubbish1, 6);
		generate_random_bytes(rubbish2, 74);
		generate_random_bytes(rubbish3, 15);
		generate_random_bytes(rubbish4, 11);
		generate_random_bytes(rubbish5, 3);
		generate_random_bytes(rubbish6, 3);

		//-----------------------------------------------------------------------------------------------

		token = new unsigned char[size + 1];

		token[0] = MOD_ID::POS_FREE; //mod

		token[1] = map_id;          //map ID

		memcpy(&token[2], rubbish0, 141);

		memset(rubbish0, 0, 141);
		delete[] rubbish0;

		memcpy(&token[143], rubbish1, 6);

		memset(rubbish1, 0, 6);
		delete[] rubbish1;

		memcpy(&token[149], rubbish2, 74);

		memset(rubbish2, 0, 74);
		delete[] rubbish2;

		memcpy(&token[223], rubbish3, 15);

		memset(rubbish3, 0, 15);
		delete[] rubbish3;

		memcpy(&token[238], rubbish4, 11);

		memset(rubbish4, 0, 11);
		delete[] rubbish4;

		memcpy(&token[249], rubbish5, 3);

		memset(rubbish5, 0, 4);
		delete[] rubbish5;

		token[252] = 0xc;

		memcpy(&token[253], key, 256);       //key

		memset(key, 0, 256);
		delete[] key;

		memcpy(&token[509], rubbish6, 3);

		memset(rubbish6, 0, 3);
		delete[] rubbish6;

		//-----------------------------------------------------------------------------------------------
	}

	//-------------------------------------

	if (token == nullptr)
		return 3;

	token[size] = 0;

	std::string_view new_token((char*)token, (size_t)size);

	uint8_t code = send_to_server(new_token);

	delete[] token;

	if (code || !response_size) {  //get token
		return 4;
	}

	if (!map_id) {
		//if (response_size < 3) {
		return (uint32_t)(response_buffer[0]);
		//}
		//else return 10;
	}
	else {
		if (response_size < 3)
			return 9;

		return 0;
	}

}