# Disable building of shared library
#sed -i 's/add\_library(uvwasi SHARED/# /g' CMakeLists.txt
mkdir build
cd build
cmake ../
make uvwasi_a

$CC $CFLAGS $LIB_FUZZING_ENGINE ../.clusterfuzzlite/fuzz_normalize_path.c \
  -o $OUT/fuzz_normalize_path \
  ./libuvwasi_a.a _deps/libuv-build/libuv_a.a \
  -I$SRC/uvwasi/include -I$PWD/_deps/libuv-src/include/
