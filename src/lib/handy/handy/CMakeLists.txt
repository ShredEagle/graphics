set(TARGET_NAME handy)

set(${TARGET_NAME}_HEADERS
    array_utils.h
    Base64.h
    Bitmask.h
    Crc.h
    Guard.h
    Observer.h
    Pool.h
    random.h
    StringId.h
    StringId_Interning.h
    tuple_utils.h
    vector_utils.h
    ZeroOnMove.h

    3rdparty/beast_base64.h
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}
             FILES ${${TARGET_NAME}_HEADERS} ${${TARGET_NAME}_SOURCES}
)

add_library(${TARGET_NAME} INTERFACE)

add_library(ad::${TARGET_NAME} ALIAS ${TARGET_NAME})

# Custom target to have the files show up in the IDE
add_custom_target(${TARGET_NAME}_IDE
    SOURCES
        ${${TARGET_NAME}_HEADERS}
)

cmc_target_current_include_directory(${TARGET_NAME})


##
## Install
##

install(TARGETS ${TARGET_NAME}
        EXPORT ${TARGET_NAME}Targets)

include(cmc-install)
# Install the header files
cmc_install_with_folders(DESTINATION include/${TARGET_NAME}/${TARGET_NAME}
                         FILES ${${TARGET_NAME}_HEADERS})
# Prepare CMake package in both build and install trees
cmc_install_packageconfig(${TARGET_NAME} ${TARGET_NAME}Targets ${PROJECT_NAME}
                          NAMESPACE ad::)
