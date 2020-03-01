cmake_minimum_required(VERSION 3.10)

project(stameska)

set(YAML_SOURCES
	3p/libyaml/src/api.c
	3p/libyaml/src/loader.c
	3p/libyaml/src/parser.c
	3p/libyaml/src/reader.c
	3p/libyaml/src/scanner.c
)
add_library(yaml STATIC ${YAML_SOURCES})
target_include_directories(yaml PUBLIC 3p/libyaml/include)
set(YAML_MAJOR 0)
set(YAML_MINOR 2)
set(YAML_PATCH 1)
target_compile_definitions(yaml PRIVATE
	YAML_MAJOR=${YAML_MAJOR}
	YAML_MINOR=${YAML_MINOR}
	YAML_PATCH=${YAML_PATCH}
	YAML_VERSION_MAJOR=${YAML_MAJOR}
	YAML_VERSION_MINOR=${YAML_MINOR}
	YAML_VERSION_PATCH=${YAML_PATCH}
	YAML_VERSION_STRING="${YAML_MAJOR}.${YAML_MINOR}.${YAML_PATCH}"
)
target_compile_definitions(yaml PUBLIC YAML_DECLARE_STATIC)

set(ROCKET_SOURCES
	3p/rocket/lib/device.c
	3p/rocket/lib/track.c
)
add_library(rocket STATIC ${ROCKET_SOURCES})
target_include_directories(rocket PUBLIC 3p)
if(WIN32)
	target_link_libraries(rocket wsock32 ws2_32)
endif()

add_subdirectory(3p/atto)

set(SOURCES
	src/AutomationBasic.cpp
	src/Export.cpp
	src/OpenGL.cpp
	src/PolledFile.cpp
	src/PolledPipelineDesc.cpp
	src/PolledShaderProgram.cpp
	src/PolledShaderSource.cpp
	src/Program.cpp
	src/ProjectSettings.cpp
	src/RenderDesc.cpp
	src/Resources.cpp
	src/ShaderSource.cpp
	src/Rocket.cpp
	src/VideoEngine.cpp
	src/YamlParser.cpp
	src/tool.cpp
	src/format.cpp
	src/video.cpp
)

if (WIN32)
	add_executable(stameska WIN32 ${SOURCES} ${HEADERS})
else()
	add_executable(stameska ${SOURCES} ${HEADERS})
endif()
set_property(TARGET stameska PROPERTY CXX_STANDARD 17)
find_package(Threads REQUIRED)
if (LINUX)
	find_package(ALSA REQUIRED)
	target_link_libraries(stameska ALSA::ALSA stdc++fs)
elseif(WIN32)
	target_link_libraries(stameska winmm)
endif()
target_link_libraries(stameska
	rocket
	atto
	Threads::Threads
	yaml)
