project(gsl)

add_library(${PROJECT_NAME} INTERFACE)

cmc_target_current_include_directory(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets)

include(cmc-install)
cmc_install_with_folders(FILES ${${PROJECT_NAME}_HEADERS}
                         DESTINATION "include/${PROJECT_NAME}/${PROJECT_NAME}")
cmc_install_packageconfig(${PROJECT_NAME} ${PROJECT_NAME}Targets
                          NAMESPACE ${CMAKE_PROJECT_NAME}::)
