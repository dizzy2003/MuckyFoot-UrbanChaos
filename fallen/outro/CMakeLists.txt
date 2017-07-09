set(OUTRO_HEADERS
    Matrix.h
    Tga.h
    back.h
    cam.h
    credits.h
    imp.h
    font.h
    mf.h
    wire.h)
set(OUTRO_SOURCES
    Matrix.cpp
    Tga.cpp
    back.cpp
    cam.cpp
    credits.cpp
    font.cpp
    imp.cpp
    main.cpp
    mf.cpp
    os.cpp
    wire.cpp)
source_group(Headers
    FILES ${OUTRO_HEADERS})
source_group(Source
    FILES ${OUTRO_SOURCES})
add_library(Outro STATIC
    ${OUTRO_HEADERS}
    ${OUTRO_SOURCES})
target_include_directories(Outro
    PRIVATE
        ../..
        ../../MFStdLib/Headers)
