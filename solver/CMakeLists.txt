
include_directories( ${CMAKE_SOURCE_DIR}/kmines/generic ${CMAKE_SOURCE_DIR}/kmines ${CMAKE_SOURCE_DIR}/libkdegames  )


########### next target ###############


########### next target ###############

set(test_SRCS test.cpp )

kde4_automoc(test ${test_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test ${test_SRCS})

target_link_libraries(test  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testFast_SRCS testFast.cpp ${libsolver_SRCS} )

kde4_automoc(testFast ${testFast_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testFast ${testFast_SRCS})

target_link_libraries(testFast  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testSolve_SRCS testSolve.cpp )

kde4_automoc(testSolve ${testSolve_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testSolve ${testSolve_SRCS})

target_link_libraries(testSolve  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testRate_SRCS testRate.cpp ${libsolver_SRCS} )

kde4_automoc(testRate ${testRate_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testRate ${testRate_SRCS})

target_link_libraries(testRate  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############

