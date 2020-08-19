cmake_minimum_required(VERSION 3.5)
project(urdf)

find_package(ament_cmake_ros REQUIRED)
find_package(pluginlib REQUIRED)
find_package(urdf_parser_plugin REQUIRED)
find_package(urdfdom REQUIRED)
find_package(urdfdom_headers REQUIRED)
find_package(tinyxml_vendor REQUIRED)
find_package(TinyXML REQUIRED)

# Find version components
if(NOT urdfdom_headers_VERSION)
set(urdfdom_headers_VERSION "0.0.0")
endif()
string(REGEX REPLACE "^([0-9]+).*" "\\1" URDFDOM_HEADERS_MAJOR_VERSION "${urdfdom_headers_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1" URDFDOM_HEADERS_MINOR_VERSION "${urdfdom_headers_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" URDFDOM_HEADERS_REVISION_VERSION "${urdfdom_headers_VERSION}")
set(generated_install_path "${CMAKE_CURRENT_BINARY_DIR}/include")
set(generated_compat_header "${generated_install_path}/${PROJECT_NAME}/urdfdom_compatibility.h")
include_directories(${generated_install_path})
configure_file(urdfdom_compatibility.h.in "${generated_compat_header}" @ONLY)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

install(FILES ${generated_compat_header} DESTINATION include/${PROJECT_NAME})

add_library(${PROJECT_NAME} src/model.cpp)
target_include_directories(${PROJECT_NAME}
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
)
ament_target_dependencies(${PROJECT_NAME}
  urdf_parser_plugin
  urdfdom
  urdfdom_headers
  pluginlib
  TinyXML)

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE "URDF_BUILDING_DLL")
endif()

if(APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif()

add_library(urdf_xml_parser SHARED
  src/urdf_plugin.cpp
)
target_link_libraries(urdf_xml_parser
  ${PROJECT_NAME}
)
ament_target_dependencies(urdf_xml_parser
  "pluginlib"
  "urdf_parser_plugin"
)

install(TARGETS urdf_xml_parser
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_cppcheck REQUIRED)
  find_package(ament_cmake_cpplint REQUIRED)
  find_package(ament_cmake_google_benchmark REQUIRED)
  find_package(ament_cmake_lint_cmake REQUIRED)
  find_package(ament_cmake_uncrustify REQUIRED)
  # This forces cppcheck to consider all files in this project to be C++,
  # including the headers which end with .h, which cppcheck would normally
  # consider to be C instead.
  ament_cppcheck(LANGUAGE "c++")
  ament_cpplint()
  ament_lint_cmake()
  ament_uncrustify(LANGUAGE "C++")

  # Write files to enable pluginlib to find urdf_xml_parser in build folder
  set(fake_urdf_prefix "${CMAKE_CURRENT_BINARY_DIR}/fake_urdf_install")
  # Copy urdf_xml_parser to fake prefix lib/ and bin/
  file(MAKE_DIRECTORY "${fake_urdf_prefix}/lib")
  file(MAKE_DIRECTORY "${fake_urdf_prefix}/bin")
  add_custom_command(TARGET urdf_xml_parser
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:urdf_xml_parser>
    "${fake_urdf_prefix}/lib/"
  )
  add_custom_command(TARGET urdf_xml_parser
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:urdf_xml_parser>
    "${fake_urdf_prefix}/bin/"
  )
  # Fake install plugin description file
  file(COPY "urdf_parser_description.xml" DESTINATION "${fake_urdf_prefix}/share/urdf/")
  # Fake install package.xml
  configure_file("package.xml" "${fake_urdf_prefix}/share/urdf/package.xml" COPYONLY)
  # Fake install entry in ament_index telling pluginlib where to find plugin description
  file(WRITE "${fake_urdf_prefix}/share/ament_index/resource_index/urdf_parser_plugin__pluginlib__plugin/urdf"
    "share/urdf/urdf_parser_description.xml\n")
  # Fake install entry in ament_index declaring this a package
  file(WRITE "${fake_urdf_prefix}/share/ament_index/resource_index/packages/urdf" "")

  ament_add_google_benchmark(plugin_overhead
    test/benchmark_plugin_overhead.cpp
    APPEND_ENV AMENT_PREFIX_PATH="${fake_urdf_prefix}"
  )
  target_link_libraries(plugin_overhead
    urdf
  )
endif()

ament_export_libraries(${PROJECT_NAME})
ament_export_targets(${PROJECT_NAME})
ament_export_include_directories(include)
ament_export_dependencies(pluginlib)
ament_export_dependencies(tinyxml_vendor)
ament_export_dependencies(TinyXML)
ament_export_dependencies(urdf_parser_plugin)
ament_export_dependencies(urdfdom)
ament_export_dependencies(urdfdom_headers)

pluginlib_export_plugin_description_file(urdf_parser_plugin "urdf_parser_description.xml")

ament_package()
