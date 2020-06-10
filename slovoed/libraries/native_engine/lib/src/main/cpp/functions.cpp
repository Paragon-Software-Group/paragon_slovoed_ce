#include "functions.h"

//#include "functions/all_functions.h"

static std::vector< INativeFunction< jbyteArray >* > gFunctions;

namespace NativeFunctions
{
void Init( const std::vector< INativeFunction< jbyteArray >* >& all_functions )
{
  gFunctions = all_functions;
}

void Uninit()
{
  for ( std::vector< INativeFunction< jbyteArray >* >::iterator iterator = gFunctions.begin() ;
        iterator != gFunctions.end() ; ++iterator )
  {
    delete *iterator;
  }
  gFunctions.clear();
}

int GetCount()
{
  return (signed) gFunctions.size();
}

INativeFunction< jbyteArray >* Get( int id )
{
  return gFunctions.at( (unsigned) id );
}

}

