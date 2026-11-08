cmake_minimum_required(VERSION 3.21)
project(AuroraPlayer VERSION 4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)
set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")

find_package(Qt6 6.12 REQUIRED COMPONENTS Core Gui Widgets Qml Quick Sql Multimedia QuickControls2 QuickShapes Network Concurrent WebEngineQuick ShaderTools)
qt_standard_project_setup(REQUIRES 6.12)

qt_add_executable(AuroraPlayer
    src/app/main.cpp
    src/core/integration/namelessbridge.cpp
    src/core/integration/namelessbridge.h
    src/core/data/libraryrepository.cpp
    src/core/data/libraryrepository.h
    src/core/libraryexchange.cpp
    src/core/libraryexchange.h
    src/core/librarymodel.cpp
    src/core/librarymodel.h
    src/core/homefeed.cpp src/core/homefeed.h
    src/core/artwork.cpp
    src/core/playbackcontroller.cpp
    src/core/playbackcontroller.h
    src/core/web/streamproxy.cpp src/core/web/streamproxy.h
    src/core/audioreactivity.cpp src/core/audioreactivity.h
    src/core/audioequalizer.cpp src/core/audioequalizer.h
    src/core/importmanager.cpp
    src/core/importmanager.h
    src/core/downloads/nativedownload.cpp
    src/core/downloadmanager.cpp
    src/core/downloadmanager.h
    src/core/downloads/audiosources.cpp
    src/core/downloads/audiosources.h
    src/core/downloads/musicsearch.cpp src/core/downloads/musicsearch.h
    src/core/downloads/opencatalog.cpp src/core/downloads/opencatalog.h
    src/core/downloads/zaycevcatalog.cpp src/core/downloads/zaycevcatalog.h
    src/core/settingsstore.cpp
    src/core/settingsstore.h
    src/core/downloads/dependencymanager.cpp
    src/core/downloads/dependencymanager.h
    src/core/downloads/importpreviewer.cpp
    src/core/downloads/importpreviewer.h
    src/core/auth/browserauthmanager.cpp
    src/core/auth/browserauthmanager.h
    src/core/auth/oauthmanager.cpp
    src/core/auth/oauthmanager.h
    src/core/auth/webloginmanager_native.cpp
    src/core/web/mediasniffer.cpp
    src/core/web/mediasniffer.h
    src/core/web/servicepayload.cpp
    src/core/web/servicepayload.h
    src/core/auth/browsersession.cpp
    src/core/auth/browsersession.h
    src/core/auth/browsercookieimporter.cpp
    src/core/auth/browsercookieimporter.h
    src/core/auth/cdpclient.cpp
    src/core/auth/cdpclient.h
    src/core/auth/webloginmanager.cpp
    src/core/auth/webloginmanager.h
    src/core/vkprovider.cpp
    src/core/vkprovider.h
    src/core/storage/storagemanager.cpp
    src/core/storage/storagemanager.h
)

qt_add_qml_module(AuroraPlayer
    URI AuroraPlayer
    VERSION 3.0
    QML_FILES
        qml/web/NativeBrowser.qml
        qml/Main.qml
        qml/Theme.qml
        qml/common/AuroraButton.qml
        qml/common/AuroraIcon.qml
        qml/common/AuroraCard.qml
        qml/common/AuroraCombo.qml
        qml/common/AuroraField.qml
        qml/common/AuroraSlider.qml
        qml/common/AuroraSwitch.qml
        qml/common/AuroraSegmented.qml
        qml/common/AuroraProgress.qml
        qml/common/AuroraDialog.qml
        qml/common/AuroraMenu.qml
        qml/common/AuroraMenuItem.qml
        qml/common/AuroraSection.qml
        qml/common/AuroraScrollBar.qml
        qml/common/AuroraToast.qml
        qml/components/NavigationPane.qml
        qml/components/TrackList.qml
        qml/components/TrackDelegate.qml
        qml/components/ListeningView.qml
        qml/components/CornerCover.qml
        qml/common/ProgressRing.qml
        qml/components/BonsaiScene.qml
        qml/components/BonsaiRoot.qml
        qml/components/AtmosphereOrb.qml
        qml/components/EffectSurface.qml
        qml/components/AppearancePanel.qml
        qml/components/LibrarySettings.qml
        qml/components/EqualizerPanel.qml
        qml/components/EqualizerDialog.qml
        qml/components/AmbientBackdrop.qml
        qml/components/AlbumArt.qml
        qml/components/PlayerBar.qml
        qml/components/SourceDialog.qml
        qml/components/DownloadDock.qml
        qml/components/TrackEditor.qml
        qml/components/AlbumEditor.qml
        qml/components/NowPlayingPanel.qml
        qml/components/StorageSummary.qml
        qml/components/DependencyCard.qml
        qml/components/RecommendationCard.qml
        qml/components/HomeMixCard.qml
        qml/components/HomeProviderShelf.qml
        qml/components/ProviderPlaylistDialog.qml
        qml/components/HomeMusicPreferences.qml
        qml/pages/HomePage.qml
        qml/pages/LibraryPage.qml
        qml/pages/ImportPage.qml
        qml/pages/SettingsPage.qml
        qml/pages/SearchPage.qml
        qml/components/AtmosphereObjects.qml
)

target_include_directories(AuroraPlayer PRIVATE src/core)
target_link_libraries(AuroraPlayer PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::Sql
    Qt6::Multimedia Qt6::QuickControls2 Qt6::QuickShapes Qt6::Network Qt6::Concurrent Qt6::WebEngineQuick
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

if(UNIX AND NOT APPLE AND NOT ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_sources(AuroraPlayer PRIVATE src/core/integration/mpris.cpp src/core/integration/mpris.h)
    target_link_libraries(AuroraPlayer PRIVATE Qt6::DBus)
    target_compile_definitions(AuroraPlayer PRIVATE AURORA_MPRIS)
endif()

include(CTest)
if(BUILD_TESTING AND NOT ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    qt_add_executable(aurora_tests tests/tst_library.cpp
        src/core/storage/storagemanager.cpp src/core/storage/storagemanager.h
        src/core/integration/namelessbridge.cpp src/core/integration/namelessbridge.h
        src/core/data/libraryrepository.cpp src/core/data/libraryrepository.h
        src/core/librarymodel.cpp src/core/librarymodel.h
        src/core/homefeed.cpp src/core/homefeed.h
        src/core/artwork.cpp
        src/core/libraryexchange.cpp src/core/libraryexchange.h
        src/core/settingsstore.cpp src/core/settingsstore.h
        src/core/importmanager.cpp src/core/importmanager.h
        src/core/playbackcontroller.cpp src/core/playbackcontroller.h
        src/core/web/streamproxy.cpp src/core/web/streamproxy.h
    src/core/audioreactivity.cpp src/core/audioreactivity.h
        src/core/audioequalizer.cpp src/core/audioequalizer.h
        src/core/downloads/dependencymanager.cpp src/core/downloads/dependencymanager.h
        src/core/downloads/importpreviewer.cpp src/core/downloads/importpreviewer.h
        src/core/downloads/nativedownload.cpp src/core/downloadmanager.cpp src/core/downloadmanager.h
        src/core/downloads/audiosources.cpp src/core/downloads/audiosources.h
        src/core/downloads/opencatalog.cpp src/core/downloads/opencatalog.h
        src/core/downloads/zaycevcatalog.cpp src/core/downloads/zaycevcatalog.h
        src/core/auth/browserauthmanager.cpp src/core/auth/browserauthmanager.h
        src/core/auth/oauthmanager.cpp src/core/auth/oauthmanager.h
        src/core/auth/webloginmanager_native.cpp src/core/web/mediasniffer.cpp src/core/web/mediasniffer.h
        src/core/web/servicepayload.cpp src/core/web/servicepayload.h
        src/core/auth/browsersession.cpp src/core/auth/browsersession.h
        src/core/auth/browsercookieimporter.cpp src/core/auth/browsercookieimporter.h
        src/core/auth/cdpclient.cpp src/core/auth/cdpclient.h
        src/core/auth/webloginmanager.cpp src/core/auth/webloginmanager.h
        src/core/vkprovider.cpp src/core/vkprovider.h)
    target_include_directories(aurora_tests PRIVATE src/core)
    target_link_libraries(aurora_tests PRIVATE Qt6::Test Qt6::Widgets Qt6::Sql Qt6::Multimedia Qt6::Network Qt6::Concurrent Qt6::WebEngineQuick)
    add_test(NAME aurora_library COMMAND aurora_tests)
    find_program(AURORA_NODE_EXECUTABLE node)
    if(AURORA_NODE_EXECUTABLE)
        add_test(NAME aurora_web_scripts COMMAND "${AURORA_NODE_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/web_scripts.mjs")
    endif()
    find_program(AURORA_PYTHON_EXECUTABLE python3)
    if(AURORA_PYTHON_EXECUTABLE)
        add_test(NAME aurora_browser_cookies COMMAND "${AURORA_PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/browser_cookies.py")
    endif()
    set_tests_properties(aurora_library PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;AURORA_HEADLESS=1" TIMEOUT 90)

    get_target_property(AURORA_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(AURORA_QT_BIN_DIRECTORY "${AURORA_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(AURORA_QMLTESTRUNNER_EXECUTABLE NAMES qmltestrunner qmltestrunner6 HINTS "${AURORA_QT_BIN_DIRECTORY}")
    if(AURORA_QMLTESTRUNNER_EXECUTABLE)
        add_test(NAME aurora_animations COMMAND "${AURORA_QMLTESTRUNNER_EXECUTABLE}" -input "${CMAKE_CURRENT_SOURCE_DIR}/tests/animations")
        set_tests_properties(aurora_animations PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software" TIMEOUT 30)
        add_test(NAME aurora_animations_null COMMAND "${AURORA_QMLTESTRUNNER_EXECUTABLE}" -input "${CMAKE_CURRENT_SOURCE_DIR}/tests/animations")
        set_tests_properties(aurora_animations_null PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=rhi;QSG_RHI_BACKEND=null" TIMEOUT 30)
    endif()
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    install(FILES packaging/org.nameless.Aurora.desktop DESTINATION share/applications)
    install(FILES packaging/aurora.svg DESTINATION share/icons/hicolor/scalable/apps RENAME org.nameless.Aurora.svg)
endif()

qt_add_resources(AuroraPlayer native_scripts PREFIX "/native/scripts" BASE src/core/web FILES src/core/web/harvest.js src/core/web/vk-library.js src/core/web/activate.js src/core/web/login.js src/core/web/browser_cookies.py src/core/web/public-source.js)
if(TARGET aurora_tests)
    qt_add_resources(aurora_tests native_scripts_test PREFIX "/native/scripts" BASE src/core/web FILES src/core/web/harvest.js src/core/web/vk-library.js src/core/web/activate.js src/core/web/login.js src/core/web/browser_cookies.py src/core/web/public-source.js)
endif()

qt_add_shaders(AuroraPlayer aurora_effects PREFIX "/aurora/shaders" BASE shaders FILES shaders/aurora-orb.frag)
