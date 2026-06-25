# Disable building of shared library
#sed -i 's/add\_library(uvwasi SHARED/# /g' CMakeLists.txt
mkdir build
cd build
cmake ../
make uvwasi_a

LIBUV_A="$(find . -name 'libuv_a.a' -o -name 'libuv.a' | head -n1)"

$CC $CFLAGS $LIB_FUZZING_ENGINE ../.clusterfuzzlite/fuzz_normalize_path.c \
  -o $OUT/fuzz_normalize_path \
  ./libuvwasi.a "$LIBUV_A" \
  -I$SRC/uvwasi/include -I$PWD/_deps/libuv-src/include/
