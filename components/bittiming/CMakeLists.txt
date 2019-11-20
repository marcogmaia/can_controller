# idf_component_register(
#     SRCS "bittiming.c"
#     INCLUDE_DIRS "."
#     REQUIRES
# )

set(COMPONENT_REQUIRES 
    # bittiming
    # can_controller
)
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "bittiming.c")
set(COMPONENT_ADD_INCLUDEDIRS "." )

register_component()