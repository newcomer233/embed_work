# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/newcomer233/Desktop/embed_final_v09/build/f/src/fmt_external"
  "/home/newcomer233/Desktop/embed_final_v09/build/f/src/fmt_external-build"
  "/home/newcomer233/Desktop/embed_final_v09/build/f"
  "/home/newcomer233/Desktop/embed_final_v09/build/f/tmp"
  "/home/newcomer233/Desktop/embed_final_v09/build/f/src/fmt_external-stamp"
  "/home/newcomer233/Desktop/embed_final_v09/build/f/src"
  "/home/newcomer233/Desktop/embed_final_v09/build/f/src/fmt_external-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/newcomer233/Desktop/embed_final_v09/build/f/src/fmt_external-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/newcomer233/Desktop/embed_final_v09/build/f/src/fmt_external-stamp${cfgdir}") # cfgdir has leading slash
endif()
