#pragma once

#include "py_pch.h"
#include "Config.h"

namespace PyImports {
    static py::module AnimationSequence = py::none();
    static py::module BigWorld = py::none();

    static py::module dependency = py::none();
    static py::module minimap = py::none();
    static py::module mod_mods_gui = py::none();
    static py::module imp = py::none();
    static py::module json = py::none();

    static py::module appLoader = py::none();
};

namespace Globals {
    static double vehicleHeight = 0.0;
}

namespace PyGlobals {
    static py::object g_appLoader = py::none();
    static py::object g_minimap = py::none();
    static py::object g_gui = py::none();

    static PyConfig g_config;
};