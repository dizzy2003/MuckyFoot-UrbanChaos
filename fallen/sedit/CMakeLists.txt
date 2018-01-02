set_target_properties(
    sedit
    PROPERTIES
        EXCLUDE_FROM_ALL ON
        EXCLUDE_FROM_DEFAULT_BUILD ON
        EXCLUDE_FROM_DEFAULT_BUILD_DEBUG OFF
)
target_compile_definitions(
    sedit
    PRIVATE
        $<$<CONFIG:Debug>:EDITOR>
        $<$<OR:$<CONFIG:MinSizeRel>,$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:FINAL>
)
target_include_directories(
    sedit
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/..
        ${CMAKE_CURRENT_LIST_DIR}/../../MFStdLib/Headers
        ${CMAKE_CURRENT_LIST_DIR}/../DDEngine/Headers
        ${CMAKE_CURRENT_LIST_DIR}/../DDLibrary/Headers
        ${CMAKE_CURRENT_LIST_DIR}/../Headers
        ${CMAKE_CURRENT_LIST_DIR}/../Ledit/Headers
        ${CMAKE_CURRENT_LIST_DIR}/Headers
)
target_sources(
    sedit
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/Headers/es.h
        ${CMAKE_CURRENT_LIST_DIR}/Headers/sedit.h
        ${CMAKE_CURRENT_LIST_DIR}/Source/es.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Source/sedit.cpp
)
