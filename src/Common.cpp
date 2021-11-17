#include "Common.h"


std::istream& operator>> (std::istream& stream, Vector3& vector) {
	stream >> vector.x;
	stream >> vector.y;
	stream >> vector.z;

	return stream;
}
