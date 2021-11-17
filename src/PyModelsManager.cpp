#include <sstream>

#include "API_functions.h"
#include "Config.h"
#include "PyModelsManager.h"


void PyModelsManager::addModel(std::string_view path, Vector3& position)
{
	ModModelPtr modelPtr = std::make_shared<ModModel>(path, position);

	py::object pyModel = modelPtr->model;
	if (pyModel.is_none())
		return;

	PyImports::BigWorld("addModel", pyModel);

	if (PyGlobals::g_config.getData()["playAnimation"].cast<bool>())
		addAnimation(modelPtr);

	models.push_back(modelPtr);
}

void PyModelsManager::parseConfig()
{
	assert(state == ModelsManagerState::ParseConfig);

	//подчищаем память от старых карт

	for (auto& section : positionsData.positionsSections)
		section.coords.clear();

	//-------------------------------
	size_t responseSize = responseBuffer.size();
	if (responseSize < 3)
		return;

	uint16_t length;

	responseBuffer >> length;
	if (length != responseSize)
		return;

	size_t cursor = responseBuffer.getCursor();
	std::string response(reinterpret_cast<const char*>(responseBuffer.begin() + cursor), responseSize - cursor);
	std::string decryptedResponse = vigenere(response, "DIO4Gb941mfOiHox6jLntKn6kqfgopFX1xaCu1JWlb3ag", false);

	responseBuffer = bstream(decryptedResponse.data(), decryptedResponse.size());
	responseBuffer >> positionsData.sections_count;

	std::array<size_t, PositionTypeSize> sectionSizes;
	for (size_t i = 0; i < PositionTypeSize; i++)
		responseBuffer >> sectionSizes[i];

	for (size_t i = 0; i < PositionTypeSize; i++) {
		if (!positionsData.positionsSections[i].isEnabled())
			continue;

		size_t sectionSize = sectionSizes[i];
		if (!sectionSize)
			continue;

		auto& section = positionsData.positionsSections[i];
		section.coords.resize(sectionSize);
	}

	uint8_t sectionID;
	for (uint8_t i = 0; i < positionsData.sections_count; i++) {
		responseBuffer >> sectionID;
		if (sectionID >= PositionTypeSize)
			break;

		auto& section = positionsData.positionsSections[sectionID];
		if (!section.isEnabled())
			continue;

		for (auto& dstPosition : section.coords) {
			responseBuffer >> dstPosition;
		}
	}

	state = ModelsManagerState::CreateModels;
}


void PyModelsManager::addModels()
{
	assert(state == ModelsManagerState::CreateModels);

	clear();

	models.reserve(positionsData.getPositionsCount());

	py::dict data = PyGlobals::g_config.getData();

	std::array<py::bool_, PositionTypeSize> enabledSections {  // TODO
		data["createLighting"],
		data["createFiring"],
		data["createLFD"]
	};

	bool isNewModels = data["newModels"].cast<bool>();

	std::stringstream path;
	for (size_t sectionID = 0; sectionID < PositionTypeSize; sectionID++) {
		path <<
			"objects/pavel3333_positions/" <<
			(isNewModels ? "new_models" : "models/lod0") << "/" <<
			"sign" << (sectionID + 1) << ".model";

		PositionsDataSection& section = positionsData.positionsSections[sectionID];
		size_t modelIndex = 0;

		for (auto modelCoords : section.coords) {
			if (!enabledSections[sectionID])
				continue;

			addModel(path.str(), modelCoords);

			if (!isNewModels)
				lights.push_back(std::make_shared<ModLight>(modelCoords, sectionID));
		}
	}

	state = ModelsManagerState::Inited;
}

void PyModelsManager::setVisible(bool visible)
{
	for (auto &model : models) {
		model->model.attr("visible") = visible;
	}
}

void PyModelsManager::clear()
{
	positionsData.clear();
	models.clear();
	lights.clear();

	for (uint8_t i = 0; i < PositionTypeSize; i++)
		minimap[i] = py::tuple();
}

void PyModelsManager::addAnimation(ModModelPtr modelPtr)
{
	/*
	    spaceID = BigWorld.player().spaceID

		clipResource = model.deprecatedGetAnimationClipResource('rotation')
		loader = AnimationSequence.Loader(clipResource, spaceID)
		animator = loader.loadSync()
		animator.bindTo(AnimationSequence.ModelWrapperContainer(model, spaceID))
		animator.speed = animSpeed
		animator.start()

		self.animator = animator
	*/

	py::object player = PyImports::BigWorld("player");
	if (player.is_none())
		return;

	py::object spaceID = player.attr("spaceID");
	if (spaceID.is_none())
		return;

	py::object pyModel = modelPtr->model;

	py::object clipResource = pyModel("deprecatedGetAnimationClipResource", "rotation");
	if (!clipResource)
		return;

	py::object loader = PyImports::AnimationSequence("Loader", clipResource, spaceID);
	if (!loader)
		return;

	py::object animator = loader("loadSync");
	if (!animator)
		return;

	py::object binder = PyImports::AnimationSequence("ModelWrapperContainer", pyModel, spaceID);
	if (!binder)
		return;

	animator("bindTo", binder);
	animator("start");

	modelPtr->animator = animator;
}
