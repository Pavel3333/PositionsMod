#include "ModLight.hpp"
#include "PyImports.h"


ModLight::ModLight(Vector3& coords, uint8_t signType)
	: light(create(coords, signType))
	, coords(coords)
{
}

ModLight::~ModLight()
{
	if (!light.is_none())
		light("destroyLight");
}

py::object ModLight::create(Vector3& coords, uint8_t signType)
{
	py::object newLight = PyImports::BigWorld("PyOmniLight");
	if (newLight.is_none())
		return py::none();

	newLight.attr("innerRadius") = 1.0;
	newLight.attr("outerRadius") = 10.0;
	newLight.attr("multiplier") = 500.0;

	//-----------position-----------

	newLight.attr("position") = std::make_tuple(
		coords.x,
		coords.y + 0.5 + Globals::vehicleHeight,
		coords.z
	);

	//------------colour------------

	const size_t pyColoursSize = 4;
	py::tuple pyColour(4);

	switch (signType) {
	case 1:                 // Yellow
		pyColour[0] = 255.0;
		pyColour[1] = 255.0;
		pyColour[2] = 0.0;
		break;
	case 2:                 // Blue
		pyColour[0] = 0.0;
		pyColour[1] = 126.0;
		pyColour[2] = 232.0;
		break;
	default:                // Purple
		pyColour[0] = 216.0;
		pyColour[1] = 0.0;
		pyColour[2] = 255.0;
	}

	pyColour[3] = 0.0;

	newLight.attr("colour") = pyColour;

	//------------------------------

	return newLight;
}