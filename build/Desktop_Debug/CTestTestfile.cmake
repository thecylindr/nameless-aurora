# CMake generated Testfile for 
# Source directory: /home/cylindr/nameless-aurora
# Build directory: /home/cylindr/nameless-aurora/build/Desktop_Debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test("aurora_library" "/home/cylindr/nameless-aurora/build/Desktop_Debug/aurora_tests")
set_tests_properties("aurora_library" PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;AURORA_HEADLESS=1" TIMEOUT "90" _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;201;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
add_test("aurora_web_scripts" "/usr/bin/node" "/home/cylindr/nameless-aurora/tests/web_scripts.mjs")
set_tests_properties("aurora_web_scripts" PROPERTIES  _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;204;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
add_test("aurora_browser_cookies" "/usr/bin/python3" "/home/cylindr/nameless-aurora/tests/browser_cookies.py")
set_tests_properties("aurora_browser_cookies" PROPERTIES  _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;208;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
add_test("aurora_animations" "/usr/bin/qmltestrunner" "-input" "/home/cylindr/nameless-aurora/tests/animations")
set_tests_properties("aurora_animations" PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software" TIMEOUT "30" _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;216;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
add_test("aurora_animations_null" "/usr/bin/qmltestrunner" "-input" "/home/cylindr/nameless-aurora/tests/animations")
set_tests_properties("aurora_animations_null" PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=rhi;QSG_RHI_BACKEND=null" TIMEOUT "30" _BACKTRACE_TRIPLES "/home/cylindr/nameless-aurora/CMakeLists.txt;218;add_test;/home/cylindr/nameless-aurora/CMakeLists.txt;0;")
