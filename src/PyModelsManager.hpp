#pragma once

#include "Common.h"
#include "PyImports.h"
#include "pch.h"
#include "py_pch.h"
#include "API_functions.h"
#include "ModModel.hpp"
#include "ModLight.hpp"


enum class ModelsManagerState {
	Init,
	ParseConfig,
	CreateModels,
	Inited,
	Fini
};

class PyModelsManager {
public:
	PyModelsManager() : state(ModelsManagerState::Init) {};

	const ModelsManagerState getState() const { return state; }
	void parseConfig();
	void addModels();
	void setVisible(bool visible);
	void clear();

	PositionsData positionsData;
	std::vector<ModModelPtr> models;
	std::vector<ModLightPtr> lights;
	std::array<py::tuple, PositionTypeSize> minimap;
private:
	void addAnimation(ModModelPtr modelPtr);
	void addModel(std::string_view path, Vector3& position);

	ModelsManagerState state;
};