//
// Created by borbat on 11.09.2018.
//

#include <vector>
#include "GetListVariants.h"
#include "java/java_objects.h"

jobject GetListVariants::native( JNIEnv* env, CSldDictionary* dictionary, jobject listNumber ) const noexcept
{
  const CSldListInfo* pListInfo;
  std::vector< jint > variants;
  ESldError error = dictionary->GetWordListInfo( JavaObjects::GetInteger( env, listNumber ), &pListInfo );
  if ( error == eOK )
  {
    UInt32 i, n = pListInfo->GetNumberOfVariants();
    variants.reserve( n );
    for ( i = 0u ; i < n ; i++ )
    {
      variants.push_back( pListInfo->GetVariantType( i ) );
    }
  }
  jsize count = (jsize) variants.size();
  jintArray res = env->NewIntArray( count );
  env->SetIntArrayRegion( res, 0u, count, variants.data() );
  return res;
}
