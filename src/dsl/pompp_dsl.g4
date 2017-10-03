// /*
//  * The PomPP software including libraries and tools is developed by the
//  * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
//  * research project supported by the JST, CREST research program.
//  * The copyrights for code used under license from other parties are
//  * included in the corresponding files.
//  * 
//  * Copyright (c) 2017, The PomPP research team.
//  */

// POMPP DSL Grammar v0.83
// October 3, 2017

grammar pompp_dsl;

statements : ( loopStatement NL | createStatement NL | addStatement NL | deleteStatement NL | removeStatement NL | listStatement NL | getStatement NL| setStatement NL |
switchStatement NL | submitStatement NL | insertStatement NL | commentOutStatement NL )+;

createStatement : 'CREATE' Type object ;
deleteStatement : 'DELETE' object ;
addStatement : 'ADD' object Property ( Integer | Floating | Absolute_Path | String | Options ) ;
removeStatement : 'REMOVE' object Property ;

listStatement : 'LIST' ( Type | 'ALL' ) ;
getStatement : 'GET' object ( Property | 'ALL' ) ;
setStatement : 'SET' object Property ( Integer | Floating | Absolute_Path | String | Options ) ;

switchStatement : 'SWITCH' object ;
submitStatement : 'SUBMIT' object ;

insertStatement : 'INSERT' Absolute_Path Integer Source_Language ( POMPP_Library_Call_App | POMPP_Library_Call_Sec String ) ;
commentOutStatement : 'COMMENTOUT' Absolute_Path Integer Source_Language ;

// types and properties
Type : 'MACHINE' | 'JOB' | 'MODEL' ;
Property : 'OBJECT_TYPE' | 'JOB_TYPE' | 'MODEL_PATH' | 'MODEL_TO_USE' | 'OPTIONS' | 'EXEC_PATH' | 'NUM_OF_THREADS' | 'ELAPSED_TIME' | 'ITERATIONS' | 'PVT_PATH' | 'MODULE_POWER' | 'CONTROL_MODE' | 'PROFILE_NAME' | 'POMPP_NPKGS_PER_NODE' | 'POMPP_NCORES_PER_PKG' | 'POMPP_TOTAL_NODES' | 'POMPP_MAX_FREQ' | 'POMPP_MIN_FREQ' | 'POMPP_PKG_TDP' | 'POMPP_DRAM_TDP' | 'POMPP_PKG_MIN' | 'POMPP_DRAM_MIN' | 'POMPP_MODULE_MIN' ;
Source_Language : 'C' | 'Fortran' ;
POMPP_Library_Call_App : 'POMPP_Init' | 'POMPP_Finalize' ;
POMPP_Library_Call_Sec : 'POMPP_Start_Sec' | 'POMPP_Stop_Sec' ;

// basic syntax
Integer : '0'..'9' | '1'..'9''0'..'9'+ ;
Floating : '0'..'9''.'('0'..'9')+ | '1'..'9'('0'..'9')+'.'('0'..'9')+ ;
String : [-.:a-zA-Z0-9_]+ ;

Absolute_Path : ( '/' String )+ ;
Options : '"' ~('\r' | '\n')* '"' ;

// object
object : String | arrayAccessor ;

// array
arrayAccessor : String arrayNotation ;
arrayNotation:  '[' Integer ']' ;

// loop
loopStatement : 'LOOP' '(' loopCounter ')' 'DO' statements 'END' 'LOOP' ;
loopCounter :  'IN' Integer '->' Integer ;

NL  : '\r'? '\n' ;

LINE_COMMENT : ('#' ~('\r' | '\n')* '\r'? '\n') -> skip;
WS : (' '|'\t')+ -> skip;
