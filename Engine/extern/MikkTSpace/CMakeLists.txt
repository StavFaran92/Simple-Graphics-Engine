add_library("MikkTSpace" STATIC mikktspace.c mikktspace.h)
target_compile_options("MikkTSpace" PUBLIC /MT)

set_target_properties(MikkTSpace PROPERTIES LINKER_LANGUAGE C)