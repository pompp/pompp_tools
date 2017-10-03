# /*
#  * The PomPP software including libraries and tools is developed by the
#  * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
#  * research project supported by the JST, CREST research program.
#  * The copyrights for code used under license from other parties are
#  * included in the corresponding files.
#  * 
#  * Copyright (c) 2017, The PomPP research team.
#  */

.SUFFIXES: .g4 .java .class .jar
.PHONY: all clean

all: pompp_dsl.jar

pompp_dsl.jar: pompp_dsl.class
	jar cvf ./jar/pompp_dsl.jar ./class ./src ./*.java

pompp_dsl.class: pompp_dsl.tokens
	javac -Xlint:deprecation ./*.java ./src/*.java -d ./class

pompp_dsl.tokens: pompp_dsl.g4
	java org.antlr.v4.Tool pompp_dsl.g4

clean:
	rm -rf ./*.java ./*.tokens ./class/* *.jar
