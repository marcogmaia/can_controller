# idf_component_register(
#     SRCS
#     INCLUDE_DIRS "."
#     REQUIRES
# )

set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
    "can_utils.c"
    "decoder.c"
    "encoder.c"
    "bittiming.c"
    "can_controller.c"
)

set(COMPONENT_ADD_INCLUDEDIRS "." "include")

register_component()