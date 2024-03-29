#include "stdafx.h"
#include "iostream"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <tchar.h>
#include <stdio.h>
#include <ctime>
#include <stdlib.h>
#include <time.h>

#define FILE_BUFFER_SIZE (4096U)
#define LICENSE_KEY_SIZE (252U)

const size_t BUF_SIZE = 4096U;
unsigned char wr_buf[BUF_SIZE + 1];  // char*   wr_buf[BUF_SIZE+1];
size_t wr_index = 0;

//generate random bytes
void generate_random_bytes(unsigned char* out, size_t length) {
	unsigned char ret = 0;
	//srand(clock());
	for (uint_fast32_t i = 0; i < length; i++) {
		ret = rand() % 256;
		while (ret == '"' || ret == '\t' || ret == '\n') ret = rand() % 256;
		out[i] = ret;
	}
}

//CRC hashing 32
static uint32_t crc32_bitwise(const char* data, size_t length, uint32_t previousCrc32 = 0)
{
	/*
	// generate string with two first bytes of Polynomial
	const uint32_t Polynomial = 0xEDB88320;

	srand(clock());

	std::cout << "Polynomial: " << Polynomial << std::endl;

	size_t len1 = rand() % 100;
	size_t len2 = rand() % 512;
	while(len2 - len1 <= 2){
	len2 = rand() % 512;
	}

	char *a = nullptr;
	a = new char[len+1];

	unsigned char rubbish1[len1];
	unsigned char rubbish2[len2 - 2 - len1];
	generate_random_bytes(rubbish1, len1);
	generate_random_bytes(rubbish2, len2 - 2 - len1);

	memcpy(&a, rubbish1, len1);
	memcpy(&a[len1], &Polynomial, 2);
	memcpy(&a[len1 + 2], rubbish2, len2 - 2 - len1);

	std::ofstream file("polynomial.txt", std::ios::binary);
	file.write(a, len2);
	file.close();
	*/

	char v[6] = "\x2d\x3\x24\x3\xb";
	char vt[6] = "\xd2\x2c\x4b\xd8\x1e";
	char b[480] = "\x81\xc\xc6\x77\xf1\x50\x80\xfa\xa1\xdd\x44\xaa\xd3\xbb\x8b\x76\xcf\xe5\xf0\xd9\x33\x50\x29\x0\x8b\x2a\x3b\x2b\x21\x5f\xee\x55\x17\x62\x7f\x17\xb2\x2a\x96\xca\xf9\x7b\x77\xec\x7f\x66\x7c\xc4\x7d\x13\xc3\x49\x23\xbe\x17\x81\x24\x41\x30\x0\x66\xa6\xc1\x50\xb2\x4d\x66\xc9\x47\xca\x7b\x2e\x70\x8e\xb6\x2b\xb3\x52\xfc\xdd\x4c\xb9\x79\x95\xda\x30\xef\xd9\x57\x19\x4e\xac\x4d\x2a\x86\x10\x47\xf\x99\xc8\x86\x57\xf4\x62\xf6\x8d\xff\x75\x61\xa5\xf3\x28\xe1\x9b\xec\xf3\x9b\x56\x3f\xaa\xaa\xd2\xd\xb7\x5a\x7e\x18\xa6\x97\xe3\xd6\xe8\xa5\x87\xc4\xd0\x11\x11\xfd\xe2\xe7\xad\xb\x4d\x25\xb2\xfd\xb6\x70\x83\xb9\x37\x6\xb3\x94\xa8\xf4\x6c\x20\x1d\x89\x11\x45\xaf\x94\x80\xa7\x60\xd7\xf9\xae\x32\xed\x56\xff\x48\xd5\x65\xc0\x7e\x8\x8a\x51\x3\xc\x7\x82\xba\x46\x7a\x4\xe0\x10\xa7\x1e\xf0\xb\x78\xa1\xc3\xb6\x42\x1e\xfc\x40\x4a\x56\xa3\x9b\x88\xf2\x10\xd2\xab\x75\xb\x9c\x5d\x6e\x31\x31\xc7\xf2\x9a\x5c\x58\x49\xb8\xd2\xc2\xe8\x82\xc2\x46\x9f\xd2\xad\x32\x14\xc\x78\xa8\x70\x1\xc0\xa0\x58\x13\x6d\x30\x1f\x99\x8a\xc\x3a\x85\x54\x49\xf4\x44\x90\x66\x63\xe5\x8c\x9a\x7e\xdc\x15\x20\xcb\x94\x4c\xa1\x19\x79\x73\xe4\x62\xa7\xb\xfd\xba\xf8\x4f\xc4\xe9\x1d\x2\x85\x5a\xee\x6a\xf9\x34\xff\x9d\x75\xc6\x74\xfb\x62\x28\x16\xfc\xed\x1f\xa6\x2c\xff\x42\x71\x73\x70\x5b\x24\x9a\x78\x10\xe2\xb\x66\xc3\x94\xd8\x94\xb6\x80\xff\x9a\x73\x1e\xed\xc2\x6b\x26\x40\xbc\xdd\xee\xd1\x93\xf9\xf3\xf2\xe0\x46\xb5\xf4\x3e\x41\xc9\xed\xfd\x47\x29\xb7\x1b\xb4\xce\x3c\x6f\x4d\x81\xf1\x28\xdb\xee\xa1\x76\x19\xfa\x6c\x87\xd0\x56\xf\xb5\x41\x54\x66\xfe\xc3\xfa\x2f\xb4\xad\x56\x10\x75\x76\x10\xdc\x5e\x1b\x46\x12\x7c\xc1\x93\xa6\x83\x9c\x71\x4\xc3\xf\xc2\x53\x57\x15\xa9\xf6\xa0\xdf\x9e\x59\xf2\xbf\xc2\xea\x8c\x44\x6f\x65\xce\xa7\x14\x55\xa6\x65\x40\xd8\x4c\x2e\xfc\xa2\x2f\xfe\x5e\x44\x2f\x8a\x5b\xae\xc\xdd\xdd\xf3\x1e\x43\x15\x4d\x47\x66\x13\x61\x27\xb7\x94\xc4\x8e\xf\x1b\xbd\x37\x96\x43\x4d\xb3\xe5\x53\x6c\xa3\x67\x13\x96\x2a\x5";

	uint32_t Polynomial_;
	unsigned char final[4];

	memset(final, 0, 4U);
	memcpy(&final, &b[54], 2U);
	for (int i = 0; i < 5; i++) final[2] += v[i]; // = 184;
	for (int i = 0; i < 5; i++) final[3] += vt[i]; // = 237;
	memcpy(&Polynomial_, final, 4);

	uint32_t crc = ~previousCrc32;
	unsigned char* current = (unsigned char*)data;
	while (length--)
	{
		crc ^= *current++;
		for (unsigned int j = 0; j < 8; j++)
			crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial_);
	}
	return ~crc; // same as crc ^ 0xFFFFFFFF
}

//get file CRC32 hash by current path
static bool get_file_hash(const char* path, uint32_t* out) {
	char buf[FILE_BUFFER_SIZE];
	uint32_t hash = 0;

	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) {
		file.close();
		return false;
	}
	// Calculating hash
	file.seekg(0, std::ios::end);
	size_t size = (int)file.tellg(); //getting file size
	file.seekg(0);
	while (!file.eof()) {
		std::streamoff buf_size = size - file.tellg();
		if (buf_size == 0) {
			break;
		}
		if (buf_size >= FILE_BUFFER_SIZE) {
			file.read(buf, FILE_BUFFER_SIZE);
			hash = crc32_bitwise(buf, FILE_BUFFER_SIZE, hash);
		}
		else {
			file.read(buf, buf_size);
			hash = crc32_bitwise(buf, FILE_BUFFER_SIZE, hash);
		}
	}
	file.close();
	// Returning hash
	memcpy(out, &hash, 4);
	return true;
}

//get license key hash by current path
static bool get_key(const char* path, unsigned char* out) {
	std::ifstream file(path, std::ios::binary);

	if (!file.is_open()) {
		file.close();
		return false;
	}

	file.seekg(0, std::ios::end);
	std::streamoff size = file.tellg(); //getting file size
	file.seekg(0);

	if (size != LICENSE_KEY_SIZE) {
		return false;
	}
	file.read((char*)out, LICENSE_KEY_SIZE);
	file.close();
	return true;
}

void xor_it(char* key, size_t length, bool is_key = false, char postfix = '0') {
	//encode by xor the row string

	char* key2 = new char[length];
	memcpy(key2, key, length);

	for (uint_fast32_t i = 1; i < length; i++) {
		key[i] = key[i] ^ key[i - 1];
	}

	char* filename = nullptr;
	if (!is_key) {
		filename = new char[length + 12];
		memcpy(filename, "xored_", 6);
		memcpy(&filename[6], key2, length - 1);
		memcpy(&filename[6 + length - 1], ".txt", 4);
		filename[6 + length - 1 + 4] = 0U;
	}
	else {
		filename = new char[15];
		memcpy(filename, "xored_", 6);
		filename[6] = postfix;
		memcpy(&filename[7], "key.txt", 7);
		filename[14] = 0U;
	}
	std::ofstream k((const char*)filename, std::ios::binary);
	k.write((const char*)key, length);
	k.close();
}

void generate_polynomial(uint32_t Polynomial) {
	// generate string with two first bytes of Polynomial
	srand(clock());

	size_t len1 = rand() % 100;
	size_t len2 = rand() % 512;
	while ((int)(len2 - len1) <= 2) {
		len2 = rand() % 512;
	}

	std::cout << "Polynomial[" << len1 << " + 2 + " << len2 << "]: " << Polynomial << std::endl;

	char *a = nullptr;
	a = new char[len1 + len2 + 2 + 1];

	memset(a, 0, len1 + len2 + 2 + 1);

	unsigned char *rubbish1 = nullptr;
	rubbish1 = new unsigned char[len1];
	unsigned char *rubbish2 = nullptr;
	rubbish2 = new unsigned char[len2 - 2 - len1];
	generate_random_bytes(rubbish1, len1);
	generate_random_bytes(rubbish2, len2 - 2 - len1);

	memcpy(a, rubbish1, len1);
	memcpy(&a[len1], &Polynomial, 2);
	memcpy(&a[len1 + 2], rubbish2, len2 - 2 - len1);
	//a[len1 + 2 + len2] = 0U;

	std::ofstream file("polynomial.txt", std::ios::binary);
	file.write(a, len2);
	file.close();

	// generating first part of Polynomial
	int k = 0;

	char *polynomial_bytes = nullptr;
	polynomial_bytes = new char[4];

	memcpy(polynomial_bytes, &Polynomial, 4);

	uint8_t thirdbyte = (uint8_t)polynomial_bytes[2];

	uint8_t fourthbyte = (uint8_t)polynomial_bytes[3];

	unsigned char b[5];
	while (k != thirdbyte) {
	k = 0;
	generate_random_bytes(b, 5);
	for (int i = 0; i < 5; i++) k += b[i];
	}

	std::cout << '5' << std::endl;

	std::ofstream fil("polynomial_thirdbyte.bin", std::ios::binary);
	fil.write((const char*)b, 5);
	fil.close();

	uint8_t k2 = 0U;

	unsigned char a2[5];
	while (k2 != fourthbyte) {
	k2 = 0U;
	generate_random_bytes(a2, 5);
	for (int i = 0; i < 5; i++) k2 += a2[i];
	}

	std::cout << '5' << std::endl;

	std::ofstream fil2("polynomial_fourthbyte.bin", std::ios::binary);
	fil2.write((const char*)a2, 5);
	fil2.close();
}

int main()
{
	uint32_t hash;
	get_file_hash("trj.pyd", &hash);
	std::cout << "hash: " << hash << std::endl;

	//generate_polynomial(hash);

	/*char filename[56] = "res_mods/mods/xfw_packages/TrajectoryMod/native/trj.pyd";
	char license_key_name[63] = "res_mods/mods/xfw_packages/TrajectoryMod/native/TRAJECTORY.lic";
	char key_enc_lic_key[34] = "Lrz~}Gc}a9S~EP#e@HQAICy|s{ @F%{bY";
	char user_agent[21] = "TrajectoryMod v.4.00";

	char base64_alphabet[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char key[48] = "\x34\x43\x75\x2F\x61\x26\x4A\x28\x71\x14\x5E\x3C\x57\x15\x6C\x5\x61\x15\x27\x54\x3D\x5C\x38\x48\x2C\x58\x34\x45\x2\x32\x47\x74\x3E\x49\x3D\x5B\x3E\x4D\x21\x79\x34\x72\x14\x5F\x3E\x3E";//"4w6ZNGlbYeJbkByidt2siadpdtlqG0u3JwtfeslXMFfKa";

	char low_path[39] =  "objects/pavel3333_trajectory/low.model";
	char middle_path[42] = "objects/pavel3333_trajectory/middle.model";
	char high_path[40] = "objects/pavel3333_trajectory/high.model";//"\x6F\x0D\x67\x02\x61\x15\x66\x49\x39\x58\x2E\x4B\x27\x14\x27\x14\x27\x78\x0C\x7E\x1F\x75\x10\x73\x07\x68\x1A\x63\x4C\x24\x4D\x2A\x42\x6C\x01\x6E\x0A\x6F\x03\x03";
	char ultrahigh_path[45] = "objects/pavel3333_trajectory/ultrahigh.model";
	char billboard_path[45] = "objects/pavel3333_trajectory/billboard.model";
	char Model[6] = "Model";
	char position[9] = "position";
	char yaw[4] = "yaw";

	char url_[45] = "http://api.pavel3333.ru/get/index.php?token=";
	xor_it(url_, 45U, true, '3');

	xor_it(filename, 56, true, '0');
	xor_it(license_key_name, 63, true, '1');
	xor_it(key_enc_lic_key, 34, true, '2');
	xor_it(user_agent, 21);
	xor_it(low_path, 39, true, 'l');
	xor_it(middle_path, 42, true, 'm');
	xor_it(high_path, 40, true, 'h');
	xor_it(ultrahigh_path, 45, true, 'u');
	xor_it(billboard_path, 45, true, 'b');
	xor_it(base64_alphabet, 65, true, 'b');
	xor_it(Model, 6);
	xor_it(position, 9);
	xor_it(yaw, 4);

	for (uint8_t i = 62U; i > 0; i--) {
		license_key_name[i] = license_key_name[i] ^ license_key_name[i - 1];
	}*/

	char firing_path[40] = "objects/pavel3333_positions/sign1.model";
	char lighting_path[40] = "objects/pavel3333_positions/sign2.model";
	char LFD_path[40] = "objects/pavel3333_positions/sign3.model";

	xor_it(firing_path, 40U, true, 'f');
	xor_it(lighting_path, 40U, true, 'l');
	xor_it(LFD_path, 40U, true, 'd');

	for (uint8_t i = 40U; i > NULL; i--) {
	firing_path[i] = firing_path[i] ^ firing_path[i - 1];
	}

	//for (uint8_t i = NULL; i < 40U; i++) std::cout << firing_path[i];
	std::cout << firing_path;
	std::cout << std::endl;

	int v = 0;
	std::cin >> v;

    return 0;
}

