# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "AuroraPlayer_autogen"
  "CMakeFiles/AuroraPlayer_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/AuroraPlayer_autogen.dir/ParseCache.txt"
  )
endif()
