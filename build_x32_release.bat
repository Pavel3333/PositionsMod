chcp 1251
rd /s /q "build/Win32"
mkdir "build/Win32"
cd "build/Win32"
cmake ../.. -DCMAKE_PREFIX_PATH="C:/WoTPython/Win32/" -DCMAKE_GENERATOR_PLATFORM=Win32 -DCMAKE_INSTALL_PREFIX="../../deploy/Win32"
cmake --build . --target INSTALL --config Release
pause