include("/home/cylindr/nameless-aurora/build/Desktop_Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/AuroraPlayer-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase;qtdeclarative;qtmultimedia;qtwebengine")

qt6_deploy_qml_imports(TARGET AuroraPlayer PLUGINS_FOUND plugins_found)
qt6_deploy_runtime_dependencies(
    EXECUTABLE "/home/cylindr/nameless-aurora/build/Desktop_Debug/AuroraPlayer"
    ADDITIONAL_MODULES ${plugins_found}
    GENERATE_QT_CONF
    NO_TRANSLATIONS
)