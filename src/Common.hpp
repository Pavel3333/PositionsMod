#pragma once

#include <iostream>
#include <cassert>

#pragma pack(push, 1)
class Vector3 {
public:
	float x;
	float y;
	float z;

	const size_t size() const { return sizeof(Vector3) / sizeof(float); };
	const size_t length() const { return size(); };

	float operator[] (size_t index) const {
		assert(index < size());

		return *(&x + index);
	};
	friend std::istream& operator>> (std::istream& stream, Vector3& vector);
};
constexpr size_t Vector3Size = sizeof(Vector3);
static_assert(Vector3Size == 12);

enum class PositionType : size_t {
	Fire,
	Light,
	Defence,

	SIZE
};
constexpr size_t PositionTypeSize = static_cast<size_t>(PositionType::SIZE);
#pragma pack(pop)