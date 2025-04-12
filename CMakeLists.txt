CMAKE_MINIMUM_REQUIRED(VERSION 3.30)

PROJECT(zhenerals)

SET(CMAKE_CXX_STANDARD 20)

FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(OpenAL REQUIRED)
FIND_PACKAGE(SDL2 REQUIRED)
FIND_PACKAGE(Vulkan REQUIRED)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(LIBAV REQUIRED IMPORTED_TARGET
  libavcodec
  libavformat
  libavutil
  libswresample
)

INCLUDE(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        123913715afeb8a437e6388b4473fcc4753e1c9a
) # 11.1.4
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  glm
  GIT_REPOSITORY  https://github.com/g-truc/glm.git
  GIT_TAG   bf71a834948186f4097caa076cd2663c69a10e1e
) # 1.0.1

FetchContent_MakeAvailable(glm)

INCLUDE_DIRECTORIES(
  ${fmt_SOURCE_DIR}/include
  ${FREETYPE_INCLUDE_DIRS}
  ${LIBAV_INCLUDE_DIRS}
  ${OPENAL_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIRS}
)

OPTION(USE_TESTS "Enable building and running of tests" OFF)
OPTION(USE_TRACY "Build with Tracy support" OFF)

IF (USE_TRACY)
  INCLUDE(FetchContent)

  IF (NOT STATIC_LINK)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
  ENDIF()

  FETCHCONTENT_DECLARE(
    tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG v0.11.1
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
  )
  FETCHCONTENT_MAKEAVAILABLE(tracy)

  ADD_DEFINITIONS("-DUSE_TRACY")
  INCLUDE_DIRECTORIES(${tracy_SOURCE_DIR}/public)
ENDIF()

IF (USE_TESTS)
  FIND_PACKAGE(GTest REQUIRED)
  INCLUDE(CTest)
  ENABLE_TESTING()
  LIST(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
ENDIF()

FUNCTION(ADD_UNIT_TEST unit_name)
  IF (BUILD_TESTING)
    ADD_EXECUTABLE(Test_${unit_name} ${ARGN})

    TARGET_LINK_LIBRARIES(Test_${unit_name}
      GTest::gtest
      GTest::gtest_main
      fmt::fmt
    )

    TARGET_COMPILE_DEFINITIONS(Test_${unit_name} PRIVATE NO_TRACY=1)

    ADD_TEST(NAME Test_${unit_name}
      COMMAND $<TARGET_FILE:Test_${unit_name}>
    )
  ENDIF()
ENDFUNCTION()

# main executable
ADD_EXECUTABLE(zhen
  game/audio/Backend.cpp
  game/audio/Playback.cpp
  game/audio/SoundData.cpp
  game/Color.cpp
  game/common.cpp
  game/EventDispatcher.cpp
  game/Game.cpp
  game/Game.cpp
  game/formats/AudioFile.cpp
  game/formats/BIGFile.cpp
  game/formats/CSFFile.cpp
  game/formats/TGAFile.cpp
  game/formats/WNDFile.cpp
  game/gfx/HostTexture.cpp
  game/gfx/TextureCache.cpp
  game/gfx/TextureLookup.cpp
  game/gfx/font/Atlas.cpp
  game/gfx/font/FontManager.cpp
  game/gfx/font/TextureGenerator.cpp
  game/GUI/Button.cpp
  game/GUI/Component.cpp
  game/GUI/ComponentFactory.cpp
  game/GUI/Label.cpp
  game/GUI/Overlay.cpp
  game/GUI/Window.cpp
  game/GUI/drawing/RenderListFactory.cpp
  game/GUI/hosts/MainMenu.cpp
  game/GUI/wnd/DrawData.cpp
  game/GUI/wnd/Font.cpp
  game/GUI/wnd/Layout.cpp
  game/GUI/wnd/ScreenRect.cpp
  game/GUI/wnd/TextColor.cpp
  game/GUI/wnd/Window.cpp
  game/GUI/wnd/WindowAndLayout.cpp
  game/inis/INIFile.cpp
  game/inis/MappedImageINI.cpp
  game/inis/SoundEffectsINI.cpp
  game/Main.cpp
  game/MurmurHash.cpp
  game/Logger.cpp
  game/Logging.cpp
  game/ResourceLoader.cpp
  game/StringLoader.cpp
  game/Window.cpp
  game/WindowFactory.cpp

  game/vugl/vma_loader.cpp
  game/vugl/vugl_command_buffer.cpp
  game/vugl/vugl_compute_pipeline.cpp
  game/vugl/vugl_context.cpp
  game/vugl/vugl_descriptor_set.cpp
  game/vugl/vugl_device_sampler.cpp
  game/vugl/vugl_element_buffer.cpp
  game/vugl/vugl_frame.cpp
  game/vugl/vugl_pipeline.cpp
  game/vugl/vugl_pipeline_setup.cpp
  game/vugl/vugl_render_pass.cpp
  game/vugl/vugl_render_pass_setup.cpp
  game/vugl/vugl_resource_allocator.cpp
  game/vugl/vugl_upload_sampler.cpp
  game/vugl/vugl_uniform_buffer.cpp
)

IF(WIN32)
  TARGET_SOURCES(zhen PUBLIC
    resources/game.rc
  )
ENDIF()

TARGET_LINK_LIBRARIES(zhen
  fmt::fmt
  ${FREETYPE_LIBRARIES}
  glm::glm
  PkgConfig::LIBAV
  ${OPENAL_LIBRARY}
  ${SDL2_LIBRARIES}
  ${Vulkan_LIBRARY}
)

MESSAGE(WARNING "libav ${LIBAV_INCLUDE_DIRS}, ${LIBAV_LIBRARIES}")

IF (USE_TRACY)
  TARGET_LINK_LIBRARIES(zhen Tracy::TracyClient)
ENDIF()

# big tool
ADD_EXECUTABLE(big
  game/formats/BIGFile.cpp
  tools/BIG.cpp
)

ADD_CUSTOM_TARGET(
  always_run ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/__header.h
)

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

ADD_UNIT_TEST(AudioFile
  game/audio/Backend.cpp

  game/audio/SoundData.cpp
  game/Logging.cpp
  game/formats/AudioFile.cpp
  game/tests/Test_AudioFile.cpp
)

TARGET_LINK_LIBRARIES(Test_AudioFile
  ${OPENAL_LIBRARY}

  PkgConfig::LIBAV
)

ADD_UNIT_TEST(BIGFile
  game/formats/BIGFile.cpp
  game/tests/Test_BIGFile.cpp
)

ADD_UNIT_TEST(CSFFile
  game/formats/CSFFile.cpp
  game/tests/Test_CSFFile.cpp
)

ADD_UNIT_TEST(Font
  game/common.cpp
  game/Logging.cpp
  game/gfx/HostTexture.cpp
  game/gfx/font/Atlas.cpp
  game/gfx/font/FontManager.cpp
  game/gfx/font/TextureGenerator.cpp
  game/tests/Test_Font.cpp
)

TARGET_LINK_LIBRARIES(Test_Font
  ${FREETYPE_LIBRARIES}
)

ADD_UNIT_TEST(MappedImageINI
  game/inis/INIFile.cpp
  game/inis/MappedImageINI.cpp
  game/tests/Test_MappedImageINI.cpp
)

ADD_UNIT_TEST(ResourceLoader
  game/formats/BIGFile.cpp
  game/Logging.cpp
  game/ResourceLoader.cpp
  game/tests/Test_ResourceLoader.cpp
)

ADD_UNIT_TEST(SoundEffectsINI
  game/inis/INIFile.cpp
  game/inis/SoundEffectsINI.cpp
  game/tests/Test_SoundEffectsINI.cpp
)

ADD_UNIT_TEST(TGAFile
  game/gfx/HostTexture.cpp
  game/Logging.cpp
  game/formats/TGAFile.cpp
  game/tests/Test_TGAFile.cpp
)

ADD_UNIT_TEST(WNDFile
  game/formats/WNDFile.cpp
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

IF (BUILD_TESTING)
  ADD_CUSTOM_COMMAND(TARGET
      Test_AudioFile
      Test_BIGFile
      Test_CSFFile
      Test_Font
      Test_MappedImageINI
      Test_ResourceLoader
      Test_SoundEffectsINI
      Test_TGAFile
      Test_WNDFile
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:zhen>/tests/resources
      && ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/game/tests/resources
      $<TARGET_FILE_DIR:zhen>/tests/resources)
ENDIF()
