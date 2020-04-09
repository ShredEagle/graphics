project(gsl)

set(${PROJECT_NAME}_HEADERS
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

add_library(${PROJECT_NAME} INTERFACE)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

cmc_target_current_include_directory(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets)

include(cmc-install)
cmc_install_with_folders(FILES ${${PROJECT_NAME}_HEADERS}
                         DESTINATION "include/${PROJECT_NAME}/${PROJECT_NAME}")
cmc_install_packageconfig(${PROJECT_NAME} ${PROJECT_NAME}Targets
                          NAMESPACE ${PROJECT_NAME}::)
