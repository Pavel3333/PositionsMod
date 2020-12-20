chcp 1251

rd /s /q "build/Win32"
mkdir "build/Win32"
cd "build/Win32"
cmake ../.. -DCMAKE_PREFIX_PATH="C:/WoTPython/Win32/" -DCMAKE_GENERATOR_PLATFORM=Win32 -DCMAKE_INSTALL_PREFIX="../../deploy/Win32"
cmake --build . --target INSTALL --config Release
cd ../../

rd /s /q "build/x64"
mkdir "build/x64"
cd "build/x64"
cmake ../.. -DCMAKE_PREFIX_PATH="C:/WoTPython/x64/" -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_INSTALL_PREFIX="../../deploy/x64"
cmake --build . --target INSTALL --config Release
cd ../../

pause