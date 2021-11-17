#include "pch.h"
#include "py_pch.h"


class ModModel {
public:
	ModModel(std::string_view modelPath, Vector3& coords, py::object animator = py::none());
	~ModModel();

	py::object model;
	py::object animator;

	Vector3 coords;
private:
	py::object createModel(std::string_view path, Vector3& coords);
};

using ModModelPtr = std::shared_ptr<ModModel>;