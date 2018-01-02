SET builddir=%SRC_DIR%\..\build_eman
if errorlevel 1 exit 1

mkdir %builddir% && cd %builddir%
if errorlevel 1 exit 1

cmake "%SRC_DIR%" -G "Visual Studio 9 2008 Win64" ^
                    -DCMAKE_BUILD_TYPE=Release

cmake --build "%builddir%" --config Release --target install

cmake --build "%builddir%" --config Release --target test-verbose
