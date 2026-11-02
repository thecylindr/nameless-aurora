cmake_minimum_required(VERSION 3.21)
project(AuroraPlayer VERSION 3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)
set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")

find_package(Qt6 6.12 REQUIRED COMPONENTS Core Gui Widgets Qml Quick Sql Multimedia QuickControls2 QuickShapes Network)
qt_standard_project_setup(REQUIRES 6.12)

qt_add_executable(AuroraPlayer
    src/app/main.cpp
    src/core/librarymodel.cpp
    src/core/librarymodel.h
    src/core/playbackcontroller.cpp
    src/core/playbackcontroller.h
    src/core/importmanager.cpp
    src/core/importmanager.h
    src/core/downloadmanager.cpp
    src/core/downloadmanager.h
    src/core/settingsstore.cpp
    src/core/settingsstore.h
    src/core/downloads/dependencymanager.cpp
    src/core/downloads/dependencymanager.h
    src/core/downloads/importpreviewer.cpp
    src/core/downloads/importpreviewer.h
    src/core/auth/browserauthmanager.cpp
    src/core/auth/browserauthmanager.h
    src/core/storage/storagemanager.cpp
    src/core/storage/storagemanager.h
)

qt_add_qml_module(AuroraPlayer
    URI AuroraPlayer
    VERSION 3.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/common/AuroraButton.qml
        qml/common/AuroraIcon.qml
        qml/common/AuroraCard.qml
        qml/common/AuroraField.qml
        qml/common/AuroraSlider.qml
        qml/common/AuroraSwitch.qml
        qml/common/AuroraSegmented.qml
        qml/common/AuroraProgress.qml
        qml/common/AuroraDialog.qml
        qml/common/AuroraSection.qml
        qml/common/AuroraScrollBar.qml
        qml/common/AuroraToast.qml
        qml/components/NavigationPane.qml
        qml/components/TrackList.qml
        qml/components/TrackDelegate.qml
        qml/components/PlayerBar.qml
        qml/components/NowPlayingPanel.qml
        qml/components/StorageSummary.qml
        qml/components/DependencyCard.qml
        qml/components/RecommendationCard.qml
        qml/pages/HomePage.qml
        qml/pages/LibraryPage.qml
        qml/pages/ImportPage.qml
        qml/pages/SettingsPage.qml
)

target_include_directories(AuroraPlayer PRIVATE src/core)
target_link_libraries(AuroraPlayer PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::Sql
    Qt6::Multimedia Qt6::QuickControls2 Qt6::QuickShapes Qt6::Network
)
set_target_properties(AuroraPlayer PROPERTIES WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)

if(ANDROID)
    math(EXPR AURORA_VERSION_CODE "${PROJECT_VERSION_MAJOR} * 10000 + ${PROJECT_VERSION_MINOR} * 100 + ${PROJECT_VERSION_PATCH}")
    set_target_properties(AuroraPlayer PROPERTIES
        QT_ANDROID_VERSION_NAME ${PROJECT_VERSION}
        QT_ANDROID_VERSION_CODE ${AURORA_VERSION_CODE}
        QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 35
        QT_ANDROID_PACKAGE_NAME org.aurora.player
        QT_ANDROID_APP_NAME "Aurora Player"
    )
elseif(UNIX AND NOT APPLE)
    set_target_properties(AuroraPlayer PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
endif()

install(TARGETS AuroraPlayer BUNDLE DESTINATION . RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
if(UNIX AND NOT APPLE AND NOT ANDROID)
    qt_generate_deploy_qml_app_script(
        TARGET AuroraPlayer
        OUTPUT_SCRIPT aurora_deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
        NO_TRANSLATIONS
    )
    install(CODE "if(AURORA_BUNDLE_QT)\ninclude(\"${aurora_deploy_script}\")\nendif()")
endif()
