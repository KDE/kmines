include_directories( ${CMAKE_SOURCE_DIR}/kmines/generic ${CMAKE_SOURCE_DIR}/kmines ${CMAKE_SOURCE_DIR}/libkdegames  )

set(test_SRCS test.cpp )

kde4_add_test_executable(test ${test_SRCS})

target_link_libraries(test  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(testFast_SRCS testFast.cpp ${libsolver_SRCS} )

kde4_add_test_executable(testFast ${testFast_SRCS})

target_link_libraries(testFast  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(testSolve_SRCS testSolve.cpp ${libsolver_SRCS} )

kde4_add_test_executable(testSolve ${testSolve_SRCS})

target_link_libraries(testSolve  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(testRate_SRCS testRate.cpp ${libsolver_SRCS} )

kde4_add_test_executable(testRate ${testRate_SRCS})

target_link_libraries(testRate  ${KDE4_KDEUI_LIBS} )



