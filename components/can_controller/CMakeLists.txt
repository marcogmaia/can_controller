# idf_component_register(
#     SRCS
#     INCLUDE_DIRS "."
#     REQUIRES
# )

set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
    "src/can_utils.c"
    "src/decoder.c"
    "src/encoder.c"
    "src/bittiming.c"
    "src/can_controller.c"
    "src/transmitter.c"
    "src/receiver.c"
)

set(COMPONENT_ADD_INCLUDEDIRS "." "include")

register_component()