// fendsave.cpp
//
// This saves the Heap header bytes required to keep the heap working even though we
// are overloading it with 

#include <MFStdLib.h>

char *fendsave_dummy="|......";
// And that's it, the overlay loader will manage these bytes for us.