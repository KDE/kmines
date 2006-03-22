kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kmines/generic ${CMAKE_SOURCE_DIR}/kmines ${CMAKE_SOURCE_DIR}/libkdegames ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(test_SRCS test.cpp )

kde4_automoc(${test_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test ${test_SRCS})

target_link_libraries(test  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testFast_SRCS testFast.cpp )

kde4_automoc(${testFast_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testFast ${testFast_SRCS})

target_link_libraries(testFast  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testSolve_SRCS testSolve.cpp )

kde4_automoc(${testSolve_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testSolve ${testSolve_SRCS})

target_link_libraries(testSolve  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testRate_SRCS testRate.cpp )

kde4_automoc(${testRate_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testRate ${testRate_SRCS})

target_link_libraries(testRate  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kmines/generic -I$(top_srcdir)/kmines -I$(top_srcdir)/libkdegames $(all_includes)
#
#noinst_LTLIBRARIES   = libsolver.la
#noinst_HEADERS       = bfield.h solver.h headerP.h adviseFast.h adviseFull.h
#libsolver_la_SOURCES = bfield.cpp solver.cpp advFastRules.cpp adviseFast.cpp \
#                       adviseFull.cpp
#METASOURCES          = solver.moc
#
#check_PROGRAMS = test testFast testSolve testRate
#LDADD = $(LIB_KDEGAMES) ./libsolver.la
#test_SOURCES = test.cpp
#testFast_SOURCES = testFast.cpp
#testSolve_SOURCES = testSolve.cpp
#testRate_SOURCES = testRate.cpp
