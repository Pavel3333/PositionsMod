chcp 1251
rd /s /q "build_x86"
mkdir build_x86
cd build_x86
cmake .. -DCMAKE_PREFIX_PATH="C:/WoTPython/Win32/" -DCMAKE_GENERATOR_PLATFORM=Win32 -DCMAKE_INSTALL_PREFIX="C:/Games/World_of_Tanks_RU/res_mods/mods/xfw_packages/Autoupdater_Main/native_32bit"
cmake --build . --target INSTALL --config RelWithDebInfo
pause