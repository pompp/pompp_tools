// /*
//  * The PomPP software including libraries and tools is developed by the
//  * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
//  * research project supported by the JST, CREST research program.
//  * The copyrights for code used under license from other parties are
//  * included in the corresponding files.
//  * 
//  * Copyright (c) 2017, The PomPP research team.
//  */

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import java.io.FileInputStream;
import java.io.InputStream;

public class pompp_dsl {
	public static void main(String[] args) throws Exception {
		System.out.println( "POMPP Tools DSL alpha test version..." );
		InputStream is = args.length > 0 ? new FileInputStream(args[0]) : System.in;
	        ANTLRInputStream ais = new ANTLRInputStream(is);	
		pompp_dslLexer lexer = new pompp_dslLexer(ais);
		CommonTokenStream tokens = new CommonTokenStream(lexer);
		pompp_dslParser parser = new pompp_dslParser(tokens);
		ParseTree tree = parser.statements(); // parse
		ParseTreeWalker walker = new ParseTreeWalker(); // create standard walker
		statementsListener POMPP_DSL_Implementation = new statementsListener(parser);
		walker.walk(POMPP_DSL_Implementation, tree); // initiate walk of tree with listener
	}
}
