# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES 
    # bittiming
    can_controller
)
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "./src/main.c")
set(COMPONENT_ADD_INCLUDEDIRS "." "./include")

register_component()
