INCLUDES = -I$(top_srcdir)/kmines/generic -I$(top_srcdir)/kmines $(all_includes)

noinst_LTLIBRARIES   = libsolver.la
noinst_HEADERS       = bfield.h solver.h headerP.h adviseFast.h adviseFull.h
libsolver_la_SOURCES = bfield.cpp solver.cpp advFastRules.cpp adviseFast.cpp \
                       adviseFull.cpp
METASOURCES          = solver.moc

check_PROGRAMS = test testFast testSolve testRate
LDADD = ../generic/libkdehighscores.la ./libsolver.la
test_SOURCES = test.cpp
testFast_SOURCES = testFast.cpp
testSolve_SOURCES = testSolve.cpp
testRate_SOURCES = testRate.cpp

messages:
	$(XGETTEXT) *.cc *.h -o $(podir)/kmines.pot
