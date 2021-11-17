#include "pch.h"
#include "py_pch.h"


class ModLight {
public:
	ModLight(Vector3& coords, uint8_t signType);
	~ModLight();

	py::object light;
	Vector3& coords;
private:
	py::object create(Vector3& coords, uint8_t signType);
};

using ModLightPtr = std::shared_ptr<ModLight>;