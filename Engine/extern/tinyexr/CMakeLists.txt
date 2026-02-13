add_library("tinyexr" STATIC miniz.c)
target_compile_options("tinyexr" PUBLIC /MT)