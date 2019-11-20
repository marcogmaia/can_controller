# idf_component_register(
#     SRCS
#     INCLUDE_DIRS "."
#     REQUIRES
# )

set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
    "decoder.c"
    "encoder.c"
)

set(COMPONENT_ADD_INCLUDEDIRS "." "./include")