cmake_minimum_required(VERSION 3.16)

project(Euterpe VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core CorePrivate Multimedia)
find_library(ZLIB_LIB z)

qt_standard_project_setup(REQUIRES 6.8)

set_source_files_properties(qml/components/Colors.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

qt_add_executable(Euterpe
    resources.qrc
    main.cpp
)
set_property(TARGET Euterpe APPEND PROPERTY
    QT_ANDROID_PACKAGE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/android
)

qt_add_qml_module(Euterpe
    URI Euterpe
    VERSION 1.0
    QML_FILES
        Main.qml

        qml/pages/SplashScreen.qml
        qml/pages/MainPage.qml
        qml/pages/SongDetailsPage.qml

        qml/components/Colors.qml
        qml/components/ActionButton.qml
        qml/components/HorizontalDivider.qml
        qml/components/TopBar.qml
        qml/components/ImageItem.qml
        qml/components/SongListSection.qml
        qml/components/SongItem.qml
        qml/components/SongQuickView.qml
        qml/components/SongDetailsTopBar.qml
        qml/components/SearchBar.qml
        qml/components/MusicPlayerControls.qml
        qml/components/SeekBar.qml
        qml/components/MetadataEditor.qml
        qml/components/Toast.qml
        qml/components/FloatingLabelTextField.qml

    SOURCES
        include/song.h
        include/song_model.h
        include/audio_player.h
        include/playback_filter_model.h
        include/playback_controller.h
        include/metadata_service.h

        src/song.cpp
        src/song_model.cpp
        src/audio_player.cpp
        src/playback_filter_model.cpp
        src/playback_controller.cpp
        src/metadata_service.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(Euterpe PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Euterpe
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ==============================
# LINKING
# ==============================
# Definisce taglib come libreria statica importata
add_library(taglib STATIC IMPORTED)
target_compile_definitions(Euterpe PRIVATE TAGLIB_STATIC)

set_target_properties(taglib PROPERTIES
    IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/3rdparty/taglib/lib/libtag.a
)

# Include headers
target_include_directories(Euterpe
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/3rdparty/taglib/include
        ${CMAKE_SOURCE_DIR}/3rdparty/taglib/include/toolkit
        ${CMAKE_SOURCE_DIR}/3rdparty/taglib/include/mpeg
)

target_link_libraries(Euterpe
    PRIVATE
        Qt6::Quick
        Qt6::Core
        Qt6::CorePrivate
        Qt6::Multimedia
        taglib
        ${ZLIB_LIB}
)

include(GNUInstallDirs)
install(TARGETS Euterpe
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
