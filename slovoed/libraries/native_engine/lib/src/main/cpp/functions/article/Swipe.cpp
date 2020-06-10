
#include <java/java_objects.h>
#include <SldDictionaryHelper.h>
#include "Swipe.h"

jobject Swipe::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _listIndex,
    jobject _globalIndex,
    jobject _swipingDirection,
    jobject _swipingMode
) const noexcept
{
  jint listIndex = JavaObjects::GetInteger( env, _listIndex );
  jint globalIndex = JavaObjects::GetInteger( env, _globalIndex );
  jint swipingMode = JavaObjects::GetInteger( env, _swipingMode );
  jint swipingDirection = JavaObjects::GetInteger( env, _swipingDirection );
  CSldDictionaryHelper dictionaryHelper = CSldDictionaryHelper( *dictionary );
  jint res = -1;

  ESldError error = dictionary->SetCurrentWordlist( listIndex );
  if ( error == eOK )
  {
    error = dictionary->GoToByGlobalIndex( globalIndex );
    if ( error == eOK )
    {
      error = dictionaryHelper.Swipe( swipingDirection, static_cast<ESwipingMode>(swipingMode), NULL );
      if ( error == eOK )
      {
        Int32 articleGlobalIndex;
        error = dictionary->GetCurrentGlobalIndex( &articleGlobalIndex );
        if ( error == eOK )
        {
          res = articleGlobalIndex;
        }
      }
    }
  }
  return JavaObjects::GetInteger( env, res );
}
