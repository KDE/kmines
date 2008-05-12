INCLUDES = -I$(top_srcdir)/kmines/generic -I$(top_srcdir)/kmines -I$(top_srcdir)/libkdegames $(all_includes)

noinst_LTLIBRARIES   = libsolver.la
noinst_HEADERS       = bfield.h solver.h headerP.h adviseFast.h adviseFull.h
libsolver_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libsolver_la_SOURCES = bfield.cpp solver.cpp advFastRules.cpp adviseFast.cpp \
                       adviseFull.cpp
METASOURCES          = solver.moc

check_PROGRAMS = test testFast testSolve testRate
test_SOURCES = test.cpp
test_LDADD = ./libsolver.la $(LIB_KDECORE)
testFast_SOURCES = testFast.cpp
testFast_LDADD = ./libsolver.la $(LIB_KDECORE)
testSolve_SOURCES = testSolve.cpp
testSolve_LDADD = ./libsolver.la $(LIB_KDECORE) $(LIB_KDEUI)
testRate_SOURCES = testRate.cpp
testRate_LDADD = ./libsolver.la $(LIB_KDECORE) $(LIB_KDEUI)
