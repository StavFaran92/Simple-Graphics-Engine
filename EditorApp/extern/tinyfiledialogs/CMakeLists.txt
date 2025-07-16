add_library("tinyfiledialogs" STATIC tinyfiledialogs.c)
target_compile_options("tinyfiledialogs" PUBLIC /MT)

set_target_properties(tinyfiledialogs PROPERTIES LINKER_LANGUAGE C)