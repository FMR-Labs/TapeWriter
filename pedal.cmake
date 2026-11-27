# Sources de la pédale, partagées par le VST3 autonome et le board.
# Chemins calculés depuis CE fichier et normalisés (sans ".."), pour que
# deux pédales qui listent le même fichier FMR produisent la même chaîne.
get_filename_component(_FMR "${CMAKE_CURRENT_LIST_DIR}/../FMR_Library" ABSOLUTE)

list(APPEND PEDAL_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/TapeWriterDSP.cpp
  ${CMAKE_CURRENT_LIST_DIR}/TapeWriterGUI.cpp
  ${_FMR}/FMR_Lib/FMR_Neural.cpp
  ${_FMR}/FMR_Lib/FMR_DSP_float.cpp
  ${_FMR}/FMR_NeuralDrive/FMR_TapeWriter.cpp
  ${_FMR}/FMR_NeuralDrive/FMR_Boost_JFET.cpp)
  
set_source_files_properties(${_FMR}/FMR_Lib/FMR_Neural.cpp PROPERTIES COMPILE_OPTIONS "/arch:AVX2;/fp:fast")