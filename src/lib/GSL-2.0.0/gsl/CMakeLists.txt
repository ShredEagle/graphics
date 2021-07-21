set(TARGET_NAME gsl)

set(${TARGET_NAME}_HEADERS
    gsl
    gsl_algorithm
    gsl_assert
    gsl_byte
    gsl_util
    multi_span
    pointers
    span
    string_span
)

add_library(${TARGET_NAME} INTERFACE)

add_library(${TARGET_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

cmc_target_current_include_directory(${TARGET_NAME})

##
## Install
##
install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME}Targets)

include(cmc-install)
cmc_install_with_folders(FILES ${${TARGET_NAME}_HEADERS}
                         DESTINATION "include/${TARGET_NAME}/${TARGET_NAME}")
cmc_install_packageconfig(${TARGET_NAME} ${TARGET_NAME}Targets ${PROJECT_NAME}
                          NAMESPACE ${TARGET_NAME}::)
