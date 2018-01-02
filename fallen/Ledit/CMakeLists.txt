set_target_properties(
    ledit
    PROPERTIES
        EXCLUDE_FROM_ALL ON
        EXCLUDE_FROM_DEFAULT_BUILD ON
        EXCLUDE_FROM_DEFAULT_BUILD_DEBUG OFF
)
target_compile_definitions(
    ledit
    PRIVATE
        $<$<CONFIG:Debug>:EDITOR>
        $<$<OR:$<CONFIG:MinSizeRel>,$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:FINAL>
)
target_include_directories(
    ledit
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../MFStdLib/Headers
        ${CMAKE_CURRENT_LIST_DIR}/../DDEngine/Headers
        ${CMAKE_CURRENT_LIST_DIR}/../DDLibrary/Headers
        ${CMAKE_CURRENT_LIST_DIR}/../Headers
        ${CMAKE_CURRENT_LIST_DIR}/Headers
)
target_sources(
    ledit
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/Headers/ed.h
        ${CMAKE_CURRENT_LIST_DIR}/Headers/gi.h
        ${CMAKE_CURRENT_LIST_DIR}/Headers/ledit.h
        ${CMAKE_CURRENT_LIST_DIR}/Source/ed.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Source/gi.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Source/ledit.cpp
)
