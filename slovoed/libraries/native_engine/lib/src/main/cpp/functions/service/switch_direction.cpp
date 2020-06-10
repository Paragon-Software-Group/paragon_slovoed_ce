//
// Created by kovtunov on 25.12.18.
//

#include <java/java_objects.h>
#include "switch_direction.h"

jobject SwitchDirection::native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _searchQuery
) const noexcept
{
    jint listIndex = JavaObjects::GetInteger( env, _listIndex );
    jint selectedListIndex = -1;
    JavaObjects::string searchQuery = JavaObjects::GetString( env, _searchQuery );

    if ( searchQuery.length() > 0u )
    {
        ESldError error = dictionary->SetCurrentWordlist( listIndex );
        if ( error == eOK )
        {
            UInt32 ResultFlag = 0;
            dictionary->SwitchDirection(searchQuery.c_str(), &ResultFlag);
            if (1 == ResultFlag)
            {
                dictionary->GetCurrentWordList(&selectedListIndex);
            }
        }
    }

    return JavaObjects::GetInteger( env, selectedListIndex );
}
