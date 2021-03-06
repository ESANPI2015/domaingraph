cmake_minimum_required(VERSION 3.2)

add_executable(testComponentNetwork testComponentNetwork.cpp)
target_link_libraries(testComponentNetwork ${PROJECT_NAME})
install(TARGETS testComponentNetwork RUNTIME DESTINATION bin)
add_test(testComponentNetwork testComponentNetwork)
add_custom_command(TARGET testComponentNetwork POST_BUILD COMMAND testComponentNetwork)

add_executable(testSoftwareNetwork testSoftwareNetwork.cpp)
target_link_libraries(testSoftwareNetwork ${PROJECT_NAME})
install(TARGETS testSoftwareNetwork RUNTIME DESTINATION bin)
add_test(testSoftwareNetwork testSoftwareNetwork)
add_custom_command(TARGET testSoftwareNetwork POST_BUILD COMMAND testSoftwareNetwork)

add_executable(testHardwareComputationalNetwork testHardwareComputationalNetwork.cpp)
target_link_libraries(testHardwareComputationalNetwork ${PROJECT_NAME})
install(TARGETS testHardwareComputationalNetwork RUNTIME DESTINATION bin)
add_test(testHardwareComputationalNetwork testHardwareComputationalNetwork)
add_custom_command(TARGET testHardwareComputationalNetwork POST_BUILD COMMAND testHardwareComputationalNetwork)

add_executable(testMapping testMapping.cpp)
target_link_libraries(testMapping ${PROJECT_NAME})
install(TARGETS testMapping RUNTIME DESTINATION bin)
add_test(testMapping testMapping)
add_custom_command(TARGET testMapping POST_BUILD COMMAND testMapping)

add_executable(createEmptyNetworks createEmptyNetworks.cpp)
target_link_libraries(createEmptyNetworks ${PROJECT_NAME})
install(TARGETS createEmptyNetworks RUNTIME DESTINATION bin)

add_executable(testGenerator testGenerator.cpp)
target_link_libraries(testGenerator ${PROJECT_NAME})
install(TARGETS testGenerator RUNTIME DESTINATION bin)
