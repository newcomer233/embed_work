# CMake generated Testfile for 
# Source directory: /home/lkj/piper
# Build directory: /home/lkj/piper/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_piper "/home/lkj/piper/build/test_piper" "/home/lkj/piper/etc/test_voice.onnx" "/home/lkj/piper/build/pi/share/espeak-ng-data" "/home/lkj/piper/build/test.wav")
set_tests_properties(test_piper PROPERTIES  _BACKTRACE_TRIPLES "/home/lkj/piper/CMakeLists.txt;110;add_test;/home/lkj/piper/CMakeLists.txt;0;")
