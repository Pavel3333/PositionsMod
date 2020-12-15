chcp 1251
rd /s /q "build_x64"
mkdir build_x64
cd build_x64
cmake .. -DCMAKE_PREFIX_PATH="C:/WoTPython/x64/" -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_INSTALL_PREFIX="C:/Games/World_of_Tanks_RU/res_mods/mods/xfw_packages/Autoupdater_Main/native_64bit"
cmake --build . --target INSTALL --config RelWithDebInfo
pause