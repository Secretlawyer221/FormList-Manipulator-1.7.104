cmake_minimum_required(VERSION 3.22)

########################################################################################################################
## Resolve CommonLibSSE (powerof3, AE / 1.7.x capable) and vcpkg.
########################################################################################################################
macro(set_from_environment VARIABLE)
	if (NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif ()
endmacro()

set_from_environment(VCPKG_ROOT)
set_from_environment(CommonLibSSEPath)
add_compile_definitions(SKYRIM_AE SKYRIM_SUPPORT_AE)

if (DEFINED VCPKG_ROOT)
	set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
endif ()

file(TO_CMAKE_PATH "${CommonLibSSEPath}" CLIB_SOURCE_DIR)
if (NOT EXISTS "${CLIB_SOURCE_DIR}/include/RE/Skyrim.h")
	message(FATAL_ERROR "Set the CommonLibSSEPath environment variable to a powerof3/CommonLibSSE (dev) checkout.")
endif ()
set(VCPKG_OVERLAY_PORTS "${CLIB_SOURCE_DIR}/cmake/ports" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ports")

set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")
set(Boost_USE_STATIC_LIBS ON)


########################################################################################################################
## Define project.
########################################################################################################################
project(
    FormListManipulator
    VERSION 1.7.0.0
    DESCRIPTION "Allows dynamic modification of FormLists."
    LANGUAGES CXX
)
set(PROJECT_FRIENDLY_NAME "Formlist Manipulator")
set(PROJECT_AUTHOR "MaskedRPGFan")
set(PROJECT_AUTHOR_EMAIL "maskedrpgfan@gmail.com")

add_compile_definitions(SKSE_SUPPORT_XBYAK)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Plugin.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Plugin.hpp
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

set(sources
		Src/main.cpp
		Src/Plugin.hpp
		Src/Manipulator/EventManager.hpp
		Src/Manipulator/Manipulator.hpp
		Src/Manipulator/RegisterFuncs.hpp
		Src/Utility/ExternalModules.hpp
		Src/Utility/LogInfo.hpp
		Src/Utility/Utility.hpp
		Src/Utility/Types/EntryType.hpp
		Src/Utility/Types/FormListType.hpp
		Src/Utility/Types/InfoType.hpp
		Src/Utility/Types/Keywords.hpp
		Src/Utility/Types/OperatingMode.hpp
		Src/Utility/Types/Types.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
	)

source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${sources})


########################################################################################################################
## Configure target DLL.
########################################################################################################################
add_subdirectory(${CLIB_SOURCE_DIR} CommonLibSSE)
find_package(unordered_dense CONFIG REQUIRED)
find_package(tsl-ordered-map CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)
find_path(MERGEMAPPER_INCLUDE_DIRS "MergeMapperPluginAPI.h")
find_path(CLIB_UTIL_INCLUDE_DIRS "ClibUtil/utils.hpp")

add_library(${PROJECT_NAME} SHARED ${sources} ${MERGEMAPPER_INCLUDE_DIRS}/MergeMapperPluginAPI.cpp)
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE)
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /permissive- /Zc:preprocessor /wd4200)

target_include_directories(${PROJECT_NAME}
        PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${MERGEMAPPER_INCLUDE_DIRS}
		${CLIB_UTIL_INCLUDE_DIRS}
        )

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		CommonLibSSE::CommonLibSSE
		Boost::regex
		unordered_dense::unordered_dense
		tsl::ordered_map
)

target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        Src/Plugin.hpp)


########################################################################################################################
## Copy results.
########################################################################################################################
set(MOD_FOLDER "$ENV{SKSE_SKYRIM_SE_MODS}/${PROJECT_NAME} - DEV")
set(DLL_FOLDER "${MOD_FOLDER}/SKSE/Plugins")

if(DEFINED ENV{SKSE_SKYRIM_SE_MODS})
# Create DLL_FOLDER if it does not exist:
add_custom_command(
	TARGET "${PROJECT_NAME}"
	POST_BUILD
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
	COMMENT "Creating the folder: ${DLL_FOLDER}."
	VERBATIM
)
# Copy the DLL to the configured output directory
add_custom_command(
	TARGET "${PROJECT_NAME}"
	POST_BUILD
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.pdb" "${DLL_FOLDER}" DEPENDS "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.pdb"
	COMMENT "Copying dll: ${PROJECT_NAME}.dll to ${DLL_FOLDER}."
	VERBATIM
)
endif()
