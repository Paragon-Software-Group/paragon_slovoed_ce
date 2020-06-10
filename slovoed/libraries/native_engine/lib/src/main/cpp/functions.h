#ifndef NATIVE_FUNCTIONS_H
#define NATIVE_FUNCTIONS_H

#include <jni.h>
#include <vector>

#include "functions/i_native_function.h"

namespace NativeFunctions
{
void Init( const std::vector< INativeFunction< jbyteArray >* >& all_functions );

void Uninit();

int GetCount();

INativeFunction< jbyteArray >* Get( int id );
}

#endif //NATIVE_FUNCTIONS_H
