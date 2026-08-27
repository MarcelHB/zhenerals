CMAKE_MINIMUM_REQUIRED(VERSION 3.30)
SET(CMAKE_POLICY_VERSION_MINIMUM 3.10)

PROJECT(zhenerals)

SET(CMAKE_CXX_STANDARD 20)

FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(OpenAL REQUIRED)
FIND_PACKAGE(SDL3 REQUIRED)
FIND_PACKAGE(Vulkan REQUIRED)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(LIBAV REQUIRED IMPORTED_TARGET
  libavcodec
  libavformat
  libavutil
  libswresample
)

# MSYS2 ld seems to be, or was at some version, broken
# as it didn't handle excessive template use well
IF (WIN32)
  FIND_PROGRAM(HAS_LLD lld)
  IF (NOT HAS_LLD)
    MESSAGE(FATAL_ERROR "You need to have lld installed.")
  ENDIF()
ENDIF()

INCLUDE(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        1be298e1bd68957e4cd352e1f676f00e07dcfb57
) # 12.2.0
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  glm
  GIT_REPOSITORY  https://github.com/g-truc/glm.git
  GIT_TAG         8d1fd52e5ab5590e2c81768ace50c72bae28f2ed
) # 1.0.3

FetchContent_MakeAvailable(glm)

INCLUDE_DIRECTORIES(
  ${fmt_SOURCE_DIR}/include
  ${FREETYPE_INCLUDE_DIRS}
  ${glm_SOURCE_DIR}
  ${LIBAV_INCLUDE_DIRS}
  ${OPENAL_INCLUDE_DIR}
  ${SDL3_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/game
)

OPTION(USE_TESTS "Enable building and running of tests" OFF)
OPTION(USE_GAME_TESTS "Enable building and running of test against game data" OFF)
OPTION(USE_TRACY "Build with Tracy support" OFF)
OPTION(USE_USE_STATIC_LINKINGING "Link own sources statically" OFF)

IF (USE_TRACY)
  INCLUDE(FetchContent)

  IF (NOT USE_STATIC_LINKING)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
  ENDIF()

  OPTION(TRACY_ON_DEMAND "" ON)
  OPTION(TRACY_ENABLE "" ON)

  FETCHCONTENT_DECLARE(
    tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG        30997d5ca6bb632cc10807a1da8a6d3de0aeeb3c
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
  ) # 0.14.1
  FETCHCONTENT_MAKEAVAILABLE(tracy)

  ADD_DEFINITIONS("-DUSE_TRACY -DTRACY_ENABLE -DTRACY_ON_DEMAND")
  INCLUDE_DIRECTORIES(${tracy_SOURCE_DIR}/public)
ENDIF()

IF (USE_TESTS OR USE_GAME_TESTS)
  FIND_PACKAGE(GTest REQUIRED)
  INCLUDE(CTest)
  ENABLE_TESTING()
  LIST(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
ENDIF()

FUNCTION(ADD_UNIT_TEST unit_name)
  IF (BUILD_TESTING AND USE_TESTS)
    ADD_EXECUTABLE(Test_${unit_name}
      ${ARGN}
    )

    TARGET_LINK_LIBRARIES(Test_${unit_name}
      inis
      formats
      GTest::gtest
      GTest::gtest_main
      fmt::fmt
    )

    TARGET_COMPILE_DEFINITIONS(Test_${unit_name} PRIVATE NO_TRACY=1 IN_TEST=1)

    ADD_TEST(NAME Test_${unit_name}
      COMMAND $<TARGET_FILE:Test_${unit_name}>
    )
  ENDIF()
ENDFUNCTION()

FUNCTION(ADD_GAME_TEST game_test_name)
  IF (BUILD_TESTING AND USE_GAME_TESTS)
    ADD_EXECUTABLE(GameTest_${game_test_name}
      ${ARGN}
      game/MemoryViewStream.cpp
      game/ResourceLoader.cpp
    )

    TARGET_LINK_LIBRARIES(GameTest_${game_test_name}
      inis
      formats
      GTest::gtest
      GTest::gtest_main
      fmt::fmt
    )

    TARGET_COMPILE_DEFINITIONS(GameTest_${game_test_name} PRIVATE NO_TRACY=1 IN_TEST=1 IS_CORE=1)

    ADD_TEST(NAME GameTest_${game_test_name}
      COMMAND $<TARGET_FILE:GameTest_${game_test_name}>
    )
  ENDIF()
ENDFUNCTION()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wfatal-errors -Wextra -Wpedantic -Wno-gnu-zero-variadic-macro-arguments -Wno-nullability-extension -Wno-missing-field-initializers -Wno-unused-parameter")

ADD_DEFINITIONS("-DGLM_ENABLE_EXPERIMENTAL")
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-completeness")
ENDIF()

IF (WIN32)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=lld")
ENDIF()

IF (USE_STATIC_LINKING)
  ADD_DEFINITIONS("-DUSE_STATIC_LINKING")
ENDIF ()

IF (USE_TRACY)
  IF(USE_STATIC_LINKING)
    ADD_LIBRARY(Tracy STATIC ${tracy_SOURCE_DIR}/public/TracyClient.cpp)
  ELSE()
    ADD_LIBRARY(Tracy SHARED ${tracy_SOURCE_DIR}/public/TracyClient.cpp)
  ENDIF()

  TARGET_COMPILE_DEFINITIONS(Tracy PRIVATE TRACY_ENABLE=1)
  TARGET_COMPILE_OPTIONS(Tracy PRIVATE "-Wno-sign-compare")
  TARGET_COMPILE_OPTIONS(Tracy PRIVATE "-Wno-format")
  TARGET_COMPILE_OPTIONS(Tracy PRIVATE "-Wno-unused-value")
  TARGET_COMPILE_OPTIONS(Tracy PRIVATE "-Wno-c11-extensions")
  TARGET_COMPILE_OPTIONS(Tracy PRIVATE "-Wno-error=cast-function-type")

  IF (WIN32)
    TARGET_LINK_LIBRARIES(Tracy ws2_32 dbghelp secur32)
  ENDIF ()
ENDIF ()

SET(vugl_sources
  game/vugl/vma_loader.cpp
  game/vugl/vugl_combined_sampler.cpp
  game/vugl/vugl_command_buffer.cpp
  game/vugl/vugl_command_pool.cpp
  game/vugl/vugl_compute_pipeline.cpp
  game/vugl/vugl_context.cpp
  game/vugl/vugl_descriptor_set.cpp
  game/vugl/vugl_element_buffer.cpp
  game/vugl/vugl_frame.cpp
  game/vugl/vugl_pipeline.cpp
  game/vugl/vugl_pipeline_setup.cpp
  game/vugl/vugl_render_pass.cpp
  game/vugl/vugl_render_pass_setup.cpp
  game/vugl/vugl_resource_allocator.cpp
  game/vugl/vugl_sampler.cpp
  game/vugl/vugl_texture.cpp
  game/vugl/vugl_uniform_buffer.cpp
)

IF (USE_STATIC_LINKING)
  ADD_LIBRARY(vugl STATIC ${vugl_sources})
ELSE ()
  ADD_LIBRARY(vugl SHARED ${vugl_sources})
ENDIF ()

TARGET_LINK_LIBRARIES(vugl
  ${Vulkan_LIBRARY}
)

SET(logging_sources
  game/Logger.cpp
  game/Logging.cpp
)
ADD_LIBRARY(logging STATIC ${logging_sources})

SET(format_sources
  game/gfx/HostTexture.cpp
  game/formats/BIGFile.cpp
  game/formats/CSFFile.cpp
  game/formats/DDSFile.cpp
  game/formats/MAPFile.cpp
  game/formats/Dict.cpp
  game/formats/TGAFile.cpp
  game/formats/WNDFile.cpp
  game/formats/W3DFile.cpp
  game/GUI/Button.cpp
  game/GUI/Component.cpp
  game/GUI/Label.cpp
  game/GUI/Window.cpp
  game/GUI/wnd/DrawData.cpp
  game/GUI/wnd/Font.cpp
  game/GUI/wnd/Layout.cpp
  game/GUI/wnd/ScreenRect.cpp
  game/GUI/wnd/TextColor.cpp
  game/GUI/wnd/Window.cpp
  game/GUI/wnd/WindowAndLayout.cpp
  game/InflatingStream.cpp
  game/objects/Object.cpp
  game/Script.cpp
)

IF (USE_STATIC_LINKING)
  ADD_LIBRARY(formats STATIC ${format_sources})
ELSE ()
  ADD_LIBRARY(formats SHARED ${format_sources})
ENDIF ()

TARGET_LINK_LIBRARIES(formats PRIVATE logging fmt::fmt)

SET(ini_sources
  game/inis/INIFile.cpp
  game/inis/MappedImageINI.cpp
  game/inis/ObjectsINI.cpp
  game/inis/RoadsBridgesINI.cpp
  game/inis/SoundEffectsINI.cpp
  game/inis/TerrainINI.cpp
  game/inis/WaterINI.cpp
  game/MurmurHash.cpp
)

IF (USE_STATIC_LINKING)
  ADD_LIBRARY(inis STATIC ${ini_sources})
ELSE ()
  ADD_LIBRARY(inis SHARED ${ini_sources})
ENDIF ()

TARGET_LINK_LIBRARIES(inis
  formats
  fmt::fmt
)

# main executable
ADD_EXECUTABLE(zhen
  game/audio/Backend.cpp
  game/audio/Playback.cpp
  game/audio/SoundData.cpp
  game/Battlefield.cpp
  game/BattlefieldFactory.cpp
  game/Color.cpp
  game/common.cpp
  game/EventDispatcher.cpp
  game/Game.cpp
  game/Geometry.cpp
  game/formats/AudioFile.cpp
  game/gfx/Camera.cpp
  game/gfx/FrameDisposable.cpp
  game/gfx/Frustum.cpp
  game/gfx/Model.cpp
  game/gfx/ModelCache.cpp
  game/gfx/TextureCache.cpp
  game/gfx/TextureLoader.cpp
  game/gfx/TextureLookup.cpp
  game/gfx/font/Atlas.cpp
  game/gfx/font/FontManager.cpp
  game/gfx/font/TextureGenerator.cpp
  game/GUI/ComponentFactory.cpp
  game/GUI/Overlay.cpp
  game/GUI/drawing/RenderListFactory.cpp
  game/GUI/hosts/EventDefaults.cpp
  game/GUI/hosts/MainMenu.cpp
  game/Main.cpp
  game/Map.cpp
  game/MemProfiling.cpp
  game/objects/Instance.cpp
  game/objects/InstanceFactory.cpp
  game/ObjectLoader.cpp
  game/MemoryViewStream.cpp
  game/ResourceLoader.cpp
  game/rendering/BattlefieldRenderer.cpp
  game/rendering/InstanceRenderer.cpp
  game/rendering/ModelRenderer.cpp
  game/rendering/PatchRenderer.cpp
  game/StringLoader.cpp
  game/ThreadPool.cpp
  game/Window.cpp
  game/WindowFactory.cpp
)

IF(WIN32)
  TARGET_SOURCES(zhen PUBLIC
    resources/game.rc
  )
ENDIF()

TARGET_LINK_LIBRARIES(zhen
  logging
  inis
  formats
  vugl
  fmt::fmt
  ${FREETYPE_LIBRARIES}
  glm::glm
  PkgConfig::LIBAV
  ${OPENAL_LIBRARY}
  ${SDL3_LIBRARIES}
)

# big tool
ADD_EXECUTABLE(big
  tools/BIG.cpp
)

TARGET_LINK_LIBRARIES(big formats)

# decompress tool
ADD_EXECUTABLE(decompress
  game/InflatingStream.cpp
  tools/decompress.cpp
)

# map dump
ADD_EXECUTABLE(mapdump
  game/common.cpp
  game/MemoryViewStream.cpp
  game/ResourceLoader.cpp
  tools/mapdump.cpp
)

TARGET_LINK_LIBRARIES(mapdump
  logging
  vugl
  inis
  formats
  fmt::fmt
)

# w3d dump
ADD_EXECUTABLE(w3ddump
  game/common.cpp
  game/MemoryViewStream.cpp
  game/ResourceLoader.cpp
  tools/w3ddump.cpp
)

TARGET_LINK_LIBRARIES(w3ddump
  logging
  inis
  formats
  fmt::fmt
)

SET(common_viewer_sources
  game/common.cpp
  game/gfx/Camera.cpp
  game/gfx/font/Atlas.cpp
  game/gfx/font/FontManager.cpp
  game/gfx/font/TextureGenerator.cpp
  game/gfx/FrameDisposable.cpp
  game/gfx/Model.cpp
  game/gfx/ModelCache.cpp
  game/gfx/TextureCache.cpp
  game/gfx/TextureLoader.cpp
  game/gfx/TextureLookup.cpp
  game/MemoryViewStream.cpp
  game/MurmurHash.cpp
  game/rendering/LineRenderer.cpp
  game/rendering/ModelRenderer.cpp
  game/ResourceLoader.cpp
  game/Window.cpp
  tools/ViewHelpers.cpp
)

# obj viewer
ADD_EXECUTABLE(objview
  game/Color.cpp
  game/objects/Instance.cpp
  game/objects/InstanceFactory.cpp
  game/ObjectLoader.cpp
  game/GUI/drawing/RenderListFactory.cpp
  game/rendering/InstanceRenderer.cpp
  game/ThreadPool.cpp
  ${common_viewer_sources}
  tools/objview.cpp
)

TARGET_LINK_LIBRARIES(objview
  logging
  inis
  formats
  vugl
  fmt::fmt
  ${FREETYPE_LIBRARIES}
  glm::glm
  ${SDL3_LIBRARIES}
)

# w3d viewer
ADD_EXECUTABLE(w3dview
  ${common_viewer_sources}
  tools/w3dview.cpp
)

TARGET_LINK_LIBRARIES(w3dview
  logging
  inis
  formats
  vugl
  fmt::fmt
  ${FREETYPE_LIBRARIES}
  glm::glm
  ${SDL3_LIBRARIES}
)

IF (USE_TRACY)
  TARGET_LINK_LIBRARIES(formats PRIVATE Tracy)
  TARGET_LINK_LIBRARIES(inis Tracy)
  TARGET_LINK_LIBRARIES(objview Tracy)
  TARGET_LINK_LIBRARIES(w3dview Tracy)
  TARGET_LINK_LIBRARIES(zhen Tracy)
ENDIF()

ADD_CUSTOM_TARGET(always_run
  ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/__header.h
)

IF (BUILD_TESTING)
  ADD_CUSTOM_TARGET(always_run_tests
    ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/__header_test.h
  )
ENDIF()

ADD_CUSTOM_COMMAND(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/__header.h
    ${CMAKE_CURRENT_BINARY_DIR}/header.h
  COMMAND bash ${CMAKE_SOURCE_DIR}/resources/shaders/compile.sh

  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:zhen>/shaders
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/resources/shaders/*.spv
    $<TARGET_FILE_DIR:zhen>/shaders

  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:zhen>/fonts
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/resources/fonts/*.ttf
    $<TARGET_FILE_DIR:zhen>/fonts

  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/resources/shaders
)

TARGET_COMPILE_DEFINITIONS(big PRIVATE NO_TRACY=1)
TARGET_COMPILE_DEFINITIONS(mapdump PRIVATE NO_TRACY=1)
TARGET_COMPILE_DEFINITIONS(w3ddump PRIVATE NO_TRACY=1)

ADD_UNIT_TEST(AudioFile
  game/audio/Backend.cpp

  game/audio/SoundData.cpp
  game/formats/AudioFile.cpp
  game/tests/Test_AudioFile.cpp
)

IF (BUILD_TESTING AND USE_TESTS)
  TARGET_LINK_LIBRARIES(Test_AudioFile
    ${OPENAL_LIBRARY}

    PkgConfig::LIBAV
  )
ENDIF()

ADD_UNIT_TEST(BIGFile
  game/tests/Test_BIGFile.cpp
)

ADD_UNIT_TEST(CSFFile
  game/tests/Test_CSFFile.cpp
)

ADD_UNIT_TEST(DDSFile
  game/tests/Test_DDSFile.cpp
)

ADD_UNIT_TEST(Dict
  game/tests/Test_Dict.cpp
)

ADD_UNIT_TEST(Font
  game/common.cpp
  game/gfx/font/Atlas.cpp
  game/gfx/font/FontManager.cpp
  game/gfx/font/TextureGenerator.cpp
  game/tests/Test_Font.cpp
)

IF (BUILD_TESTING AND USE_TESTS)
  TARGET_LINK_LIBRARIES(Test_Font
    ${FREETYPE_LIBRARIES}
  )
ENDIF()

ADD_UNIT_TEST(Frustum
  game/gfx/Camera.cpp
  game/gfx/Frustum.cpp
  game/tests/Test_Frustum.cpp
)

ADD_UNIT_TEST(Geometry
  game/ThreadPool.cpp
  game/tests/Test_Geometry.cpp
)

ADD_UNIT_TEST(MappedImageINI
  game/tests/Test_MappedImageINI.cpp
)

ADD_UNIT_TEST(MurmurHash
  game/MurmurHash.cpp
  game/tests/Test_MurmurHash.cpp
)

ADD_UNIT_TEST(MemoryViewStream
  game/MemoryViewStream.cpp
  game/tests/Test_MemoryViewStream.cpp
)

ADD_UNIT_TEST(ObjectsINI
  game/MurmurHash.cpp
  game/tests/Test_ObjectsINI.cpp
)

ADD_UNIT_TEST(ResourceLoader
  game/MemoryViewStream.cpp
  game/ResourceLoader.cpp
  game/tests/Test_ResourceLoader.cpp
)

ADD_UNIT_TEST(RoadsBridgesINI
  game/MurmurHash.cpp
  game/tests/Test_RoadsBridgesINI.cpp
)

ADD_UNIT_TEST(SoundEffectsINI
  game/MurmurHash.cpp
  game/tests/Test_SoundEffectsINI.cpp
)

ADD_UNIT_TEST(TerrainINI
  game/MurmurHash.cpp
  game/tests/Test_TerrainINI.cpp
)

ADD_UNIT_TEST(TGAFile
  game/tests/Test_TGAFile.cpp
)

ADD_UNIT_TEST(WaterINI
  game/tests/Test_WaterINI.cpp
)

ADD_UNIT_TEST(WNDFile
  game/GUI/wnd/DrawData.cpp
  game/GUI/wnd/Font.cpp
  game/GUI/wnd/Font.cpp
  game/GUI/wnd/Layout.cpp
  game/GUI/wnd/ScreenRect.cpp
  game/GUI/wnd/TextColor.cpp
  game/GUI/wnd/Window.cpp
  game/GUI/wnd/WindowAndLayout.cpp
  game/tests/Test_WNDFile.cpp
)

ADD_GAME_TEST(MAPFile
  game/Battlefield.cpp
  game/BattlefieldFactory.cpp
  game/Geometry.cpp
  game/gfx/Camera.cpp
  game/gfx/Model.cpp
  game/gfx/ModelCache.cpp
  game/Map.cpp
  game/MemoryViewStream.cpp
  game/MurmurHash.cpp
  game/objects/Instance.cpp
  game/objects/InstanceFactory.cpp
  game/ObjectLoader.cpp
  game/inis/TerrainINI.cpp
  game/Script.cpp
  game/ThreadPool.cpp
  game/tests/GameTest_Map.cpp
)

ADD_GAME_TEST(ObjectsINI
  game/MemoryViewStream.cpp
  game/MurmurHash.cpp
  game/tests/GameTest_ObjectsINI.cpp
)

ADD_GAME_TEST(Textures
  game/gfx/TextureLoader.cpp
  game/tests/GameTest_Textures.cpp
)

ADD_GAME_TEST(W3DFile
  game/MemoryViewStream.cpp
  game/gfx/Model.cpp
  game/tests/GameTest_W3D.cpp
)

IF (BUILD_TESTING)
  ADD_CUSTOM_COMMAND(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/__header_test.h
      ${CMAKE_CURRENT_BINARY_DIR}/header_test.h
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:zhen>/tests/resources
      && ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/game/tests/resources
      $<TARGET_FILE_DIR:zhen>/tests/resources)
ENDIF()
