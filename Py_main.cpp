#define CONSOLE_VER1

#include "CConfig.h"

#include "Py_config.h"

#include <stdlib.h>
#include <direct.h>

#include "../NY_Event/MyLogger.h"

#include <queue>

INIT_LOCAL_MSG_BUFFER;

#define CREATE_MODELS  true
#define CREATE_LIGHTS  true
#define CREATE_MARKERS true

#define NEW_MINIMAP_MARKERS true

#if CREATE_MODELS
typedef struct {
	bool processed = false;

	PyObject* model    = nullptr;
#if !PATCH_9_22
	PyObject* animator = nullptr;
#endif
	float* coords      = nullptr;
} ModModel;

std::vector<ModModel*> models;
#endif

#if CREATE_LIGHTS
typedef struct {
	PyObject* light = nullptr;
	float* coords   = nullptr;
} ModLight;

std::vector<ModLight*> lights;
#endif

#if !PATCH_9_22
PyObject* AnimationSequence = nullptr;
#endif

PyObject* BigWorld = nullptr;

#if CREATE_MARKERS
PyObject* Minimap = nullptr;
PyObject** minimap = nullptr;
#endif

PyObject* g_gui = nullptr;
PyObject* g_appLoader = nullptr;
PyObject* json = nullptr;

uint8_t mapID       = NULL;
uint32_t databaseID = NULL;

uint8_t first_check = 100U;
uint32_t request    = 100U;

double height_offset = 0.0;

map current_map;

extern EVENT_ID EventsID;

bool isInited = false;
bool isChatOpened = false;
bool isModelCreated = false;
bool isMarksCreated = false;
bool flag = false;

bool isModpack() { traceLog
	char path[50];

	sprintf_s(path, 50U, "./mods/%s/piranhas.mod_pack.wotmod", Config::patch);

	return file_exists(path);
}

bool write_data(char* data_path, PyObject* data_p) { traceLog
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

	if (!data_json_s) { traceLog
		return false;
	}

	size_t data_size = PyObject_Length(data_json_s);

	std::ofstream data_w(data_path);

	data_w.write(PyString_AS_STRING(data_json_s), data_size);

	data_w.close();

	Py_DECREF(data_json_s);
	return true;
}

bool read_data(bool isData) { traceLog
	char* data_path;
	PyObject* data_src;
	if (isData) { traceLog 
		data_path = "mods/configs/pavel3333/" MOD_NAME "/" MOD_NAME ".json";
		data_src = g_self->data;
	}
	else { traceLog
		data_src = g_self->i18n;
		data_path = "mods/configs/pavel3333/" MOD_NAME "/i18n/ru.json";
	}

	std::ifstream data(data_path, std::ios::binary);

	if (!data.is_open()) { traceLog
		data.close();
		if (!write_data(data_path, data_src)) return false;
	}
	else { traceLog
		data.seekg(0, std::ios::end);
		size_t size = (size_t)data.tellg(); //getting file size
		data.seekg(0);

		char* data_s = new char[size + 1];

		while (!data.eof()) {
			data.read(data_s, size);
		}
		data.close();

		PyObject* data_p = PyString_FromStringAndSize(data_s, size);

		delete[] data_s;

		PyObject* __loads = PyString_FromString("loads");

		PyObject* data_json_s = PyObject_CallMethodObjArgs(json, __loads, data_p, NULL);

		Py_DECREF(__loads);
		Py_DECREF(data_p);

		if (!data_json_s) { traceLog
			PyErr_Clear();

			if (!write_data(data_path, data_src)) return false;
			return true;
		}

		PyObject* old = data_src;
		if (isData) g_self->data = data_json_s;
		else g_self->i18n = data_json_s;

		PyDict_Clear(old);
		Py_DECREF(old);
	}

	return true;
}

void clearModelsSections() { traceLog
	uint8_t i = NULL;

	if (!current_map.firing.empty()) { traceLog
		for (std::vector<float*>::const_iterator it = current_map.firing.cbegin();
			it != current_map.firing.cend();
			it++) {
			if (*it == nullptr) continue;

			for (uint8_t counter = NULL; counter < 3; counter++) {
				memset(&(*it)[counter], NULL, 4);
			}

			delete[] *it;
		}

		current_map.firing.~vector();
	}

	if (!current_map.lighting.empty()) { traceLog
		for (std::vector<float*>::const_iterator it = current_map.lighting.cbegin();
			it != current_map.lighting.cend();
		it++) {
			if (*it == nullptr) continue;

			for (uint8_t counter = NULL; counter < 3; counter++) {
				memset(&(*it)[counter], NULL, 4);
			}

			delete[] *it;
		}

		current_map.lighting.~vector();
	}

	if (!current_map.LFD.empty()) { traceLog
		for (std::vector<float*>::const_iterator it = current_map.LFD.cbegin();
			it != current_map.LFD.cend();
			it++) {
			if (*it == nullptr) continue;

			for (uint8_t counter = NULL; counter < 3; counter++) {
				memset(&(*it)[counter], NULL, 4);
			}

			delete[] *it;
	}

		current_map.LFD.~vector();
	}
}

#if CREATE_LIGHTS
static PyObject* pos_light(float coords[3], uint8_t signType) {
	if (!isInited)
		goto end_pos_light_1;

	extendedDebugLog("light creating...");

	PyObject* Light = PyObject_CallMethod(BigWorld, "PyOmniLight", NULL);

	if (!Light) { traceLog
		extendedDebugLog("PyOmniLight creating FAILED");

		goto end_pos_light_1;
	}

	//---------inner radius---------

	PyObject* innerRadius = PyFloat_FromDouble(1.0);

	if (PyObject_SetAttrString(Light, "innerRadius", innerRadius)) { traceLog
		extendedDebugLog("PyOmniLight innerRadius setting FAILED");

		Py_DECREF(innerRadius);
		goto end_pos_light_2;
	}

	//---------outer radius---------

	PyObject* outerRadius = PyFloat_FromDouble(10.0);

	if (PyObject_SetAttrString(Light, "outerRadius", outerRadius)) { traceLog
		extendedDebugLog("PyOmniLight outerRadius setting FAILED");

		Py_DECREF(outerRadius);
		goto end_pos_light_2;
	}

	//----------multiplier----------

	PyObject* multiplier = PyFloat_FromDouble(500.0);

	if (PyObject_SetAttrString(Light, "multiplier", multiplier)) { traceLog
		extendedDebugLog("PyOmniLight multiplier setting FAILED");

		Py_DECREF(multiplier);
		goto end_pos_light_2;
	}

	//-----------position-----------

	PyObject* coords_p = PyTuple_New(3);

	if (!coords_p) { traceLog
		extendedDebugLog("PyOmniLight coords creating FAILED");

		goto end_pos_light_2;
	}

	for (uint8_t i = NULL; i < 3; i++) {
		if (i == 1)
			PyTuple_SET_ITEM(coords_p, i, PyFloat_FromDouble(coords[i] + 0.5 + height_offset));
		else
			PyTuple_SET_ITEM(coords_p, i, PyFloat_FromDouble(coords[i]));
	}

	if (PyObject_SetAttrString(Light, "position", coords_p)) { traceLog
		extendedDebugLog("PyOmniLight coords setting FAILED");

		Py_DECREF(coords_p);
		goto end_pos_light_2;
	}

	//------------colour------------

	PyObject* colour_p = PyTuple_New(4);

	if (!colour_p) { traceLog
		extendedDebugLog("PyOmniLight colour creating FAILED");

		goto end_pos_light_2;
	}

	double* colour = new double[5];

	if (signType == 1) {		//yellow
		colour[0] = 255.0;
		colour[1] = 255.0;
		colour[2] = 0.0;
	}
	else if (signType == 2) {	//green
		colour[0] = 0.0;
		colour[1] = 255.0;
		colour[2] = 0.0;
	}
	else {						//red
		colour[0] = 255.0;
		colour[1] = 0.0;
		colour[2] = 0.0;
	}

	colour[3] = 0.0;

	for (uint8_t i = NULL; i < 4; i++)
		PyTuple_SET_ITEM(colour_p, i, PyFloat_FromDouble(colour[i]));

	delete[] colour;

#if PATCH_9_22
	//-----------visible------------

	PyObject* visible = PyBool_FromLong(1);

	if (auto res = PyObject_SetAttrString(Light, "visible", visible)) { traceLog
		extendedDebugLog("PyOmniLight visible setting FAILED");

		Py_DECREF(visible);
		goto end_pos_light_2;
	}
#endif

	//------------------------------

	if (PyObject_SetAttrString(Light, "colour", colour_p)) { traceLog
		extendedDebugLog("PyOmniLight colour setting FAILED");

		Py_DECREF(colour_p);
end_pos_light_2: traceLog
		Py_DECREF(Light);
end_pos_light_1: traceLog
		return nullptr;
	}

	superExtendedDebugLog("light creating OK!");

	return Light;
}
#endif

#if CREATE_MODELS
static PyObject* pos_model(char* path, float coords[3]) {
	if (!isInited)
		goto end_pos_model_1;

	PyObject* Model = PyObject_CallMethod(BigWorld, "Model", "s", path);

	if (!Model)
		goto end_pos_model_1;

	PyObject* coords_p = PyTuple_New(3);

	if (!coords_p)
		goto end_pos_model_2;

	for (uint8_t i = NULL; i < 3; i++) {
		if (i == 1)
			PyTuple_SET_ITEM(coords_p, i, PyFloat_FromDouble(coords[i] + height_offset));
		else
			PyTuple_SET_ITEM(coords_p, i, PyFloat_FromDouble(coords[i]));
	}

	if (PyObject_SetAttrString(Model, "position", coords_p)) { traceLog
		Py_DECREF(coords_p);
end_pos_model_2: traceLog
		Py_DECREF(Model);
end_pos_model_1: traceLog
		return NULL;
	}

	superExtendedDebugLog("model creating OK!");

	return Model;
};
#endif

#if CREATE_MARKERS
void pos_marker_tuple(std::vector<float*> markerContainer, uint8_t markerID = NULL) {
	if (markerContainer.empty() || !minimap[markerID] || markerID > 3)
		return;

	size_t count = NULL;

	if      (markerID == FIRING)   count = current_map.firing.size();
	else if (markerID == LIGHTING) count = current_map.lighting.size();
	else if (markerID == LFD_S)    count = current_map.LFD.size();

	for (std::vector<float*>::const_iterator it = markerContainer.cbegin();
		it != markerContainer.cend();
		it++) {
		ptrdiff_t ctr = std::distance(markerContainer.cbegin(), it);

		if (*it == nullptr) { traceLog
			Py_INCREF(Py_None);
			PyTuple_SET_ITEM(minimap[markerID], ctr, Py_None);

			continue;
		}
		else {
			PyObject* minimap_xyz = PyTuple_New(3);

			for (uint8_t counter = NULL; counter < 3; counter++) {
				PyTuple_SET_ITEM(minimap_xyz, counter, PyFloat_FromDouble((*it)[counter]));

				superExtendedDebugLog("%f, ", (*it)[counter]);
			}

			PyTuple_SET_ITEM(minimap[markerID], ctr, minimap_xyz);
		}
	}
}
#endif

uint8_t create_models() { traceLog
	if (!isInited || first_check || request || !g_self) { traceLog
		return 1;
	}

	bool createLighting = true, createFiring = true, createLFD = true;

	if (PyDict_GetItemString(g_self->data, "createFiring") == Py_False) { traceLog
		createFiring = false;
	}

	if (PyDict_GetItemString(g_self->data, "createLighting") == Py_False) { traceLog
		createLighting = false;
	}

	if (PyDict_GetItemString(g_self->data, "createLFD") == Py_False) { traceLog
		createLFD = false;
	}

	debugLog("parsing...");

	//Py_BEGIN_ALLOW_THREADS
	parse_config(&current_map, createLighting, createFiring, createLFD);
	//Py_END_ALLOW_THREADS

	debugLog("parsing OK!");

	if (!current_map.sections_count || !current_map.minimap_count)
		return 2;

	superExtendedDebugLog("sect count: %d\npos count: %d", (uint32_t)current_map.sections_count, (uint32_t)current_map.minimap_count);

	debugLog("creating...");

#if CREATE_MODELS
	models.~vector();
	models.resize(current_map.minimap_count);
#endif

#if CREATE_LIGHTS
	lights.~vector();
#endif

	for (uint8_t i = NULL; i < current_map.minimap_count; i++) {
#if CREATE_MODELS
		if (models[i] != nullptr) {
			Py_XDECREF(models[i]->model);

			models[i]->model     = nullptr;
			models[i]->coords    = nullptr;
			models[i]->processed = false;

			delete models[i];
			models[i] = nullptr;
		}
#endif
	}

	uint8_t counter_model = NULL;

#if CREATE_MARKERS
	superExtendedDebugLog("minimap: [");

	//minimap section

	minimap = new PyObject*[3];

	for (uint8_t i = NULL; i < 3; i++)
		minimap[i] = nullptr;

	//minimap creating

	if (createFiring) { traceLog
		minimap[FIRING] = PyTuple_New(current_map.firing.size());

		pos_marker_tuple(current_map.firing, FIRING);
	}
	if (createLighting) { traceLog
		minimap[LIGHTING] = PyTuple_New(current_map.lighting.size());

		pos_marker_tuple(current_map.lighting, LIGHTING);
	}
	if (createLFD) { traceLog
		minimap[LFD_S] = PyTuple_New(current_map.LFD.size());

		pos_marker_tuple(current_map.LFD, LFD_S);
	}

	//----------------

#endif

	//firing section

	char* firing_path = new char[54];
	memcpy(firing_path, "\x6F\xD\x67\x2\x61\x15\x66\x49\x39\x58\x2E\x4B\x27\x14\x27\x14\x27\x78\x8\x67\x14\x7D\x9\x60\xF\x61\x12\x3D\x50\x3F\x5B\x3E\x52\x21\xE\x62\xD\x69\x59\x76\x5\x6C\xB\x65\x54\x7A\x17\x78\x1C\x79\x15\x15", 53U);

	for (uint8_t i = 52U; i > NULL; i--) {
		firing_path[i] = firing_path[i] ^ firing_path[i - 1];
	}

	superExtendedDebugLog("firing: [");

	if (!current_map.firing.empty()) { traceLog
		for (std::vector<float*>::const_iterator it = current_map.firing.cbegin();
			it != current_map.firing.cend();
			it++) {
			if (createFiring) {
				if (*it == nullptr) { traceLog
					superExtendedDebugLog("NULL, ");

					counter_model++;
					continue;
				}

				superExtendedDebugLog("[");
#if CREATE_MODELS
				models[counter_model] = new ModModel;

				models[counter_model]->coords    = *it;
				models[counter_model]->model     = pos_model(firing_path, *it);
				models[counter_model]->processed = false;
#endif

#if CREATE_LIGHTS
				ModLight* light = new ModLight {
					pos_light(*it, 1),
					*it
				};

				lights.push_back(light);
#endif

				counter_model++;

				superExtendedDebugLog("], ");
			}
			else counter_model++;
		}
	}

	superExtendedDebugLog("], ");

	delete[] firing_path;

	//lighting section

	char* lighting_path = new char[54];
	memcpy(lighting_path, "\x6F\xD\x67\x2\x61\x15\x66\x49\x39\x58\x2E\x4B\x27\x14\x27\x14\x27\x78\x8\x67\x14\x7D\x9\x60\xF\x61\x12\x3D\x50\x3F\x5B\x3E\x52\x21\xE\x62\xD\x69\x59\x76\x5\x6C\xB\x65\x57\x79\x14\x7B\x1F\x7A\x16\x16", 53U);

	for (uint8_t i = 52U; i > NULL; i--) {
		lighting_path[i] = lighting_path[i] ^ lighting_path[i - 1];
	}

	superExtendedDebugLog("lighting: [");

	if (!current_map.lighting.empty()) { traceLog
		for (std::vector<float*>::const_iterator it = current_map.lighting.cbegin();
			it != current_map.lighting.cend();
			it++) {
			if (createLighting) {
				if (*it == nullptr) { traceLog
					superExtendedDebugLog("NULL, ");

					counter_model++;
					continue;
				}

				superExtendedDebugLog("[");
#if CREATE_MODELS
				models[counter_model] = new ModModel;

				models[counter_model]->coords    = *it;
				models[counter_model]->model     = pos_model(lighting_path, *it);
				models[counter_model]->processed = false;
#endif

#if CREATE_LIGHTS
				ModLight* light = new ModLight{
					pos_light(*it, 2),
					*it
				};

				lights.push_back(light);
#endif

				counter_model++;

				superExtendedDebugLog("], ");
			}
			else {
				counter_model++;
			}
		}
	}

	superExtendedDebugLog("], ");

	delete[] lighting_path;

	//LFD section

	char* LFD_path = new char[54];
	memcpy(LFD_path, "\x6F\xD\x67\x2\x61\x15\x66\x49\x39\x58\x2E\x4B\x27\x14\x27\x14\x27\x78\x8\x67\x14\x7D\x9\x60\xF\x61\x12\x3D\x50\x3F\x5B\x3E\x52\x21\xE\x62\xD\x69\x59\x76\x5\x6C\xB\x65\x56\x78\x15\x7A\x1E\x7B\x17\x17", 53U);

	for (uint8_t i = 52U; i > NULL; i--) {
		LFD_path[i] = LFD_path[i] ^ LFD_path[i - 1];
	}

	superExtendedDebugLog("LFD: [");

	if (!current_map.LFD.empty()) {
		for (std::vector<float*>::const_iterator it = current_map.LFD.cbegin();
			it != current_map.LFD.cend();
			it++) {
			if (createLFD) {
				if (*it == nullptr) { traceLog
					superExtendedDebugLog("NULL, ");

					counter_model++;
					continue;
				}

				superExtendedDebugLog("[");
#if CREATE_MODELS
				models[counter_model] = new ModModel;

				models[counter_model]->coords    = *it;
				models[counter_model]->model     = pos_model(LFD_path, *it);
				models[counter_model]->processed = false;
#endif

#if CREATE_LIGHTS
				ModLight* light = new ModLight{
					pos_light(*it, 3),
					*it
				};

				lights.push_back(light);
#endif

				counter_model++;

				superExtendedDebugLog("], ");
			}
			else {
				counter_model++;
			}
		}
	}

	superExtendedDebugLog("], ");

	delete[] LFD_path;

	debugLog("creating OK!");

	return NULL;
}

uint8_t init_models() { traceLog
	if (!isInited 
		|| first_check
		|| request
#if CREATE_MODELS
		|| models.empty()
#endif
		) { traceLog
		return 1;
	}

#if CREATE_MODELS
	debugLog("models adding...");

#if !PATCH_9_22
	//получение BigWorld.player().spaceID

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

#endif

	for (uint8_t i = NULL; i < current_map.minimap_count; i++) {
		if (models[i] == nullptr) continue;

		if (models[i]->model == Py_None || !models[i]->model || models[i]->processed) { traceLog
			Py_XDECREF(models[i]->model);

			models[i]->model  = nullptr;
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

			superExtendedDebugLog("True");
		}
		else superExtendedDebugLog("False");
		
#if PATCH_9_22
		/*
		action = model.action('rotation')
		if(action): action()
		*/

		if (PyObject_HasAttrString(models[i]->model, "action")) { //подстраховываемся
			if (PyObject* action_func = PyObject_CallMethod(models[i]->model, "action", "s", "rotation")) {
				if (PyObject* action = PyObject_CallObject(action_func, NULL))
					Py_DECREF(action);

				Py_DECREF(action_func);

				superExtendedDebugLog("True");
			}
			else superExtendedDebugLog("False");
		}
		else superExtendedDebugLog("animation OFF");
#else
		/*
				clipResource = model.deprecatedGetAnimationClipResource('rotation')
				loader = AnimationSequence.Loader(clipResource, spaceID)
				animator = loader.loadSync()
				animator.bindTo(AnimationSequence.ModelWrapperContainer(model, spaceID))
				animator.speed = animSpeed
				animator.start()

				self.animator = animator
		*/

		

		extendedDebugLog("creating animation...");

		Py_INCREF(models[i]->model);

		if (PyObject_HasAttrString(models[i]->model, "deprecatedGetAnimationClipResource")) { //подстраховываемся
			if (PyObject* clipResource = PyObject_CallMethod(models[i]->model, "deprecatedGetAnimationClipResource", "s", "rotation")) {
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

							extendedDebugLog("creating animation OK!");

							Py_DECREF(binder);
						}
					}

					Py_DECREF(loader);
				}

				Py_DECREF(clipResource);
			}
		}

		Py_DECREF(models[i]->model);
#endif
		
	}
	
	extendedDebugLog("models adding OK!");
#endif

	return NULL;
}

void get_height_offset() { traceLog
	superExtendedDebugLog("Getting height offset...");
	
	PyObject* player = PyObject_CallMethod(BigWorld, "player", NULL);

	if (!player) return;

	PyObject* player_vehicle = PyObject_GetAttrString(player, "vehicle");
	
	Py_DECREF(player);

	if (!player_vehicle) return;

	PyObject* player_vehicle_typeDescriptor = PyObject_GetAttrString(player_vehicle, "typeDescriptor");
	
	Py_DECREF(player_vehicle);

	if (!player_vehicle_typeDescriptor) return;

	PyObject* player_vehicle_typeDescriptor_hull   = PyObject_GetAttrString(player_vehicle_typeDescriptor, "hull");
	PyObject* player_vehicle_typeDescriptor_turret = PyObject_GetAttrString(player_vehicle_typeDescriptor, "turret");

	Py_DECREF(player_vehicle_typeDescriptor);

	if (player_vehicle_typeDescriptor_hull && player_vehicle_typeDescriptor_turret) { traceLog
		PyObject* player_vehicle_typeDescriptor_hull_hitTester = PyObject_GetAttrString(player_vehicle_typeDescriptor_hull, "hitTester");
		PyObject* player_vehicle_typeDescriptor_turret_hitTester = PyObject_GetAttrString(player_vehicle_typeDescriptor_turret, "hitTester");

		Py_DECREF(player_vehicle_typeDescriptor_hull);
		Py_DECREF(player_vehicle_typeDescriptor_turret);

		if (player_vehicle_typeDescriptor_hull_hitTester && player_vehicle_typeDescriptor_turret_hitTester) { traceLog
			PyObject* player_vehicle_typeDescriptor_hull_hitTester_bbox   = PyObject_GetAttrString(player_vehicle_typeDescriptor_hull_hitTester, "bbox");
			PyObject* player_vehicle_typeDescriptor_turret_hitTester_bbox = PyObject_GetAttrString(player_vehicle_typeDescriptor_turret_hitTester, "bbox");

			Py_DECREF(player_vehicle_typeDescriptor_hull_hitTester);
			Py_DECREF(player_vehicle_typeDescriptor_turret_hitTester);

			PyObject* heightIndex = PyInt_FromSize_t(1);

			PyObject* fromBBoxIndex = PyInt_FromSize_t(0);
			PyObject* toBBoxIndex = PyInt_FromSize_t(1);

			double fromHeight = 0.0;
			double toHeight = 0.0;

			if (player_vehicle_typeDescriptor_hull_hitTester_bbox) { traceLog
				PyObject* fromHullBBox = PyObject_GetItem(player_vehicle_typeDescriptor_hull_hitTester_bbox, fromBBoxIndex);

				if (fromHullBBox) { traceLog
					PyObject* fromHullBBoxHeight = PyObject_GetItem(fromHullBBox, heightIndex);

					if (fromHullBBoxHeight) { traceLog
						fromHeight += PyFloat_AS_DOUBLE(fromHullBBoxHeight);
						
						superExtendedDebugLog("fromHeight : %f", fromHeight);

						Py_DECREF(fromHullBBoxHeight);
					}

					Py_DECREF(fromHullBBox);
				}
				else extendedDebugLog("Getting height offset debug 6.1.1 FAILED");

				PyObject* toHullBBox = PyObject_GetItem(player_vehicle_typeDescriptor_hull_hitTester_bbox, toBBoxIndex);

				if (toHullBBox) { traceLog
					PyObject* toHullBBoxHeight = PyObject_GetItem(toHullBBox, heightIndex);

					if (toHullBBoxHeight) { traceLog
						toHeight += PyFloat_AS_DOUBLE(toHullBBoxHeight);
						
						superExtendedDebugLog("toHeight : %f", toHeight);

						Py_DECREF(toHullBBoxHeight);
					}

					Py_DECREF(toHullBBox);
				}
				else extendedDebugLog("Getting height offset debug 6.1.2 FAILED");

				Py_DECREF(player_vehicle_typeDescriptor_hull_hitTester_bbox);
			}
			else extendedDebugLog("Getting height offset debug 6.1 FAILED");

			if (player_vehicle_typeDescriptor_turret_hitTester_bbox) { traceLog
				PyObject* fromTurretBBox = PyObject_GetItem(player_vehicle_typeDescriptor_turret_hitTester_bbox, fromBBoxIndex);

				if (fromTurretBBox) { traceLog
					PyObject* fromTurretBBoxHeight = PyObject_GetItem(fromTurretBBox, heightIndex);

					if (fromTurretBBoxHeight) { traceLog
						fromHeight += PyFloat_AS_DOUBLE(fromTurretBBoxHeight);
						
						superExtendedDebugLog("fromHeight : %f", fromHeight);

						Py_DECREF(fromTurretBBoxHeight);
					}

					Py_DECREF(fromTurretBBox);
				}
				else extendedDebugLog("Getting height offset debug 6.2.1 FAILED");

				PyObject* toTurretBBox = PyObject_GetItem(player_vehicle_typeDescriptor_turret_hitTester_bbox, toBBoxIndex);

				if (toTurretBBox) { traceLog
					PyObject* toTurretBBoxHeight = PyObject_GetItem(toTurretBBox, heightIndex);

					if (toTurretBBoxHeight) { traceLog
						toHeight += PyFloat_AS_DOUBLE(toTurretBBoxHeight);
						
						superExtendedDebugLog("toHeight : %f", toHeight);

						Py_DECREF(toTurretBBoxHeight);
					}

					Py_DECREF(toTurretBBox);
				}
				else extendedDebugLog("Getting height offset debug 6.2.2 FAILED");

				Py_DECREF(player_vehicle_typeDescriptor_turret_hitTester_bbox);
			}
			else extendedDebugLog("Getting height offset debug 6.2 FAILED");

			Py_DECREF(heightIndex);

			Py_DECREF(fromBBoxIndex);
			Py_DECREF(toBBoxIndex);

			//result

			height_offset = 1.0 + toHeight - fromHeight;

			superExtendedDebugLog("height_offset: %f", height_offset);

			superExtendedDebugLog("getting height offset OK!");
		}
		else {
			extendedDebugLog("Getting height offset debug 5 FAILED");

			Py_XDECREF(player_vehicle_typeDescriptor_hull_hitTester);
			Py_XDECREF(player_vehicle_typeDescriptor_turret_hitTester);
		}
	}
	else {
		extendedDebugLog("Getting height offset debug 4 FAILED");

		Py_XDECREF(player_vehicle_typeDescriptor_hull);
		Py_XDECREF(player_vehicle_typeDescriptor_turret);
	}

	extendedDebugLog("Getting height offset end");

	//-----------------------------
}

void get(uint8_t map_ID) { traceLog
	if (!isInited || first_check || !databaseID) return;

	extendedDebugLog("generating token...");

	uint8_t event_id = EventsID.GET_POSITIONS_TOKEN;

	if (isModpack()) event_id = EventsID.GET_POSITIONS_MODPACK; //есть модпак Пираний

	//Py_BEGIN_ALLOW_THREADS;
	request = send_token(databaseID, map_ID, event_id);
	//Py_END_ALLOW_THREADS;

	if (request) { traceLog
		debugLog("Error code %d", request);
#if trace_log
		writeDebugDataToFile(GET, (char*)response_buffer, response_size);
#endif
		return;
	}

	extendedDebugLog("generating token OK!");

	get_height_offset();

	request = create_models();

	if (request) { traceLog
		debugLog("error while creating models: %d", request);
		
		return;
	}

	request = init_models();

	if (request) { traceLog
		debugLog("error while init models: %d", request);

		return;
	}

	return;
};

uint8_t pos_first_check() { traceLog
	if (!isInited || !databaseID) return 1;

	uint8_t event_id = isModpack() ? EventsID.GET_POSITIONS_MODPACK : EventsID.GET_POSITIONS_TOKEN; //есть модпак Пираний

	//Py_BEGIN_ALLOW_THREADS
	if (first_check = send_token(databaseID, NULL, event_id)) { traceLog
		debugLog("Error code %d", first_check);

		writeDebugDataToFile(CHECK, (char*)response_buffer, response_size);

		return 2;
	}
	//Py_END_ALLOW_THREADS

	return NULL;
}

uint8_t del_models() { traceLog
	if (!isInited
		|| first_check
		|| request
#if CREATE_MODELS
		|| models.empty()
#endif
		) return 1;

#if CREATE_MODELS
	extendedDebugLog("models deleting...");

	auto it1 = models.begin();
	while (it1 != models.end()) {
		ModModel* model = *it1;

		if (model == nullptr)
			goto end_cycle_del_models_1;

		if (model->model && model->model != Py_None && model->processed) {
			PyObject* __delModel = PyString_FromString("delModel");

			if (PyObject* result = PyObject_CallMethodObjArgs(BigWorld, __delModel, model->model, NULL))
				Py_DECREF(result);

			Py_DECREF(__delModel);
		}

		Py_XDECREF(model->model);
		model->model = nullptr;

#if !PATCH_9_22
		Py_XDECREF(model->animator);
		model->animator = nullptr;
#endif

		model->coords   = nullptr;

		model->processed = false;

		delete model;
		model = nullptr;
end_cycle_del_models_1:
		models.erase(it1);
	}

	models.~vector();
	
	extendedDebugLog("models deleting OK!");
#endif

	return NULL;
}

#if CREATE_MARKERS
static PyObject* get_minimap(uint8_t markerID) { traceLog
	extendedDebugLog("getting minimap...");

	if (!isInited || first_check || request || !current_map.sections_count || !current_map.minimap_count || minimap == nullptr || !minimap[markerID] || markerID > 2) return PyTuple_New(NULL);

	extendedDebugLog("getting minimap OK!");

	Py_INCREF(minimap[markerID]);

	return minimap[markerID];
};
#endif

void createMarkers() { traceLog
	if (!isInited) return;

	extendedDebugLog("createMarkers...");

#if CREATE_MARKERS
	PyObject* res = NULL;

	if (PyDict_GetItemString(g_self->data, "createMarkers") == Py_True) { traceLog
#if NEW_MINIMAP_MARKERS
	PyObject* minimap_exists = PyObject_GetAttrString(Minimap, "minimapEntries");
#else
	PyObject* minimap_exists = PyObject_GetAttrString(Minimap, "minimap");
#endif

		if (minimap_exists != Py_None) { traceLog
			if (isModelCreated) { traceLog
				if (isMarksCreated) { traceLog
#if NEW_MINIMAP_MARKERS
					res = PyObject_CallMethod(Minimap, "delMarkers", NULL);
#else
					res = PyObject_CallMethod(Minimap, "delMarkerList", NULL);
#endif
				}
				else {
#if NEW_MINIMAP_MARKERS

					char** markerIDs = new char*[3];

					markerIDs[FIRING]   = "yellow";
					markerIDs[LIGHTING] = "green";
					markerIDs[LFD_S]    = "red";

					for (uint8_t i = NULL; i < 3; i++) {
						PyObject* minimap_marks = get_minimap(i);

						res = PyObject_CallMethod(Minimap, "createMinimapPoints", "Os", minimap_marks, markerIDs[i]);

						Py_DECREF(minimap_marks);
					}

					delete[] markerIDs;
#else
					for (uint8_t i = NULL; i < 3; i++) {
						PyObject* minimap_marks = get_minimap(i);

						res = PyObject_CallMethod(Minimap, "createMinimapPoints", "Os", minimap_marks, "DeadPointEntry");

						Py_DECREF(minimap_marks);
					}
#endif
				}

				isMarksCreated = !isMarksCreated;
			}
			else {
				if (isMarksCreated) { traceLog
#if NEW_MINIMAP_MARKERS
					res = PyObject_CallMethod(Minimap, "delMarkers", NULL);
#else
					res = PyObject_CallMethod(Minimap, "delMarkerList", NULL);
#endif
					isMarksCreated = !isMarksCreated;
				}

			}
		}

		Py_DECREF(minimap_exists);
	}

	Py_XDECREF(res);
#endif

	extendedDebugLog("createMarkers end");
}

static PyObject* set_visible(bool isVisible) { traceLog
	if (!isInited || first_check || request) { traceLog
		return PyInt_FromSize_t(1);
	}

#if CREATE_MODELS
	PyObject* py_visible = PyBool_FromLong(isVisible);

	if (models.empty()) return PyInt_FromSize_t(3);

	extendedDebugLog("Models visiblity changing...");

	for (uint16_t i = NULL; i < current_map.minimap_count; i++) {
		if (models[i] == nullptr) continue;

		if (!models[i]->model || models[i]->model == Py_None || !models[i]->processed) { traceLog
			Py_XDECREF(models[i]->model);

			models[i]->model  = nullptr;
			models[i]->coords = nullptr;
			models[i]->processed = false;

			delete models[i];
			models[i] = nullptr;

			continue;
		}

		PyObject_SetAttrString(models[i]->model, "visible", py_visible);
	}

	Py_DECREF(py_visible);

	extendedDebugLog("Models visiblity changing OK!");
#endif

	Py_RETURN_NONE;
};

void createPositions() { traceLog
	isModelCreated = !isModelCreated;
	set_visible(isModelCreated);
}

uint8_t battle_greetings() { traceLog
	uint8_t result = NULL;
	
	if (!isInited || !g_self) { traceLog
		result = 1;
		goto end_battle_greetings_1;
	}

	if (PyDict_GetItemString(g_self->data, "showBattleGreetings") == Py_False) { traceLog
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

	PyObject* message_on    = nullptr;
	PyObject* message_count = nullptr;

	bool delete_message_count = false;

	if (!first_check && PyDict_GetItemString(g_self->data, "enabled") == Py_True) { traceLog
		message_on = PyDict_GetItemString(g_self->i18n, "UI_message_on");

		if (request != 9 && current_map.minimap_count) { traceLog
			PyObject* message_count_raw_p = PyDict_GetItemString(g_self->i18n, "UI_trjcount_0");

			if (message_count_raw_p) { traceLog
				char* message_count_raw_src = PyString_AS_STRING(message_count_raw_p);

				size_t size = strlen(message_count_raw_src);

				char* message_count_raw = new char[size + 6];
				char* counter = new char[5];
				memset(counter, ' ', 4);

				memcpy(message_count_raw, message_count_raw_src, size);

				_itoa_s(current_map.minimap_count, counter, 5, 10);
				memcpy(&message_count_raw[size], counter, 5);
				message_count_raw[size + 5] = NULL;

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

	if (!message_on) { traceLog
		result = 9;
		goto end_battle_greetings_2;
	}

	PyObject* first_attr = Py_None;
	Py_INCREF(first_attr);

	PyObject* __as_showGoldMessageS = PyString_FromString("as_showGoldMessageS");
	PyObject* message_enabled = PyObject_CallMethodObjArgs(BPM, __as_showGoldMessageS, first_attr, message_on, NULL);

	if (!message_enabled) { traceLog
		Py_DECREF(__as_showGoldMessageS);
		
		result = 10;
		goto end_battle_greetings_3;
	}

	if (!message_count) { traceLog
		Py_DECREF(__as_showGoldMessageS);

		result = 11;
		goto end_battle_greetings_4;
	}

	PyObject* message_count_ = PyObject_CallMethodObjArgs(BPM, __as_showGoldMessageS, first_attr, message_count, NULL);

	Py_DECREF(__as_showGoldMessageS);

	if (!message_count_) { traceLog
		result = 12;
		goto end_battle_greetings_5;
	}

	extendedDebugLog("battle greetings OK!");

	Py_DECREF(message_count_);
end_battle_greetings_5:  traceLog
	if (delete_message_count) Py_DECREF(message_count);
end_battle_greetings_4:  traceLog
	Py_DECREF(message_enabled);
end_battle_greetings_3:  traceLog 
	Py_DECREF(first_attr);
end_battle_greetings_2: traceLog
	Py_DECREF(BPM);
end_battle_greetings_1:
	return result;
}

static PyObject* pos_battle_greetings(PyObject *self, PyObject *args) { traceLog
	extendedDebugLog("battle greetings...");

	if (!isInited || !g_self) return PyInt_FromSize_t(1);

	//minimap markers creating on start battle

	if (!first_check                                                       && 
		PyDict_GetItemString(g_self->data, "enabled")           == Py_True &&
		PyDict_GetItemString(g_self->data, "showOnStartBattle") == Py_True) { traceLog
		createPositions();
		createMarkers();
	}

	//------------------

	uint8_t battle_greet = battle_greetings();

	if (battle_greet) return PyInt_FromSize_t(battle_greet);
	else Py_RETURN_NONE;
};

static PyObject* pos_start(PyObject *self, PyObject *args) { traceLog
	if (!isInited || first_check || PyDict_GetItemString(g_self->data, "enabled") == Py_False) { traceLog
		return PyInt_FromSize_t(1);
	}

	PyObject* player = PyObject_CallMethod(BigWorld, "player", NULL);

	isModelCreated = false;
	isMarksCreated = false;

	if (!player) return PyInt_FromSize_t(2);

	PyObject* arena = PyObject_GetAttrString(player, "arena");

	Py_DECREF(player);

	if (!arena) return PyInt_FromSize_t(3);

	PyObject* arenaType = PyObject_GetAttrString(arena, "arenaType");

	Py_DECREF(arena);

	if (!arenaType) return PyInt_FromSize_t(4);

	PyObject* map_PS = PyObject_GetAttrString(arenaType, "geometryName");

	Py_DECREF(arenaType);

	if (!map_PS) return PyInt_FromSize_t(5);

	char* map_s = PyString_AS_STRING(map_PS);

	Py_DECREF(map_PS);

	char map_ID_s[4];
	memcpy(map_ID_s, map_s, 3);
	if (map_ID_s[2] == '_')
		map_ID_s[2] = NULL;
	map_ID_s[3] = NULL;

	mapID = atoi(map_ID_s);

	extendedDebugLog("mapID = %d", (uint32_t)mapID);

	request = NULL;

	get(mapID);

	if (request) { traceLog
		return PyInt_FromSize_t(request);
	}

	set_visible(false);

	Py_RETURN_NONE;
};

static PyObject* pos_fini(PyObject *self, PyObject *args) { traceLog
	if (!isInited || first_check || request || PyDict_GetItemString(g_self->data, "enabled") == Py_False) { traceLog
		return PyInt_FromSize_t(1);
	}

	extendedDebugLog("fini...");

#if CREATE_MARKERS
	if (PyDict_GetItemString(g_self->data, "createMarkers") == Py_True) { traceLog
#if NEW_MINIMAP_MARKERS
		PyObject* minimap_exists = PyObject_GetAttrString(Minimap, "minimapEntries");
#else
		PyObject* minimap_exists = PyObject_GetAttrString(Minimap, "minimap");
#endif

		if (minimap_exists) { traceLog
			if (minimap_exists != Py_None) { traceLog
#if NEW_MINIMAP_MARKERS
				PyObject* res = PyObject_CallMethod(Minimap, "delMarkers", NULL);
#else
				PyObject* res = PyObject_CallMethod(Minimap, "delMarkerList", NULL);
#endif

				Py_XDECREF(res);
			}

            Py_DECREF(minimap_exists);
		}
	}
#endif

	request = NULL;

#if CREATE_LIGHTS
	auto it2 = lights.begin();
	while (it2 != lights.end()) { traceLog
		ModLight* light = *it2;

	    if (light == nullptr)
			goto end_cycle_pos_fini_2;

#if !PATCH_9_22
		if (light->light && light->light != Py_None) { traceLog
			if (PyObject* result_light = PyObject_CallMethod(light->light, "destroyLight", NULL)) { traceLog
				Py_DECREF(result_light);
			}
		} traceLog
#endif

		Py_XDECREF(light->light);

		light->light  = nullptr;
		light->coords = nullptr;

		traceLog

		delete light;
		light = nullptr;
end_cycle_pos_fini_2: traceLog
		it2 = lights.erase(it2);
	} traceLog

	lights.~vector();
#endif

	uint8_t delete_models = del_models();

	if (delete_models) { traceLog
		debugLog("error while deleting models: %d", (uint32_t)delete_models);

		return PyInt_FromSize_t(2);
	}

	current_map.minimap_count = NULL;

	if (current_map.sections_count && current_map.minimap_count) { traceLog
#if CREATE_MARKERS
		if (minimap != nullptr) { traceLog
			for (uint8_t i = NULL; i < 3; i++) {
				if (!minimap[i]) continue;

				size_t count = NULL;

				if      (i == FIRING)   count = current_map.firing.size();
				else if (i == LIGHTING) count = current_map.lighting.size();
				else if (i == LFD_S)    count = current_map.LFD.size();

				for (uint8_t j = NULL; j < count; j++) {
					PyObject* minimap_i_j = PyTuple_GET_ITEM(minimap[i], j);

					for (uint8_t k = NULL; k < 3; k++) {
						Py_XDECREF(PyTuple_GET_ITEM(minimap_i_j, k));
					}

					Py_XDECREF(minimap_i_j);
				}
				
				Py_XDECREF(minimap[i]);
			}

			minimap = nullptr;
		}
#endif

	//Py_BEGIN_ALLOW_THREADS
	clearModelsSections();
	//Py_END_ALLOW_THREADS
	}

	current_map.sections_count = NULL;
	current_map.minimap_count = NULL;
	mapID = NULL;
	height_offset = 0.0;

	extendedDebugLog("fini OK!");

	Py_RETURN_NONE;
};

static PyObject* pos_err_code(PyObject *self, PyObject *args) { traceLog
	if (!isInited) Py_RETURN_NONE;

	return PyInt_FromSize_t(first_check);
};

static PyObject* pos_check(PyObject *self, PyObject *args) { traceLog
	if (!isInited || flag) return PyInt_FromSize_t(1);

	extendedDebugLog("checking...");

	PyObject* player = PyObject_CallMethod(BigWorld, "player", NULL);

	if (!player) return PyInt_FromSize_t(2);

	PyObject* DBID_string = PyObject_GetAttrString(player, "databaseID");

	Py_DECREF(player);

	if (!DBID_string) return PyInt_FromSize_t(3);

	PyObject* DBID_int = PyNumber_Int(DBID_string);

	Py_DECREF(DBID_string);

	if (!DBID_int) { traceLog
		return PyInt_FromSize_t(4);
	}

	databaseID = PyInt_AS_LONG(DBID_int);

	Py_DECREF(DBID_int);

	extendedDebugLog("DBID created");

	uint8_t result = pos_first_check();

	if (result) { traceLog
		return  PyInt_FromSize_t(result);
	}
	else { traceLog
		flag = true;

		Py_RETURN_NONE;
	}
};

static PyObject* pos_init(PyObject *self, PyObject *args) { traceLog
	if (!isInited) return PyInt_FromSize_t(1);

	PyObject* template_;
	PyObject* apply;
	PyObject* byteify;

	if (!PyArg_ParseTuple(args, "OOO", &template_, &apply, &byteify)) { traceLog
		PyInt_FromSize_t(2);
	}

	if (g_gui && PyCallable_Check(template_) && PyCallable_Check(apply)) { traceLog
		Py_INCREF(template_);
		Py_INCREF(apply);

		PyObject* result = PyObject_CallMethod(g_gui, "register", "sOOO", g_self->ids, template_, g_self->data, apply);

		Py_XDECREF(result);
		Py_DECREF(apply);
		Py_DECREF(template_);
	}

	if (!g_gui && PyCallable_Check(byteify)) { traceLog
		Py_INCREF(byteify);

		PyObject* args1 = PyTuple_Pack(1,  g_self->i18n);

		PyObject* result = PyObject_CallObject(byteify, args1);

		Py_DECREF(args1);

		if (result) { traceLog
			PyDict_Clear(g_self->i18n);
			Py_DECREF(g_self->i18n);

			g_self->i18n = result;
		}

		Py_DECREF(byteify);
	}

	Py_RETURN_NONE;
};

static PyObject* pos_hook_setForcedGuiControlMode(PyObject *self, PyObject *args) { traceLog
	if (!isInited || first_check || request)
		Py_RETURN_NONE;

	PyObject* isChatOpened_ = PyTuple_GetItem(args, NULL);

	if (!isChatOpened_)
		Py_RETURN_NONE;

	isChatOpened = PyInt_AsLong(isChatOpened_);

	Py_RETURN_NONE;
};

static PyObject* pos_hideMarkersInBattle(PyObject *self, PyObject *args) { traceLog
	if (!isInited || first_check || request) Py_RETURN_NONE;

#if CREATE_MARKERS

#if NEW_MINIMAP_MARKERS
PyObject* minimap_exists = PyObject_GetAttrString(Minimap, "minimapEntries");
#else
PyObject* minimap_exists = PyObject_GetAttrString(Minimap, "minimap");
#endif

	if (minimap_exists != Py_None && isMarksCreated) { traceLog
#if NEW_MINIMAP_MARKERS
		PyObject* res = PyObject_CallMethod(Minimap, "delMarkers", NULL);
#else
		PyObject* res = PyObject_CallMethod(Minimap, "delMarkerList", NULL);
#endif

		Py_XDECREF(res);
		isMarksCreated = false;
	}
#endif

	Py_RETURN_NONE;
};

static PyObject* pos_inject_handle_key_event(PyObject *self, PyObject *args) { //traceLog
	if (!isInited || first_check || request)
		goto end_pos_inject_handle_key_event;

	PyObject* isKeyGetted_buttonShow    = nullptr;
	PyObject* isKeyGetted_buttonMinimap = nullptr;

	if (isChatOpened)
		goto end_pos_inject_handle_key_event;

	if (g_gui) {
		PyObject* __get_key = PyString_FromString("get_key");

		isKeyGetted_buttonShow    = PyObject_CallMethodObjArgs(g_gui, __get_key, PyDict_GetItemString(g_self->data, "buttonShow"), NULL);
		isKeyGetted_buttonMinimap = PyObject_CallMethodObjArgs(g_gui, __get_key, PyDict_GetItemString(g_self->data, "buttonMinimap"), NULL);

		Py_DECREF(__get_key);
	}
	else {
		PyObject* key = PyObject_GetAttrString(PyTuple_GET_ITEM(args, NULL), "key");

		if (!key)
			goto end_pos_inject_handle_key_event;

		PyObject* ____contains__ = PyString_FromString("__contains__");

		isKeyGetted_buttonShow    = PyObject_CallMethodObjArgs(PyDict_GetItemString(g_self->data, "buttonShow"),    ____contains__, key, NULL);
		isKeyGetted_buttonMinimap = PyObject_CallMethodObjArgs(PyDict_GetItemString(g_self->data, "buttonMinimap"), ____contains__, key, NULL);

		Py_DECREF(____contains__);
	}

	if (isKeyGetted_buttonShow == Py_True) { traceLog
		createPositions();
		createMarkers();

		if (isModelCreated) { traceLog
			battle_greetings();
		}
	}
	Py_XDECREF(isKeyGetted_buttonShow);

	if (isKeyGetted_buttonMinimap == Py_True) { traceLog
		createMarkers();
	}
	Py_XDECREF(isKeyGetted_buttonMinimap);


end_pos_inject_handle_key_event:
	Py_RETURN_NONE;
};

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
	LoadLibraryA("./res_mods/mods/xfw_packages/" MOD_NAME "/native/libeay32.dll");
	LoadLibraryA("./res_mods/mods/xfw_packages/" MOD_NAME "/native/ssleay32.dll");
	LoadLibraryA("./res_mods/mods/xfw_packages/" MOD_NAME "/native/libcurl.dll");

	if (auto err = curl_init()) { traceLog
		debugLogEx(ERROR, "initevent - curl_init: error %d", err);

		goto end_initpos_1;
	}

	BigWorld = PyImport_AddModule("BigWorld");

	if (!BigWorld) 
		goto end_initpos_1;

	debugLog("appLoader init...");

#if PATCH_9_22
	PyObject* appLoader = PyImport_ImportModule("gui.app_loader");

	if (!appLoader) 
		goto end_initpos_1;

	g_appLoader = PyObject_GetAttrString(appLoader, "g_appLoader");

	Py_DECREF(appLoader);

	if (!g_appLoader) 
		goto end_initpos_1;
#else
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
#endif

	debugLog("appLoader init OK");

	json = PyImport_ImportModule("json");

	if (!json)
		goto end_initpos_2;

	debugLog("Config init...");

	if (PyType_Ready(&Config_p))
		goto end_initpos_2;

	Py_INCREF(&Config_p);

	debugLog("Config init OK");

	PyObject* g_config = PyObject_CallObject((PyObject*)&Config_p, NULL);

	Py_DECREF(&Config_p);

	if (!g_config)
		goto end_initpos_2;

	PyObject* pos_module = Py_InitModule("pos", pos_methods);

	if (!pos_module)
		goto end_initpos_2;

	if (PyModule_AddObject(pos_module, "l", g_config))
		goto end_initpos_2;
#if CREATE_MARKERS
	debugLog("Minimap module loading...");

	PyObject* Minimap_module = PyImport_ImportModule(MOD_NAME ".native.Minimap");

	if (!Minimap_module)
		goto end_initpos_2;

	debugLog("Minimap class loading...");

#if NEW_MINIMAP_MARKERS
	Minimap = PyObject_CallMethod(Minimap_module, "Minimap", NULL);
#else
	Minimap = PyObject_CallMethod(Minimap_module, "Minimap", "s", "DeadPointEntry");
#endif

	Py_DECREF(Minimap_module);

	if (!Minimap) 
		goto end_initpos_2;

	debugLog("Minimap class loaded OK!");

#endif

#if !PATCH_9_22
	AnimationSequence = PyImport_ImportModule("AnimationSequence");

	if(!AnimationSequence)
		goto end_initpos_3;
#endif
	debugLog("g_gui module loading...");

	PyObject* mod_mods_gui = PyImport_ImportModule("gui.mods.mod_mods_gui");

	if (!mod_mods_gui) { traceLog
		PyErr_Clear();
		g_gui = nullptr;

		debugLog("mod_mods_gui not initialized");
	}
	else { traceLog
		g_gui = PyObject_GetAttrString(mod_mods_gui, "g_gui");

		Py_DECREF(mod_mods_gui);

		if (!g_gui)
			goto end_initpos_3;

		debugLog("mod_mods_gui loaded OK!");
	}

	traceLog

	if (!g_gui) { traceLog
		_mkdir("mods/configs");
		_mkdir("mods/configs/pavel3333");
		_mkdir("mods/configs/pavel3333/" MOD_NAME);
		_mkdir("mods/configs/pavel3333/" MOD_NAME "/i18n");

		if (!read_data(true) || !read_data(false))
			goto end_initpos_3;
	}
	else { traceLog
#if PATCH_9_22
		PyObject* data_i18n = PyObject_CallMethod(g_gui, "register_data", "sOO",  Config::ids, g_self->data, g_self->i18n);
#else
		PyObject* data_i18n = PyObject_CallMethod(g_gui, "register_data", "sOOs", Config::ids, g_self->data, g_self->i18n, "pavel3333");
#endif

		if (!data_i18n) { traceLog
			Py_DECREF(g_gui);
end_initpos_3: traceLog
#if CREATE_MARKERS
			Py_DECREF(Minimap);
#endif
end_initpos_2: traceLog
			Py_DECREF(g_appLoader);
end_initpos_1: traceLog
			return;
		}

		PyDict_Clear(g_self->data);
		PyDict_Clear(g_self->i18n);

		Py_DECREF(g_self->data);
		Py_DECREF(g_self->i18n);

		g_self->data = PyTuple_GET_ITEM(data_i18n, 0);
		g_self->i18n = PyTuple_GET_ITEM(data_i18n, 1);

		Py_DECREF(data_i18n);
	} traceLog

	isInited = true;
};