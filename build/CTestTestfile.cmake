# CMake generated Testfile for 
# Source directory: /home/cylindr/nameless-aurora
# Build directory: /home/cylindr/nameless-aurora/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test("aurora_library" "/home/cylindr/nameless-aurora/build/aurora_tests")
set_tests_properties("aurora_library" PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;AURORA_HEADLESS=1" TIMEOUT "90" _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;194;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
add_test("aurora_web_scripts" "/usr/bin/node" "/home/cylindr/nameless-aurora/tests/web_scripts.mjs")
set_tests_properties("aurora_web_scripts" PROPERTIES  _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;197;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
add_test("aurora_browser_cookies" "/usr/bin/python3" "/home/cylindr/nameless-aurora/tests/browser_cookies.py")
set_tests_properties("aurora_browser_cookies" PROPERTIES  _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;201;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
add_test("aurora_animations" "/usr/lib/qt6/bin/qmltestrunner" "-input" "/home/cylindr/nameless-aurora/tests/animations")
set_tests_properties("aurora_animations" PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software" TIMEOUT "30" _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;209;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
add_test("aurora_animations_null" "/usr/lib/qt6/bin/qmltestrunner" "-input" "/home/cylindr/nameless-aurora/tests/animations")
set_tests_properties("aurora_animations_null" PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=rhi;QSG_RHI_BACKEND=null" TIMEOUT "30" _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;211;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
