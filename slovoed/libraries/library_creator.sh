#!/bin/sh

if [ "$#" -lt 2 ]; then
  echo "Usage $0 <Component Name> <Component Description>"
else
  if [ -e "$1" ] ; then
    echo "Component $1 exists"
  else
    if ! cp -R _lib_template "$1" ; then
      echo "Can't prepare project"
    else
      sed "s/%%LIB_NAME%%/$1/g;s/%%LIB_DESCRIPTION%%/$2/g" "$1/lib/src/main/AndroidManifest.xml" > "$1/lib/src/main/AndroidManifest.xml.new"
      mv -f "$1/lib/src/main/AndroidManifest.xml.new" "$1/lib/src/main/AndroidManifest.xml"
      sed "s/%%LIB_NAME%%/$1/g;s/%%LIB_DESCRIPTION%%/$2/g" "$1/lib/build-version.gradle" > "$1/lib/build-version.gradle.new"
      mv -f "$1/lib/build-version.gradle.new" "$1/lib/build-version.gradle"
#      svn add "$1"
#      find "$1" -iname '*.cpp' -exec svn ps svn:eol-style native {} \; -exec svn ps svn:keywords Id {} \;
#      find "$1" -iname '*.h' -exec svn ps svn:eol-style native {} \; -exec svn ps svn:keywords Id {} \;
#      find "$1" -iname '*.java' -exec svn ps svn:eol-style native {} \; -exec svn ps svn:keywords Id {} \;
#      find "$1" -iname '*.mk' -exec svn ps svn:eol-style native {} \; -exec svn ps svn:keywords Id {} \;
#      find "$1" -iname '*.properties' -exec svn ps svn:eol-style native {} \; -exec svn ps svn:keywords Id {} \;
#      find "$1" -iname '*.gradle' -exec svn ps svn:eol-style native {} \; -exec svn ps svn:keywords Id {} \;
#      find "$1" -iname '*.txt' -exec svn ps svn:eol-style native {} \; -exec svn ps svn:keywords Id {} \;
#      find "$1" -iname '*.pro' -exec svn ps svn:eol-style native {} \; -exec svn ps svn:keywords Id {} \;
#      find "$1" -iname '*.xml' -exec svn ps svn:eol-style native {} \; -exec svn ps svn:keywords Id {} \;  -exec svn ps svn:mime-type text/xml {} \;
#      svn ps svn:ignore -F svn.ignore "$1"
#      svn ps svn:ignore -F svn.ignore "$1/lib"
    fi
  fi 
fi
