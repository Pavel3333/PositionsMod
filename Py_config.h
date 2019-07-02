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

extern ConfigObject* g_self;

void init_config();

PyObject* init_buttons();

PyObject* init_data();

PyObject* init_i18n();

PyObject * Config_new(PyTypeObject*, PyObject*, PyObject*);

void Config_dealloc(ConfigObject* self);

PyMemberDef config_members[];

extern PyTypeObject Config_p;