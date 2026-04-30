#!/bin/bash
emcc -O3 -std=c++11 \
    -Isrc \
    src/pcst_fast.cc \
    src/pcst_fast_wasm.cc \
    --bind \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MALLOC=emmalloc \
    -s MODULARIZE=1 \
    -s 'EXPORT_NAME="PCSTFastModule"' \
    -s NODEJS_CATCH_EXIT=0 \
    -s NODEJS_CATCH_REJECTION=0 \
    -o pcst_fast.js
