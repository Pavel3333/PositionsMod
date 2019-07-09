#pragma once
#include "API_functions.h"

#include "CConfig.h"
#include "python2.7/Python.h"
#include "python2.7/structmember.h"

typedef struct {
	PyObject_HEAD
		char* ids;
	char* patch;
	char* author;
	char* version;
	uint16_t version_id;
	PyObject* buttons;
	PyObject* data;
	PyObject* i18n;
} ConfigObject;

ConfigObject* g_self = NULL;

void init_config() {
	Config::i18n.version = Config::version_id;
	Config::data.version = Config::version_id;
	Config::data.buttonShow = Config::buttons.buttonShow;
	Config::data.buttonMinimap = Config::buttons.buttonMinimap;
}

PyObject* init_buttons() {
	PyObject* buttons = PyDict_New();
	if (!buttons)
		goto end_init_buttons_1;

	////////////////////////Show button tuple////////////////////////

	PyObject* buttonShow = PyInt_FromSize_t(Config::data.buttonShow);

	PyObject* buttonShow_tuple = PyTuple_Pack(1, buttonShow);
	if (!buttonShow_tuple) {
		Py_DECREF(buttonShow);
		goto end_init_buttons_1;
	}

	if (PyDict_SetItemString(buttons, "buttonShow", buttonShow_tuple)) {
		Py_DECREF(buttonShow_tuple);
		goto end_init_buttons_1;
	}

	////////////////////////Minimap button tuple////////////////////////

	PyObject* buttonMinimap = PyInt_FromSize_t(Config::data.buttonMinimap);

	PyObject* buttonMinimap_tuple = PyTuple_Pack(1, buttonMinimap);
	if (!buttonMinimap_tuple) {
		Py_DECREF(buttonMinimap);
		goto end_init_buttons_1;
	}

	if (PyDict_SetItemString(buttons, "buttonMinimap", buttonMinimap_tuple)) {
		Py_DECREF(buttonMinimap_tuple);
end_init_buttons_1:
		return NULL;
	}

	return buttons;
}

PyObject* init_data() {
	PyObject* data = PyDict_New();
	if (!data)
		return NULL;

	PyObject* data_version = PyInt_FromSize_t(Config::data.version);

	if (PyDict_SetItemString(data, "version", data_version)) {
		Py_DECREF(data_version);
		goto end_init_data;
	}

	PyObject* data_enabled = PyBool_FromLong(Config::data.enabled);

	if (PyDict_SetItemString(data, "enabled", data_enabled)) {
		Py_DECREF(data_enabled);
		goto end_init_data;
	}

	PyObject* data_showOnStartBattle = PyBool_FromLong(Config::data.showOnStartBattle);

	if (PyDict_SetItemString(data, "showOnStartBattle", data_showOnStartBattle)) {
		Py_DECREF(data_showOnStartBattle);
		goto end_init_data;
	}

	PyObject* data_showBattleGreetings = PyBool_FromLong(Config::data.showBattleGreetings);

	if (PyDict_SetItemString(data, "showBattleGreetings", data_showBattleGreetings)) {
		Py_DECREF(data_showBattleGreetings);
		goto end_init_data;
	}

	PyObject* data_createMarkers = PyBool_FromLong(Config::data.createMarkers);

	if (PyDict_SetItemString(data, "createMarkers", data_createMarkers)) {
		Py_DECREF(data_createMarkers);
		goto end_init_data;
	}

	PyObject* data_createLighting = PyBool_FromLong(Config::data.createLighting);

	if (PyDict_SetItemString(data, "createLighting", data_createLighting)) {
		Py_DECREF(data_createLighting);
		goto end_init_data;
	}

	PyObject* data_createFiring = PyBool_FromLong(Config::data.createFiring);

	if (PyDict_SetItemString(data, "createFiring", data_createFiring)) {
		Py_DECREF(data_createFiring);
		goto end_init_data;
	}

	PyObject* data_createLFD = PyBool_FromLong(Config::data.createLFD);

	if (PyDict_SetItemString(data, "createLFD", data_createLFD)) {
		Py_DECREF(data_createLFD);
		goto end_init_data;
	}

	PyObject* data_playAnimation = PyBool_FromLong(Config::data.playAnimation);

	if (PyDict_SetItemString(data, "playAnimation", data_playAnimation)) {
		Py_DECREF(data_playAnimation);
		goto end_init_data;
	}

	PyObject* data_newModels = PyBool_FromLong(Config::data.newModels);

	if (PyDict_SetItemString(data, "newModels", data_newModels)) {
		Py_DECREF(data_newModels);
		goto end_init_data;
	}

	PyObject* data_hideMarkersInBattle = PyBool_FromLong(Config::data.hideMarkersInBattle);

	if (PyDict_SetItemString(data, "hideMarkersInBattle", data_hideMarkersInBattle)) {
		Py_DECREF(data_hideMarkersInBattle);
end_init_data:
		Py_DECREF(data);

		return NULL;
	}

	return data;
}

PyObject* init_i18n() {
	PyObject* i18n = PyDict_New();
	if (!i18n)
		return NULL;

	PyObject* i18n_version = PyInt_FromSize_t(Config::i18n.version);

	if (PyDict_SetItemString(i18n, "version", i18n_version)) {
		Py_DECREF(i18n_version);
		goto end_init_i18n;
	}

	PyObject* UI_description = PyString_FromString("Free Positions Mod");
	PyObject* empty_tooltip = PyString_FromStringAndSize("", NULL);
	PyObject* UI_setting_buttonShow_text = PyString_FromString("Button: Show positions");
	PyObject* UI_setting_buttonMinimap_text = PyString_FromString("Button: Show minimap markers");
	PyObject* UI_setting_showOnStartBattle_text = PyString_FromString("Show on Start Battle");
	PyObject* UI_setting_hideMarkersInBattle_text = PyString_FromString("Hide markers on start battle");
	PyObject* UI_setting_showBattleGreetings_text = PyString_FromString("Show message on start battle");
	PyObject* UI_setting_createMarkers_text = PyString_FromString("Create markers on minimap");
	PyObject* UI_setting_createLighting_text = PyString_FromString("Create lighting positions");
	PyObject* UI_setting_createFiring_text = PyString_FromString("Create firing positions");
	PyObject* UI_setting_createLFD_text = PyString_FromString("Create LFD positions");
	PyObject* UI_setting_playAnimation_text = PyString_FromString("Play animation");
	PyObject* UI_setting_newModels_text = PyString_FromString("New models");
	PyObject* UI_message_thx = PyString_FromString("Positions Mod: Loaded 758 positions.");
	PyObject* UI_message_thx_2 = PyString_FromString("Official site of Positions mod");
	PyObject* UI_message_channel = PyString_FromString("Official channel RAINN VOD of Positions mod");
	PyObject* UI_message_trj_ad = PyString_FromString("Like Positions Mod? ");
	PyObject* UI_message_trj_ad_2 = PyString_FromString("Try out the free Trajectory Mod");
	PyObject* UI_message_on = PyString_FromString("Mod Positions Free ON");
	PyObject* UI_message_off = PyString_FromString("Mod Positions Free OFF");
	PyObject* UI_poscount_0 = PyString_FromString("Positions count: %d");
	PyObject* UI_poscount_1 = PyString_FromString("There aren't any positions!");
	PyObject* UI_err_2 = PyString_FromString("Error. Redownload mod ");
	PyObject* UI_err_3 = PyString_FromString("Error. Maybe network don't works?");
	PyObject* UI_err_6 = PyString_FromString("Error. Redownload mod ");
	PyObject* UI_link = PyString_FromString("from official site");

	if (PyDict_SetItemString(i18n, "UI_description", UI_description) ||
		PyDict_SetItemString(i18n, "UI_setting_buttonShow_text", UI_setting_buttonShow_text) ||
		PyDict_SetItemString(i18n, "UI_setting_buttonShow_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_buttonMinimap_text", UI_setting_buttonMinimap_text) ||
		PyDict_SetItemString(i18n, "UI_setting_buttonMinimap_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_showOnStartBattle_text", UI_setting_showOnStartBattle_text) ||
		PyDict_SetItemString(i18n, "UI_setting_showOnStartBattle_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_hideMarkersInBattle_text", UI_setting_hideMarkersInBattle_text) ||
		PyDict_SetItemString(i18n, "UI_setting_hideMarkersInBattle_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_showBattleGreetings_text", UI_setting_showBattleGreetings_text) ||
		PyDict_SetItemString(i18n, "UI_setting_showBattleGreetings_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_createMarkers_text", UI_setting_createMarkers_text) ||
		PyDict_SetItemString(i18n, "UI_setting_createMarkers_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_createLighting_text", UI_setting_createLighting_text) ||
		PyDict_SetItemString(i18n, "UI_setting_createLighting_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_createFiring_text", UI_setting_createFiring_text) ||
		PyDict_SetItemString(i18n, "UI_setting_createFiring_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_createLFD_text", UI_setting_createLFD_text) ||
		PyDict_SetItemString(i18n, "UI_setting_createLFD_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_playAnimation_text", UI_setting_playAnimation_text) ||
		PyDict_SetItemString(i18n, "UI_setting_playAnimation_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_setting_newModels_text", UI_setting_newModels_text) ||
		PyDict_SetItemString(i18n, "UI_setting_newModels_tooltip", empty_tooltip) ||
		PyDict_SetItemString(i18n, "UI_message_thx", UI_message_thx) ||
		PyDict_SetItemString(i18n, "UI_message_thx_2", UI_message_thx_2) ||
		PyDict_SetItemString(i18n, "UI_message_channel", UI_message_channel) ||
		PyDict_SetItemString(i18n, "UI_message_off", UI_message_off) ||
		PyDict_SetItemString(i18n, "UI_message_on", UI_message_on) ||
		PyDict_SetItemString(i18n, "UI_message_trj_ad", UI_message_trj_ad) ||
		PyDict_SetItemString(i18n, "UI_message_trj_ad_2", UI_message_trj_ad_2) ||
		PyDict_SetItemString(i18n, "UI_poscount_0", UI_poscount_0) ||
		PyDict_SetItemString(i18n, "UI_poscount_1", UI_poscount_1) ||
		PyDict_SetItemString(i18n, "UI_err_2", UI_err_2) ||
		PyDict_SetItemString(i18n, "UI_err_3", UI_err_3) ||
		PyDict_SetItemString(i18n, "UI_err_6", UI_err_6) ||
		PyDict_SetItemString(i18n, "UI_link", UI_link)
		) {
end_init_i18n:
		Py_DECREF(i18n);
		return NULL;
	}

	return i18n;
}

PyObject * Config_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	init_config();

	g_self = (ConfigObject*)type->tp_alloc(type, 0);

	if (g_self) {

		g_self->ids = Config::ids;
		if (!g_self->ids)
			goto end_Config_new_1;

		g_self->patch = Config::patch;
		if (!g_self->patch)
			goto end_Config_new_1;

		g_self->author = Config::author;
		if (!g_self->author)
			goto end_Config_new_1;

		g_self->version = Config::version;
		if (!g_self->version)
			goto end_Config_new_1;

		g_self->version_id = Config::version_id;
		if (!g_self->version_id)
			goto end_Config_new_1;

		////////////////////////Buttons dict////////////////////////

		g_self->buttons = init_buttons();

		if (!g_self->buttons)
			goto end_Config_new_1;

		////////////////////////Markers list////////////////////////

		g_self->data = init_data();

		if (!g_self->data)
			goto end_Config_new_1;

		PyObject* buttons_buttonShow = PyDict_GetItemString(g_self->buttons, "buttonShow");

		if (!buttons_buttonShow)
			goto end_Config_new_1;

		if (PyDict_SetItemString(g_self->data, "buttonShow", buttons_buttonShow)) {
			Py_DECREF(buttons_buttonShow);
			goto end_Config_new_1;
		}

		PyObject* buttons_buttonMinimap = PyDict_GetItemString(g_self->buttons, "buttonMinimap");

		if (!buttons_buttonMinimap)
			goto end_Config_new_2;

		if (PyDict_SetItemString(g_self->data, "buttonMinimap", buttons_buttonMinimap)) {
			Py_DECREF(buttons_buttonMinimap);
end_Config_new_2:
			Py_DECREF(buttons_buttonShow);
end_Config_new_1:
			Py_DECREF(g_self);
			return NULL;
		}

		g_self->i18n = init_i18n();

	}

	return (PyObject *)g_self;
}

void Config_dealloc(ConfigObject* self)
{
	if (self->buttons) {
		PyDict_Clear(self->buttons);

		Py_DECREF(self->buttons);
	}

	if (self->data) {
		PyDict_Clear(self->data);

		Py_DECREF(self->data);
	}

	if (self->i18n) {
		PyDict_Clear(self->i18n);

		Py_DECREF(self->i18n);
	}

	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMemberDef config_members[] = {
	{ "ids", T_STRING, offsetof(ConfigObject, ids), NULL },
	{ "author", T_STRING, offsetof(ConfigObject, author), NULL },
	{ "patch", T_STRING, offsetof(ConfigObject, patch), NULL },
	{ "version", T_STRING, offsetof(ConfigObject, version), NULL },
	{ "version_id", T_SHORT, offsetof(ConfigObject, version_id), NULL },
	{ "buttons", T_OBJECT, offsetof(ConfigObject, buttons), NULL },
	{ "data", T_OBJECT, offsetof(ConfigObject, data), NULL },
	{ "i18n", T_OBJECT, offsetof(ConfigObject, i18n), NULL },
	{ NULL }
};

PyTypeObject Config_p{
	PyVarObject_HEAD_INIT(NULL, 0)
	"pos.Config",					  /*tp_name*/
	sizeof(ConfigObject),             /*tp_basicsize*/

									  /* Methods to implement standard operations */

	NULL,
	(destructor)Config_dealloc,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	/* Method suites for standard classes */

	NULL,
	NULL,
	NULL,

	/* More standard operations (here for binary compatibility) */

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	/* Functions to access object as input/output buffer */
	NULL,

	/* Flags to define presence of optional/expanded features */
	Py_TPFLAGS_DEFAULT,
	"Config for Free Positions Mod", /* Documentation string */

								/* Assigned meaning in release 2.0 */
								/* call function for all accessible objects */
	NULL,

	/* delete references to contained objects */
	NULL,

	/* Assigned meaning in release 2.1 */
	/* rich comparisons */
	NULL,

	/* weak reference enabler */
	NULL,

	/* Added in release 2.2 */
	/* Iterators */
	NULL,
	NULL,

	/* Attribute descriptor and subclassing stuff */
	NULL, //struct PyMethodDef *tp_methods;
	config_members, //struct PyMemberDef *tp_members;
	NULL, //struct PyGetSetDef *tp_getset;
	NULL, //struct _typeobject *tp_base;
	NULL, //PyObject *tp_dict;
	NULL,
	NULL,
	NULL,
	NULL,//(initproc)Config_init,
	NULL,
	Config_new
};
