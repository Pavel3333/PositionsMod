#include "ModModel.hpp"
#include "PyImports.h"


ModModel::ModModel(std::string_view modelPath, Vector3& coords, py::object animator)
	: animator(animator)
	, coords(coords)
	, model(createModel(modelPath, coords))
{
}

ModModel::~ModModel()
{
	if (!model.is_none())
		PyImports::BigWorld("delModel", model);
}

py::object ModModel::createModel(std::string_view path, Vector3& coords)
{
	if (path.empty())
		return py::none();

	py::object newModel = PyImports::BigWorld("Model", path);
	if (!newModel)
		return py::none();

	py::tuple pyCoords(3);
	if (!pyCoords)
		return py::none();

	newModel.attr("position") = py::make_tuple(
		coords.x,
		coords.y + Globals::vehicleHeight,
		coords.z
	);

	return newModel;
}