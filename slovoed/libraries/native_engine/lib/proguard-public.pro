-dontskipnonpubliclibraryclassmembers

-keep class com.paragon_software.engine.nativewrapper.NativeFunctions {
 private static java.lang.Object ncallback(...);
 private static native java.lang.Object ncall(...);
}

-keep class com.paragon_software.engine.SlovoedEngine { *; }
-keep class com.paragon_software.engine.ExternalBasesHolder { *; }
-keep class com.paragon_software.engine.nativewrapper.NativeDictionary { *; }
-keep class com.paragon_software.engine.rx.base.BaseEngineTaskRunner { *; }
-keep class com.paragon_software.engine.rx.base.BaseEngineTaskRunner$SaveCurrentTask { *; }
-keep class com.paragon_software.engine.rx.deserializearticle.DeserializerUtils { *; }
-keep class com.paragon_software.engine.rx.deserializearticle.DeserializeArticlesTaskRunner$ForCallback { *; }
-keep class com.paragon_software.engine.rx.scrollandfts.SearchProcessor { *; }
-keep class com.paragon_software.engine.rx.scrollandfts.ResultContainer { *; }
-keep class com.paragon_software.engine.rx.scrollandfts.ResultContainer$ParagonSearchResult { *; }
-keep class com.paragon_software.engine.rx.preloadedwords.PreloadedWordsNativeCallback { *; }
-keep class com.paragon_software.engine.rx.preloadedwords.PreloadedWordsNativeCallback$MutableDirectory { *; }
-keep class com.paragon_software.engine.nativewrapper.ArticleItemFactory { *; }