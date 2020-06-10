-keep class com.paragon_software.utils_slovoed.font.Font { *; }
-keep class com.paragon_software.utils_slovoed.font.FontsUtils { *; }

# json data classes
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.About { *; }
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.Catalog { *; }
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.CatalogAbout { *; }
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.LocaleData { *; }
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.AcesProperties { *; }
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.Extra { *; }

# xml data classes
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.Data { *; }
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.DictsPattern { *; }
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.LangTranslations { *; }
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.Language { *; }
-keepclassmembers class com.paragon_software.utils_slovoed.pdahpc.Strings { *; }

# org.simpleframework:simple-xml
-dontwarn org.simpleframework.xml.stream.**

-keep public class org.simpleframework.** { *; }
-keep class org.simpleframework.xml.** { *; }
-keep class org.simpleframework.xml.core.** { *; }
-keep class org.simpleframework.xml.util.** { *; }

-keepattributes *Annotation*, Signature

-keepclassmembers class * {
    @org.simpleframework.xml.* *;
}
