#!/bin/bash

cd ../../../emsdk
source ./emsdk_env.sh
cd ../Projects/snake
emcc -o bin/Web/snake.html game/src/main.cpp -Wall -std=c++14 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os -I. -I raylib-master/src -I raylib-master/src/external -L. -L raylib-master/src -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 --preload-file game/src/assets/backgrounds --preload-file game/src/assets/Sounds --preload-file game/src/assets/ --shell-file raylib-master/src/shell.html web/libraylib.a -DPLATFORM_WEB -s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' -s EXPORTED_RUNTIME_METHODS=ccall
cd bin/Web
python3 -m http.server 8080