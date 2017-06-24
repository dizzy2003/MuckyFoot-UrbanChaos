set(MFSTDLIB_HEADERS
    Headers/MFStdLib.h
    Headers/StdFile.h
    Headers/StdKeybd.h
    Headers/StdMaths.h
    Headers/StdMem.h
    Headers/StdMouse.h)
set(MFSTDLIB_SOURCES
    Source/StdLib/StdFile.cpp
    Source/StdLib/StdMaths.cpp
    Source/StdLib/StdMem.cpp)
source_group(Headers
    FILES ${MFSTDLIB_HEADERS})
source_group(Source
    FILES ${MFSTDLIB_SOURCES})
add_library(MFStdLib STATIC
    ${MFSTDLIB_HEADERS}
    ${MFSTDLIB_SOURCES})
target_include_directories(MFStdLib
    PUBLIC Headers)
