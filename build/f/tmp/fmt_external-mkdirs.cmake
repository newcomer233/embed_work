# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/lkj/piper/build/f/src/fmt_external"
  "/home/lkj/piper/build/f/src/fmt_external-build"
  "/home/lkj/piper/build/f"
  "/home/lkj/piper/build/f/tmp"
  "/home/lkj/piper/build/f/src/fmt_external-stamp"
  "/home/lkj/piper/build/f/src"
  "/home/lkj/piper/build/f/src/fmt_external-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/lkj/piper/build/f/src/fmt_external-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/lkj/piper/build/f/src/fmt_external-stamp${cfgdir}") # cfgdir has leading slash
endif()
