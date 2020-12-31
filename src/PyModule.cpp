#define CONSOLE_VER1

#include "CConfig.h"

#include "Py_config.h"

#include <stdlib.h>
#include <direct.h>

#include "MyLogger.h"

#include <array>
#include <string>
#include <memory>
#include <queue>

INIT_LOCAL_MSG_BUFFER;

std::array<const char*, 3> libraries{
	"libeay32",
	"ssleay32",
	"libcurl"
};

static PyObject* pos_light(float coords[3], uint8_t signType);
static PyObject* pos_model(char* path, float coords[3]);

struct ModModel {
	ModModel(char* modelPath = nullptr, float* coords = nullptr, PyObject* animator = nullptr, bool processed = false)
		: processed(processed)
		, animator(animator)
		, coords(coords)
	{
		model = (modelPath && coords)
			? pos_model(modelPath, coords)
			: nullptr;
	}

	bool processed;

	PyObject* model;
	PyObject* animator;

	float* coords;
};

std::vector<ModModel*> models;


struct ModLight {
	ModLight(float coords[3], uint8_t signType);
	~ModLight();
	PyObject* light;
	float* coords;
};

ModLight::ModLight(float coords[3], uint8_t signType)
	: light(nullptr)
	, coords(coords)
{
	this->light = pos_light(coords, signType);
}

ModLight::~ModLight()
{
	if (light && light != Py_None) {
		if (PyObject* result_light = PyObject_CallMethod(light, "destroyLight", NULL)) Py_DECREF(result_light);
	}

	Py_XDECREF(light);

	light = nullptr;
	coords = nullptr;
}

std::vector<std::shared_ptr<ModLight>> lights;


PyObject* AnimationSequence = nullptr;

PyObject* BigWorld = nullptr;

PyObject* imp = nullptr;
PyObject* Minimap = nullptr;

PyObject** minimap = nullptr;

PyObject* g_gui = nullptr;
PyObject* g_appLoader = nullptr;
PyObject* json = nullptr;

uint8_t mapID = NULL;
uint32_t databaseID = NULL;

uint8_t first_check = 100;
uint32_t request = 100;

double height_offset = 0.0;

map current_map;

extern EVENT_ID EventsID;

bool isInited = false;
bool isChatOpened = false;
bool isModelCreated = false;
bool isMarksCreated = false;
bool flag = false;

bool isModpack()
{
	char path[50];

	sprintf_s(path, 50, "./mods/%s/piranhas.mod_pack.wotmod", Config::patch);

	return file_exists(path);
}

bool write_data(char* data_path, PyObject* data_p)
{
	PyObject* arg2 = Py_False;
	Py_INCREF(arg2);

	PyObject* arg3 = Py_True;
	Py_INCREF(arg3);

	PyObject* arg4 = Py_True;
	Py_INCREF(arg4);

	PyObject* arg5 = Py_True;
	Py_INCREF(arg5);

	PyObject* arg6 = Py_None;
	Py_INCREF(arg6);

	PyObject* indent = PyInt_FromSize_t(4);
	Py_INCREF(indent);

	PyObject* __dumps = PyString_FromString("dumps");

	PyObject* data_json_s = PyObject_CallMethodObjArgs(json, __dumps, data_p, arg2, arg3, arg4, arg5, arg6, indent, NULL);

	Py_DECREF(__dumps);
	Py_DECREF(arg2);
	Py_DECREF(arg3);
	Py_DECREF(arg4);
	Py_DECREF(arg5);
	Py_DECREF(arg6);
	Py_DECREF(indent);

	if (!data_json_s)
		return false;

	size_t data_size = PyObject_Length(data_json_s);

	std::ofstream data_w(data_path);

	data_w.write(PyString_AS_STRING(data_json_s), data_size);

	data_w.close();

	Py_DECREF(data_json_s);

	return true;
}

bool read_data(bool isData)
{
	char* data_path;
	PyObject* data_src;
	if (isData) {
		data_path = "./mods/configs/pavel3333/" MOD_NAME "/" MOD_NAME ".json";
		data_src = g_self->data;
	}
	else {
		data_path = "./mods/configs/pavel3333/" MOD_NAME "/i18n/ru.json";
		data_src = g_self->i18n;
	}

	std::ifstream data(data_path, std::ios::binary);

	if (!data.is_open()) {
		data.close();
		if (!write_data(data_path, data_src))
			return false;
	}
	else {
		data.seekg(0, std::ios::end);
		size_t size = (size_t)data.tellg(); //getting file size
		data.seekg(0);

		char* data_s = new char[size + 1];

		while (!data.eof())
			data.read(data_s, size);

		data.close();

		PyObject* data_p = PyString_FromStringAndSize(data_s, size);

		delete[] data_s;

		PyObject* __loads = PyString_FromString("loads");

		PyObject* data_json_s = PyObject_CallMethodObjArgs(json, __loads, data_p, NULL);

		Py_DECREF(__loads);
		Py_DECREF(data_p);

		if (!data_json_s) {
			PyErr_Clear();

			return write_data(data_path, data_src);
		}

		PyObject* old = data_src;
		if (isData) g_self->data = data_json_s;
		else g_self->i18n = data_json_s;

		PyDict_Clear(old);
		Py_DECREF(old);
	}

	return true;
}

void clearModelsSections()
{
	std::array<std::vector<float*>*, 3> coordsContainers {
		&current_map.firing,
		&current_map.lightning,
		&current_map.LFD
	};

	for (auto& coordsContainer : coordsContainers) {
		for (auto& coords : *coordsContainer)
			if (coords)
				delete[] coords;

		coordsContainer->clear();
	}
}

static PyObject* pos_light(float coords[3], uint8_t signType)
{
	if (!isInited)
		goto end_pos_light_1;

	PyObject* newLight = PyObject_CallMethod(BigWorld, "PyOmniLight", NULL);
	if (!newLight) {
		goto end_pos_light_1;
	}

	//---------inner radius---------

	PyObject* innerRadius = PyFloat_FromDouble(1.0);

	if (PyObject_SetAttrString(newLight, "innerRadius", innerRadius)) {

		Py_DECREF(innerRadius);
		goto end_pos_light_2;
	}

	//---------outer radius---------

	PyObject* outerRadius = PyFloat_FromDouble(10.0);

	if (PyObject_SetAttrString(newLight, "outerRadius", outerRadius)) {
		Py_DECREF(outerRadius);
		goto end_pos_light_2;
	}

	//----------multiplier----------

	PyObject* multiplier = PyFloat_FromDouble(500.0);

	if (PyObject_SetAttrString(newLight, "multiplier", multiplier)) {
		Py_DECREF(multiplier);
		goto end_pos_light_2;
	}

	//-----------position-----------

	PyObject* coords_p = PyTuple_New(3);
	if (!coords_p)
		goto end_pos_light_2;

	for (uint8_t i = 0; i < 3; i++) {
		float coord = coords[i];
		if (i == 1)
			coord += 0.5 + height_offset;

		PyTuple_SET_ITEM(coords_p, i, PyFloat_FromDouble(coord));
	}

	if (PyObject_SetAttrString(newLight, "position", coords_p)) {
		Py_DECREF(coords_p);
		goto end_pos_light_2;
	}

	//------------colour------------

	PyObject* colour_p = PyTuple_New(4);
	if (!colour_p)
		goto end_pos_light_2;

	double colour[4];

	if (signType == 1) {		//yellow
		colour[0] = 255.0;
		colour[1] = 255.0;
		colour[2] = 0.0;
	}
	else if (signType == 2) {	//blue
		colour[0] = 0.0;
		colour[1] = 126.0;
		colour[2] = 232.0;
	}
	else {						//purple
		colour[0] = 216.0;
		colour[1] = 0.0;
		colour[2] = 255.0;
	}

	colour[3] = 0.0;

	for (uint8_t i = 0; i < 4; i++)
		PyTuple_SET_ITEM(colour_p, i, PyFloat_FromDouble(colour[i]));

	//------------------------------

	if (PyObject_SetAttrString(newLight, "colour", colour_p)) {
		Py_DECREF(colour_p);
	end_pos_light_2:
		Py_DECREF(newLight);
	end_pos_light_1:
		return nullptr;
	}

	return newLight;
}

static PyObject* pos_model(char* path, float coords[3]) {
	if (!isInited)
		goto end_pos_model_1;

	PyObject* newModel = PyObject_CallMethod(BigWorld, "Model", "s", path);
	if (!newModel)
		goto end_pos_model_1;

	PyObject* coords_p = PyTuple_New(3);

	if (!coords_p)
		goto end_pos_model_2;

	for (uint8_t i = 0; i < 3; i++) {
		float coord = coords[i];
		if (i == 1)
			coord += height_offset;

		PyTuple_SET_ITEM(coords_p, i, PyFloat_FromDouble(coord));
	}

	if (PyObject_SetAttrString(newModel, "position", coords_p)) {
		Py_DECREF(coords_p);
	end_pos_model_2:
		Py_DECREF(newModel);
	end_pos_model_1:
		return NULL;
	}

	return newModel;
}

size_t getPositionsCountByType(uint8_t type) {
	switch (type) {
	case FIRING:
		return current_map.firing.size();
	case LIGHTING:
		return current_map.lightning.size();
	case LFD_S:
		return current_map.LFD.size();
	default:
		return 0;
	}
}

void pos_marker_tuple(std::vector<float*> markerContainer, uint8_t markerID = FIRING)
{
	if (markerContainer.empty() || !minimap[markerID] || markerID > 3)
		return;

	PyObject* minimap_xyz;
	size_t markerIndex = 0;
	for (auto &marker : markerContainer) {
		if (!marker) {
			minimap_xyz = Py_None;
			Py_INCREF(minimap_xyz);
		}
		else {
			minimap_xyz = PyTuple_New(3);

			for (uint8_t counter = 0; counter < 3; counter++)
				PyTuple_SET_ITEM(minimap_xyz, counter, PyFloat_FromDouble(marker[counter]));
		}

		PyTuple_SET_ITEM(minimap[markerID], markerIndex++, minimap_xyz);
	}
}

size_t createModelsPart(uint8_t modelsType, size_t totalModelsCount, bool needToCreate, bool isNewModels)
{
	char* path;
	std::vector<float*>* modelsCoords;
	switch (modelsType) {
	case 1:
		path = isNewModels
			? "objects/pavel3333_positions/new_models/sign1.model"
			: "objects/pavel3333_positions/models/lod0/sign1.model";
		modelsCoords = &current_map.firing;
		break;
	case 2:
		path = isNewModels
			? "objects/pavel3333_positions/new_models/sign2.model"
			: "objects/pavel3333_positions/models/lod0/sign2.model";
		modelsCoords = &current_map.lightning;
		break;
	case 3:
		path = isNewModels
			? "objects/pavel3333_positions/new_models/sign3.model"
			: "objects/pavel3333_positions/models/lod0/sign3.model";
		modelsCoords = &current_map.LFD;
		break;
	default:
		return 0;
	}

	size_t modelIndex = 0;

	for (auto& modelCoords : *modelsCoords) {
		if (!needToCreate || !modelCoords) {
			modelIndex++;
			continue;
		}

		models[totalModelsCount + modelIndex] = new ModModel {
			path,
			modelCoords
		};

		if (!isNewModels)
			lights.push_back(std::make_shared<ModLight>(modelCoords, modelsType));

		modelIndex++;
	}

	return modelIndex;
}

uint8_t create_models()
{
	if (!isInited || first_check || request || !g_self)
		return 1;

	bool createLightning = PyDict_GetItemString(g_self->data, "createLighting") == Py_True;
	bool createFiring = PyDict_GetItemString(g_self->data, "createFiring") == Py_True;
	bool createLFD = PyDict_GetItemString(g_self->data, "createLFD") == Py_True;

	parse_config(&current_map, createLightning, createFiring, createLFD);

	if (!current_map.sections_count)
		return 2;

	models.clear();
	models.resize(current_map.getTotalCount());

	lights.clear();

	for (uint8_t i = 0; i < current_map.getTotalCount(); i++) {
		if (models[i] != nullptr) {
			Py_XDECREF(models[i]->model);

			models[i]->model = nullptr;
			models[i]->coords = nullptr;
			models[i]->processed = false;

			delete models[i];
			models[i] = nullptr;
		}
	}

	//minimap section

	minimap = new PyObject * [3];

	for (uint8_t i = 0; i < 3; i++)
		minimap[i] = nullptr;

	//minimap creating

	if (createFiring) {
		minimap[FIRING] = PyTuple_New(current_map.firing.size());

		pos_marker_tuple(current_map.firing, FIRING);
	}
	if (createLightning) {
		minimap[LIGHTING] = PyTuple_New(current_map.lightning.size());

		pos_marker_tuple(current_map.lightning, LIGHTING);
	}
	if (createLFD) {
		minimap[LFD_S] = PyTuple_New(current_map.LFD.size());

		pos_marker_tuple(current_map.LFD, LFD_S);
	}

	//----------------

	bool isNewModels = PyDict_GetItemString(g_self->data, "newModels") == Py_True;

	uint8_t counter_model = 0;

	counter_model += createModelsPart(2, counter_model, createFiring, isNewModels);
	counter_model += createModelsPart(2, counter_model, createLightning, isNewModels);
	counter_model += createModelsPart(3, counter_model, createLFD, isNewModels);

	return 0;
}

uint8_t init_models()
{
	if (!isInited
		|| first_check
		|| request
		|| models.empty()
	)
		return 1;

	//get BigWorld.player().spaceID

	PyObject* player = PyObject_CallMethod(BigWorld, "player", NULL);

	if (!player)
		return 1;

	PyObject* spaceID_py = PyObject_GetAttrString(player, "spaceID");

	Py_DECREF(player);

	if (!spaceID_py)
		return 2;

	long spaceID = PyInt_AsLong(spaceID_py);

	if (spaceID == -1)
		return 3;

	Py_DECREF(spaceID_py);

	for (uint8_t i = 0; i < current_map.getTotalCount(); i++) {
		if (models[i] == nullptr)
			continue;

		if (models[i]->model == Py_None || !models[i]->model || models[i]->processed) {
			Py_XDECREF(models[i]->model);

			models[i]->model = nullptr;
			models[i]->coords = nullptr;
			models[i]->processed = false;

			delete models[i];
			models[i] = nullptr;

			continue;
		}

		PyObject* __addModel = PyString_FromString("addModel");

		PyObject* result = PyObject_CallMethodObjArgs(BigWorld, __addModel, models[i]->model, NULL);

		Py_DECREF(__addModel);

		if (result) {
			Py_DECREF(result);

			models[i]->processed = true;
		}

		if (!PyDict_GetItemString(g_self->data, "playAnimation"))
			continue;

		/*
			clipResource = model.deprecatedGetAnimationClipResource('rotation')
			loader = AnimationSequence.Loader(clipResource, spaceID)
			animator = loader.loadSync()
			animator.bindTo(AnimationSequence.ModelWrapperContainer(model, spaceID))
			animator.speed = animSpeed
			animator.start()

			self.animator = animator
		*/

		Py_INCREF(models[i]->model);

		PyObject* clipResource;

		if (PyObject_HasAttrString(models[i]->model, "deprecatedGetAnimationClipResource") &&
			PyObject_CallMethod(models[i]->model, "deprecatedGetAnimationClipResource", "s", "rotation")
		) {
			if (PyObject* loader = PyObject_CallMethod(AnimationSequence, "Loader", "Ol", clipResource, spaceID)) {
				if (PyObject* animator = PyObject_CallMethod(loader, "loadSync", NULL)) {
					if (PyObject* binder = PyObject_CallMethod(AnimationSequence, "ModelWrapperContainer", "Ol", models[i]->model, spaceID)) {
						PyObject* __bindTo = PyString_FromString("bindTo");

						Py_INCREF(animator);

						if (PyObject* res = PyObject_CallMethodObjArgs(animator, __bindTo, binder, NULL))
							Py_DECREF(res);

						Py_DECREF(animator);

						Py_DECREF(__bindTo);

						Py_INCREF(animator);

						if (PyObject* res = PyObject_CallMethod(animator, "start", NULL))
							Py_DECREF(res);

						Py_DECREF(animator);

						models[i]->animator = animator;

						Py_DECREF(binder);
					}

					Py_DECREF(animator);
				}

				Py_DECREF(loader);
			}

			Py_DECREF(clipResource);
		}

		Py_DECREF(models[i]->model);
	}

	return 0;
}

float getVehiclePartHeight(PyObject* partBBox, PyObject* heightIndex, PyObject* fromBBoxIndex, PyObject* toBBoxIndex)
{
	float partMinY = 0.0;
	float partMaxY = 0.0;

	PyObject* fromHullBBox = PyObject_GetItem(partBBox, fromBBoxIndex);
	if (fromHullBBox) {
		PyObject* fromHullBBoxHeight = PyObject_GetItem(fromHullBBox, heightIndex);
		if (fromHullBBoxHeight) {
			partMinY = PyFloat_AS_DOUBLE(fromHullBBoxHeight);

			Py_DECREF(fromHullBBoxHeight);
		}

		Py_DECREF(fromHullBBox);
	}

	PyObject* toHullBBox = PyObject_GetItem(partBBox, toBBoxIndex);
	if (toHullBBox) {
		PyObject* toHullBBoxHeight = PyObject_GetItem(toHullBBox, heightIndex);
		if (toHullBBoxHeight) {
			partMaxY = PyFloat_AS_DOUBLE(toHullBBoxHeight);

			Py_DECREF(toHullBBoxHeight);
		}

		Py_DECREF(toHullBBox);
	}

	return partMaxY - partMinY;
}

void get_height_offset()
{
	PyObject* player = PyObject_CallMethod(BigWorld, "player", NULL);

	if (!player)
		return;

	PyObject* player_vehicle = PyObject_GetAttrString(player, "vehicle");

	Py_DECREF(player);

	if (!player_vehicle)
		return;

	PyObject* player_vehicle_typeDescriptor = PyObject_GetAttrString(player_vehicle, "typeDescriptor");

	Py_DECREF(player_vehicle);

	if (!player_vehicle_typeDescriptor)
		return;

	PyObject* player_vehicle_typeDescriptor_hull = PyObject_GetAttrString(player_vehicle_typeDescriptor, "hull");
	PyObject* player_vehicle_typeDescriptor_turret = PyObject_GetAttrString(player_vehicle_typeDescriptor, "turret");

	Py_DECREF(player_vehicle_typeDescriptor);

	if (!player_vehicle_typeDescriptor_hull || !player_vehicle_typeDescriptor_turret) {
		Py_XDECREF(player_vehicle_typeDescriptor_hull);
		Py_XDECREF(player_vehicle_typeDescriptor_turret);

		return;
	}

	PyObject* player_vehicle_typeDescriptor_hull_hitTester = PyObject_GetAttrString(player_vehicle_typeDescriptor_hull, "hitTester");
	PyObject* player_vehicle_typeDescriptor_turret_hitTester = PyObject_GetAttrString(player_vehicle_typeDescriptor_turret, "hitTester");

	Py_DECREF(player_vehicle_typeDescriptor_hull);
	Py_DECREF(player_vehicle_typeDescriptor_turret);

	if (!player_vehicle_typeDescriptor_hull_hitTester || !player_vehicle_typeDescriptor_turret_hitTester) {
		Py_XDECREF(player_vehicle_typeDescriptor_hull_hitTester);
		Py_XDECREF(player_vehicle_typeDescriptor_turret_hitTester);

		return;
	}

	PyObject* player_vehicle_typeDescriptor_hull_hitTester_bbox = PyObject_GetAttrString(player_vehicle_typeDescriptor_hull_hitTester, "bbox");
	PyObject* player_vehicle_typeDescriptor_turret_hitTester_bbox = PyObject_GetAttrString(player_vehicle_typeDescriptor_turret_hitTester, "bbox");

	Py_DECREF(player_vehicle_typeDescriptor_hull_hitTester);
	Py_DECREF(player_vehicle_typeDescriptor_turret_hitTester);

	PyObject* heightIndex = PyInt_FromSize_t(1);

	PyObject* fromBBoxIndex = PyInt_FromSize_t(0);
	PyObject* toBBoxIndex = PyInt_FromSize_t(1);

	double height = 0.0;

	if (player_vehicle_typeDescriptor_hull_hitTester_bbox) {
		height += getVehiclePartHeight(
			player_vehicle_typeDescriptor_hull_hitTester_bbox,
			heightIndex,
			fromBBoxIndex,
			toBBoxIndex
		);

		Py_DECREF(player_vehicle_typeDescriptor_hull_hitTester_bbox);
	}

	if (player_vehicle_typeDescriptor_turret_hitTester_bbox) {
		height += getVehiclePartHeight(
			player_vehicle_typeDescriptor_turret_hitTester_bbox,
			heightIndex,
			fromBBoxIndex,
			toBBoxIndex
		);

		Py_DECREF(player_vehicle_typeDescriptor_turret_hitTester_bbox);
	}

	Py_DECREF(heightIndex);

	Py_DECREF(fromBBoxIndex);
	Py_DECREF(toBBoxIndex);

	// result

	height_offset = 1.0 + height;

	//-----------------------------
}

EVENT_ID getCurrentEventID()
{
	return isModpack()
		? EVENT_ID::GET_POSITIONS_MODPACK  // Using Piranhas modpack
		: EVENT_ID::GET_POSITIONS_TOKEN;   // Mod works in default mode
}

void get(uint8_t map_ID)
{
	if (!isInited || first_check || !databaseID)
		return;

	request = send_token(databaseID, map_ID, getCurrentEventID());

	if (request) {
		writeDebugDataToFile(GET, (char*)response_buffer, response_size);
		return;
	}

	if (PyDict_GetItemString(g_self->data, "newModels") == Py_False)
		get_height_offset();

	request = create_models();

	if (request)
		return;

	request = init_models();

	if (request)
		return;
}

uint8_t pos_first_check()
{
	if (!isInited || !databaseID)
		return 1;

	if (first_check = send_token(databaseID, 0, getCurrentEventID())) {
		writeDebugDataToFile(CHECK, (char*)response_buffer, response_size);

		return 2;
	}

	return 0;
}

uint8_t del_models()
{
	if (!isInited
		|| first_check
		|| request
		|| models.empty()
	)
		return 1;


	for (auto model : models) {
		if (model == nullptr)
			continue;

		if (model->model && model->model != Py_None && model->processed) {
			PyObject* __delModel = PyString_FromString("delModel");

			if (PyObject* result = PyObject_CallMethodObjArgs(BigWorld, __delModel, model->model, NULL))
				Py_DECREF(result);

			Py_DECREF(__delModel);
		}

		Py_XDECREF(model->model);
		Py_XDECREF(model->animator);

		delete model;
	}

	models.clear();

	return NULL;
}

static PyObject* get_minimap(uint8_t markerID)
{
	if (
		!isInited ||
		first_check ||
		request ||
		!current_map.sections_count ||
		!current_map.getTotalCount() ||
		minimap == nullptr ||
		!minimap[markerID] ||
		markerID > 2
	)
		return 0;

	Py_INCREF(minimap[markerID]);

	return minimap[markerID];
};

void createMarkers()
{
	if (!isInited)
		return;

	PyObject* res = nullptr;

	if (PyDict_GetItemString(g_self->data, "createMarkers") == Py_True) {
		PyObject* minimap_exists = PyObject_GetAttrString(Minimap, "minimapEntries");
		if (minimap_exists != Py_None) {
			if (isModelCreated) {
				if (isMarksCreated)
					res = PyObject_CallMethod(Minimap, "delMarkers", NULL);
				else {
					char** markerIDs = new char* [3];

					markerIDs[FIRING] = "yellow";
					markerIDs[LIGHTING] = "green";
					markerIDs[LFD_S] = "red";

					for (uint8_t i = 0; i < 3; i++) {
						PyObject* minimap_marks = get_minimap(i);
						if (!minimap_marks)
							continue;

						res = PyObject_CallMethod(Minimap, "createMinimapPoints", "Os", minimap_marks, markerIDs[i]);

						Py_DECREF(minimap_marks);
					}

					delete[] markerIDs;
				}

				isMarksCreated = !isMarksCreated;
			}
			else if (isMarksCreated) {
				res = PyObject_CallMethod(Minimap, "delMarkers", 0);
				isMarksCreated = !isMarksCreated;
			}
		}

		Py_DECREF(minimap_exists);
	}

	Py_XDECREF(res);
}

static PyObject* set_visible(bool isVisible)
{
	if (!isInited || first_check || request)
		return PyInt_FromSize_t(1);

	PyObject* py_visible = PyBool_FromLong(isVisible);

	if (models.empty())
		return PyInt_FromSize_t(3);

	for (size_t i = 0; i < current_map.getTotalCount(); i++) {
		if (models[i] == nullptr)
			continue;

		if (!models[i]->model || models[i]->model == Py_None || !models[i]->processed) {
			Py_XDECREF(models[i]->model);

			models[i]->model = nullptr;
			models[i]->coords = nullptr;
			models[i]->processed = false;

			delete models[i];
			models[i] = nullptr;

			continue;
		}

		PyObject_SetAttrString(models[i]->model, "visible", py_visible);
	}

	Py_DECREF(py_visible);

	Py_RETURN_NONE;
}

void createPositions() {
	isModelCreated = !isModelCreated;
	set_visible(isModelCreated);
}

uint8_t battle_greetings()
{
	uint8_t result = 0;

	if (!isInited || !g_self) {
		result = 1;
		goto end_battle_greetings_1;
	}

	if (PyDict_GetItemString(g_self->data, "showBattleGreetings") == Py_False) {
		result = 2;
		goto end_battle_greetings_1;
	}

	PyObject* battle = PyObject_CallMethod(g_appLoader, "getDefBattleApp", NULL);

	if (!battle) {
		result = 3;
		goto end_battle_greetings_1;
	}

	PyObject* containerManager = PyObject_GetAttrString(battle, "containerManager");

	Py_DECREF(battle);

	if (!containerManager) {
		result = 4;
		goto end_battle_greetings_1;
	}

	PyObject* viewContainer = PyObject_CallMethod(containerManager, "getContainer", "s", "view");

	Py_DECREF(containerManager);

	if (!viewContainer) {
		result = 5;
		goto end_battle_greetings_1;
	}

	PyObject* battle_page = PyObject_CallMethod(viewContainer, "getView", NULL);

	Py_DECREF(viewContainer);

	if (!battle_page) {
		result = 6;
		goto end_battle_greetings_1;
	}

	PyObject* components = PyObject_GetAttrString(battle_page, "components");

	Py_DECREF(battle_page);

	if (!components) {
		result = 7;
		goto end_battle_greetings_1;
	}

	PyObject* __battlePlayerMessages = PyString_FromString("battlePlayerMessages");

	PyObject* BPM = PyObject_GetItem(components, __battlePlayerMessages);

	Py_DECREF(__battlePlayerMessages);
	Py_DECREF(components);

	if (!BPM) {
		result = 8;
		goto end_battle_greetings_1;
	}

	PyObject* message_on = nullptr;
	PyObject* message_count = nullptr;

	bool delete_message_count = false;

	if (!first_check && PyDict_GetItemString(g_self->data, "enabled") == Py_True) {
		message_on = PyDict_GetItemString(g_self->i18n, "UI_message_on");

		if (request != 9 && current_map.getTotalCount()) {
			PyObject* message_count_raw_p = PyDict_GetItemString(g_self->i18n, "UI_trjcount_0");

			if (message_count_raw_p) {
				char* message_count_raw_src = PyString_AS_STRING(message_count_raw_p);

				size_t size = strlen(message_count_raw_src);

				char* message_count_raw = new char[size + 6];
				char* counter = new char[5];
				memset(counter, ' ', 4);

				memcpy(message_count_raw, message_count_raw_src, size);

				_itoa_s(current_map.getTotalCount(), counter, 5, 10);
				memcpy(&message_count_raw[size], counter, 5);
				message_count_raw[size + 5] = 0;

				memset(counter, ' ', 4);
				delete[] counter;

				message_count = PyString_FromString(message_count_raw);

				delete_message_count = true;

				memset(message_count_raw, ' ', 24);
				delete[] message_count_raw;
			}
		}
		else
			message_count = PyDict_GetItemString(g_self->i18n, "UI_trjcount_1");
	}
	else
		message_on = PyDict_GetItemString(g_self->i18n, "UI_message_off");

	if (!message_on) {
		result = 9;
		goto end_battle_greetings_2;
	}

	PyObject* first_attr = Py_None;
	Py_INCREF(first_attr);

	PyObject* __as_showGoldMessageS = PyString_FromString("as_showGoldMessageS");
	PyObject* message_enabled = PyObject_CallMethodObjArgs(BPM, __as_showGoldMessageS, first_attr, message_on, NULL);

	if (!message_enabled) {
		Py_DECREF(__as_showGoldMessageS);

		result = 10;
		goto end_battle_greetings_3;
	}

	if (!message_count) {
		Py_DECREF(__as_showGoldMessageS);

		result = 11;
		goto end_battle_greetings_4;
	}

	PyObject* message_count_ = PyObject_CallMethodObjArgs(BPM, __as_showGoldMessageS, first_attr, message_count, NULL);

	Py_DECREF(__as_showGoldMessageS);

	if (!message_count_) {
		result = 12;
		goto end_battle_greetings_5;
	}

	Py_DECREF(message_count_);
end_battle_greetings_5:
	if (delete_message_count) Py_DECREF(message_count);
end_battle_greetings_4:
	Py_DECREF(message_enabled);
end_battle_greetings_3:
	Py_DECREF(first_attr);
end_battle_greetings_2:
	Py_DECREF(BPM);
end_battle_greetings_1:
	return result;
}

static PyObject* pos_battle_greetings(PyObject* self, PyObject* args)
{
	if (!isInited || !g_self)
		return PyInt_FromSize_t(1);

	//minimap markers creating on start battle

	if (!first_check &&
		PyDict_GetItemString(g_self->data, "enabled") == Py_True &&
		PyDict_GetItemString(g_self->data, "showOnStartBattle") == Py_True) {

		createPositions();
		createMarkers();
	}

	//------------------

	uint8_t battle_greet = battle_greetings();

	if (battle_greet)
		return PyInt_FromSize_t(battle_greet);
	else
		Py_RETURN_NONE;
}

static PyObject* pos_start(PyObject* self, PyObject* args)
{

	if (!isInited || first_check || PyDict_GetItemString(g_self->data, "enabled") == Py_False)
		return PyInt_FromSize_t(1);

	PyObject* player = PyObject_CallMethod(BigWorld, "player", NULL);

	isModelCreated = false;
	isMarksCreated = false;

	if (!player)
		return PyInt_FromSize_t(2);

	PyObject* arena = PyObject_GetAttrString(player, "arena");

	Py_DECREF(player);

	if (!arena)
		return PyInt_FromSize_t(3);

	PyObject* arenaType = PyObject_GetAttrString(arena, "arenaType");

	Py_DECREF(arena);

	if (!arenaType)
		return PyInt_FromSize_t(4);

	PyObject* map_PS = PyObject_GetAttrString(arenaType, "geometryName");

	Py_DECREF(arenaType);

	if (!map_PS)
		return PyInt_FromSize_t(5);

	char* map_s = PyString_AS_STRING(map_PS);

	Py_DECREF(map_PS);

	char map_ID_s[4];
	memcpy(map_ID_s, map_s, 3);
	if (map_ID_s[2] == '_')
		map_ID_s[2] = 0;
	map_ID_s[3] = 0;

	mapID = atoi(map_ID_s);

	request = 0;

	get(mapID);

	if (request)
		return PyInt_FromSize_t(request);

	set_visible(false);

	Py_RETURN_NONE;
}

static PyObject* pos_fini(PyObject* self, PyObject* args)
{
	if (!isInited || first_check || request || PyDict_GetItemString(g_self->data, "enabled") == Py_False)
		return PyInt_FromSize_t(1);

	if (PyDict_GetItemString(g_self->data, "createMarkers") == Py_True) {
		PyObject* minimap_exists = PyObject_GetAttrString(Minimap, "minimapEntries");
		if (minimap_exists) {
			if (minimap_exists != Py_None) {
				PyObject* res = PyObject_CallMethod(Minimap, "delMarkers", NULL);

				Py_XDECREF(res);
			}

			Py_DECREF(minimap_exists);
		}
	}

	request = 0;

	lights.clear();

	uint8_t delete_models = del_models();

	if (delete_models)
		return PyInt_FromSize_t(2);

	if (current_map.sections_count && current_map.getTotalCount()) {
		if (minimap != nullptr) {
			for (uint8_t i = 0; i < 3; i++) {
				if (!minimap[i])
					continue;

				for (uint8_t j = 0; j < getPositionsCountByType(i); j++) {
					PyObject* minimap_i_j = PyTuple_GET_ITEM(minimap[i], j);

					for (uint8_t k = 0; k < 3; k++)
						Py_XDECREF(PyTuple_GET_ITEM(minimap_i_j, k));

					Py_XDECREF(minimap_i_j);
				}

				Py_XDECREF(minimap[i]);
			}

			minimap = nullptr;
		}

		clearModelsSections();
	}

	current_map.sections_count = 0;
	mapID = 0;
	height_offset = 0.0;

	Py_RETURN_NONE;
}

static PyObject* pos_err_code(PyObject* self, PyObject* args)
{
	if (!isInited)
		Py_RETURN_NONE;

	return PyInt_FromSize_t(first_check);
}

static PyObject* pos_check(PyObject* self, PyObject* args)
{
	if (!isInited || flag)
		return PyInt_FromSize_t(1);

	PyObject* player = PyObject_CallMethod(BigWorld, "player", NULL);

	if (!player)
		return PyInt_FromSize_t(2);

	PyObject* DBID_string = PyObject_GetAttrString(player, "databaseID");

	Py_DECREF(player);

	if (!DBID_string)
		return PyInt_FromSize_t(3);

	PyObject* DBID_int = PyNumber_Int(DBID_string);

	Py_DECREF(DBID_string);

	if (!DBID_int)
		return PyInt_FromSize_t(4);

	databaseID = PyInt_AS_LONG(DBID_int);

	Py_DECREF(DBID_int);

	uint8_t result = pos_first_check();

	if (result)
		return  PyInt_FromSize_t(result);

	flag = true;

	Py_RETURN_NONE;
}

static PyObject* pos_init(PyObject* self, PyObject* args)
{
	if (!isInited)
		return PyInt_FromSize_t(1);

	PyObject* template_;
	PyObject* apply;
	PyObject* byteify;

	if (!PyArg_ParseTuple(args, "OOO", &template_, &apply, &byteify))
		return PyInt_FromSize_t(2);

	if (g_gui && PyCallable_Check(template_) && PyCallable_Check(apply)) {
		Py_INCREF(template_);
		Py_INCREF(apply);

		PyObject* result = PyObject_CallMethod(g_gui, "register", "sOOO", g_self->ids, template_, g_self->data, apply);

		Py_XDECREF(result);
		Py_DECREF(apply);
		Py_DECREF(template_);
	}

	if (!g_gui && PyCallable_Check(byteify)) {
		Py_INCREF(byteify);

		PyObject* args1 = PyTuple_Pack(1, g_self->i18n);

		PyObject* result = PyObject_CallObject(byteify, args1);

		Py_DECREF(args1);

		if (result) {
			PyDict_Clear(g_self->i18n);
			Py_DECREF(g_self->i18n);

			g_self->i18n = result;
		}

		Py_DECREF(byteify);
	}

	Py_RETURN_NONE;
}

static PyObject* pos_hook_setForcedGuiControlMode(PyObject* self, PyObject* args)
{
	if (!isInited || first_check || request)
		return PyInt_FromSize_t(1);

	PyObject* isChatOpened_ = PyTuple_GetItem(args, NULL);

	if (!isChatOpened_)
		return PyInt_FromSize_t(2);

	isChatOpened = PyInt_AsLong(isChatOpened_);

	Py_RETURN_NONE;
}

static PyObject* pos_hideMarkersInBattle(PyObject* self, PyObject* args)
{
	if (!isInited || first_check || request)
		return PyInt_FromSize_t(1);

	PyObject* minimap_exists = PyObject_GetAttrString(Minimap, "minimapEntries");
	if (minimap_exists != Py_None && isMarksCreated) {
		PyObject* res = PyObject_CallMethod(Minimap, "delMarkers", NULL);

		Py_XDECREF(res);
		isMarksCreated = false;
	}

	Py_RETURN_NONE;
}

static PyObject* pos_inject_handle_key_event(PyObject* self, PyObject* args)
{
	if (!isInited || first_check || request)
		goto end_pos_inject_handle_key_event;

	PyObject* isKeyGetted_buttonShow = nullptr;
	PyObject* isKeyGetted_buttonMinimap = nullptr;

	if (isChatOpened)
		goto end_pos_inject_handle_key_event;

	if (g_gui) {
		PyObject* __get_key = PyString_FromString("get_key");

		isKeyGetted_buttonShow = PyObject_CallMethodObjArgs(g_gui, __get_key, PyDict_GetItemString(g_self->data, "buttonShow"), NULL);
		isKeyGetted_buttonMinimap = PyObject_CallMethodObjArgs(g_gui, __get_key, PyDict_GetItemString(g_self->data, "buttonMinimap"), NULL);

		Py_DECREF(__get_key);
	}
	else {
		PyObject* key = PyObject_GetAttrString(PyTuple_GET_ITEM(args, 0), "key");

		if (!key)
			goto end_pos_inject_handle_key_event;

		PyObject* ____contains__ = PyString_FromString("__contains__");

		isKeyGetted_buttonShow = PyObject_CallMethodObjArgs(PyDict_GetItemString(g_self->data, "buttonShow"), ____contains__, key, NULL);
		isKeyGetted_buttonMinimap = PyObject_CallMethodObjArgs(PyDict_GetItemString(g_self->data, "buttonMinimap"), ____contains__, key, NULL);

		Py_DECREF(____contains__);
	}

	if (isKeyGetted_buttonShow == Py_True) {
		createPositions();
		createMarkers();

		if (isModelCreated)
			battle_greetings();
	}
	Py_XDECREF(isKeyGetted_buttonShow);

	if (isKeyGetted_buttonMinimap == Py_True)
		createMarkers();

	Py_XDECREF(isKeyGetted_buttonMinimap);

end_pos_inject_handle_key_event:
	Py_RETURN_NONE;
}

static struct PyMethodDef pos_methods[] =
{
	{ "a", pos_battle_greetings,             METH_NOARGS,  ":P" }, //battle_greetings
	{ "b", pos_check,                        METH_VARARGS, ":P" }, //check
	{ "c", pos_start,                        METH_NOARGS,  ":P" }, //start
	{ "d", pos_fini,                         METH_NOARGS,  ":P" }, //fini
	{ "e", pos_err_code,                     METH_NOARGS,  ":P" }, //get_error_code
	{ "g", pos_init,                         METH_VARARGS, ":P" }, //init
	{ "h", pos_inject_handle_key_event,      METH_VARARGS, ":P" }, //inject_handle_key_event
	{ "i", pos_hideMarkersInBattle,          METH_NOARGS,  ":P" }, //hideMarkersInBattle
	{ "k", pos_hook_setForcedGuiControlMode, METH_VARARGS, ":P" }, //hook_setForcedGuiControlMode
	{ NULL, NULL, 0, NULL }
};

//---------------------------INITIALIZATION--------------------------

PyMODINIT_FUNC initpos(void)
{
	std::string native_path;
	std::string minimap_module_path;

	PyObject* platform = PyImport_ImportModule("platform");
	if (!platform) {
		debugLogEx(ERROR, "initpos - error while importing platform");

		goto end_initpos_1;
	}

	PyObject* _architecture = PyString_FromString("architecture");
	PyObject* py_architecture_tuple = PyObject_CallMethodObjArgs(platform, _architecture, NULL);
	Py_XDECREF(_architecture);
	if (!py_architecture_tuple) {
		debugLogEx(ERROR, "initpos - error while getting arch");

		goto end_initpos_1;
	}

	PyObject* py_architecture = PyTuple_GetItem(py_architecture_tuple, 0);
	if (!py_architecture) {
		debugLogEx(ERROR, "initpos - error while getting arch item");
		Py_DECREF(py_architecture_tuple);
		goto end_initpos_2;
	}

	native_path = "./res_mods/mods/xfw_packages/" MOD_NAME "/native_" + std::string(PyString_AsString(py_architecture)) + "/";
	minimap_module_path = native_path + "Minimap.pyd";

	Py_DECREF(py_architecture_tuple);

	for (auto& lib_name : libraries) {
		std::string path = native_path + lib_name + ".dll";
		if (!LoadLibraryA(path.c_str())) {
			debugLogEx(ERROR, "initpos - error while loading DLL: %s", path.c_str());
			goto end_initpos_1;
		}
	}

	if (auto err = curl_init()) {
		debugLogEx(ERROR, "initpos - curl_init: error %d", err);

		goto end_initpos_1;
	}

	BigWorld = PyImport_AddModule("BigWorld");

	if (!BigWorld)
		goto end_initpos_1;

	PyObject* appLoader = PyImport_ImportModule("skeletons.gui.app_loader");

	if (!appLoader)
		goto end_initpos_1;

	PyObject* IAppLoader = PyObject_GetAttrString(appLoader, "IAppLoader");

	Py_DECREF(appLoader);

	if (!appLoader)
		goto end_initpos_1;

	PyObject* dependency = PyImport_ImportModule("helpers.dependency");

	if (!dependency) {
		Py_DECREF(IAppLoader);
		goto end_initpos_1;
	}

	PyObject* __instance = PyString_FromString("instance");

	g_appLoader = PyObject_CallMethodObjArgs(dependency, __instance, IAppLoader, NULL);

	Py_DECREF(__instance);
	Py_DECREF(IAppLoader);
	Py_DECREF(dependency);

	if (!g_appLoader)
		goto end_initpos_1;

	json = PyImport_ImportModule("json");

	if (!json)
		goto end_initpos_2;

	if (PyType_Ready(&Config_p))
		goto end_initpos_2;

	Py_INCREF(&Config_p);

	PyObject* g_config = PyObject_CallObject((PyObject*)&Config_p, NULL);

	Py_DECREF(&Config_p);

	if (!g_config)
		goto end_initpos_2;

	PyObject* pos_module = Py_InitModule("pos", pos_methods);
	if (!pos_module)
		goto end_initpos_2;

	if (PyModule_AddObject(pos_module, "l", g_config))
		goto end_initpos_2;

	imp = PyImport_ImportModule("imp");
	if (!imp)
		goto end_initpos_2;

	PyObject* Minimap_module = PyObject_CallMethod(imp, "load_dynamic", "ss", "Minimap", minimap_module_path.c_str(), NULL);
	if (!Minimap_module)
		goto end_initpos_2;

	Minimap = PyObject_CallMethod(Minimap_module, "Minimap", NULL);

	Py_DECREF(Minimap_module);

	if (!Minimap)
		goto end_initpos_2;

	AnimationSequence = PyImport_ImportModule("AnimationSequence");
	if (!AnimationSequence)
		goto end_initpos_3;

	PyObject* mod_mods_gui = PyImport_ImportModule("gui.mods.mod_mods_gui");
	if (!mod_mods_gui) {
		PyErr_Clear();
		g_gui = nullptr;
	}
	else {
		g_gui = PyObject_GetAttrString(mod_mods_gui, "g_gui");

		Py_DECREF(mod_mods_gui);

		if (!g_gui)
			goto end_initpos_3;
	}



	if (!g_gui) {
		_mkdir("./mods/configs");
		_mkdir("./mods/configs/pavel3333");
		_mkdir("./mods/configs/pavel3333/" MOD_NAME);
		_mkdir("./mods/configs/pavel3333/" MOD_NAME "/i18n");

		if (!read_data(true) || !read_data(false))
			goto end_initpos_3;
	}
	else {
		PyObject* data_i18n = PyObject_CallMethod(g_gui, "register_data", "sOOs", Config::ids, g_self->data, g_self->i18n, "pavel3333");
		if (!data_i18n) {
			Py_DECREF(g_gui);
		end_initpos_3:
			Py_DECREF(Minimap);

		end_initpos_2:
			Py_DECREF(g_appLoader);
		end_initpos_1:
			return;
		}

		PyDict_Clear(g_self->data);
		PyDict_Clear(g_self->i18n);

		Py_DECREF(g_self->data);
		Py_DECREF(g_self->i18n);

		g_self->data = PyTuple_GET_ITEM(data_i18n, 0);
		g_self->i18n = PyTuple_GET_ITEM(data_i18n, 1);

		Py_DECREF(data_i18n);
	}

	isInited = true;
}