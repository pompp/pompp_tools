// /*
//  * The PomPP software including libraries and tools is developed by the
//  * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
//  * research project supported by the JST, CREST research program.
//  * The copyrights for code used under license from other parties are
//  * included in the corresponding files.
//  * 
//  * Copyright (c) 2017, The PomPP research team.
//  */

// Generated from pompp_dsl.g4 by ANTLR 4.7
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link pompp_dslParser}.
 */
public interface pompp_dslListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#statements}.
	 * @param ctx the parse tree
	 */
	void enterStatements(pompp_dslParser.StatementsContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#statements}.
	 * @param ctx the parse tree
	 */
	void exitStatements(pompp_dslParser.StatementsContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#createStatement}.
	 * @param ctx the parse tree
	 */
	void enterCreateStatement(pompp_dslParser.CreateStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#createStatement}.
	 * @param ctx the parse tree
	 */
	void exitCreateStatement(pompp_dslParser.CreateStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#deleteStatement}.
	 * @param ctx the parse tree
	 */
	void enterDeleteStatement(pompp_dslParser.DeleteStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#deleteStatement}.
	 * @param ctx the parse tree
	 */
	void exitDeleteStatement(pompp_dslParser.DeleteStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#addStatement}.
	 * @param ctx the parse tree
	 */
	void enterAddStatement(pompp_dslParser.AddStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#addStatement}.
	 * @param ctx the parse tree
	 */
	void exitAddStatement(pompp_dslParser.AddStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#removeStatement}.
	 * @param ctx the parse tree
	 */
	void enterRemoveStatement(pompp_dslParser.RemoveStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#removeStatement}.
	 * @param ctx the parse tree
	 */
	void exitRemoveStatement(pompp_dslParser.RemoveStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#listStatement}.
	 * @param ctx the parse tree
	 */
	void enterListStatement(pompp_dslParser.ListStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#listStatement}.
	 * @param ctx the parse tree
	 */
	void exitListStatement(pompp_dslParser.ListStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#getStatement}.
	 * @param ctx the parse tree
	 */
	void enterGetStatement(pompp_dslParser.GetStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#getStatement}.
	 * @param ctx the parse tree
	 */
	void exitGetStatement(pompp_dslParser.GetStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#setStatement}.
	 * @param ctx the parse tree
	 */
	void enterSetStatement(pompp_dslParser.SetStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#setStatement}.
	 * @param ctx the parse tree
	 */
	void exitSetStatement(pompp_dslParser.SetStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#switchStatement}.
	 * @param ctx the parse tree
	 */
	void enterSwitchStatement(pompp_dslParser.SwitchStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#switchStatement}.
	 * @param ctx the parse tree
	 */
	void exitSwitchStatement(pompp_dslParser.SwitchStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#submitStatement}.
	 * @param ctx the parse tree
	 */
	void enterSubmitStatement(pompp_dslParser.SubmitStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#submitStatement}.
	 * @param ctx the parse tree
	 */
	void exitSubmitStatement(pompp_dslParser.SubmitStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#insertStatement}.
	 * @param ctx the parse tree
	 */
	void enterInsertStatement(pompp_dslParser.InsertStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#insertStatement}.
	 * @param ctx the parse tree
	 */
	void exitInsertStatement(pompp_dslParser.InsertStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#commentOutStatement}.
	 * @param ctx the parse tree
	 */
	void enterCommentOutStatement(pompp_dslParser.CommentOutStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#commentOutStatement}.
	 * @param ctx the parse tree
	 */
	void exitCommentOutStatement(pompp_dslParser.CommentOutStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#object}.
	 * @param ctx the parse tree
	 */
	void enterObject(pompp_dslParser.ObjectContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#object}.
	 * @param ctx the parse tree
	 */
	void exitObject(pompp_dslParser.ObjectContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#arrayAccessor}.
	 * @param ctx the parse tree
	 */
	void enterArrayAccessor(pompp_dslParser.ArrayAccessorContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#arrayAccessor}.
	 * @param ctx the parse tree
	 */
	void exitArrayAccessor(pompp_dslParser.ArrayAccessorContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#arrayNotation}.
	 * @param ctx the parse tree
	 */
	void enterArrayNotation(pompp_dslParser.ArrayNotationContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#arrayNotation}.
	 * @param ctx the parse tree
	 */
	void exitArrayNotation(pompp_dslParser.ArrayNotationContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#loopStatement}.
	 * @param ctx the parse tree
	 */
	void enterLoopStatement(pompp_dslParser.LoopStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#loopStatement}.
	 * @param ctx the parse tree
	 */
	void exitLoopStatement(pompp_dslParser.LoopStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link pompp_dslParser#loopCounter}.
	 * @param ctx the parse tree
	 */
	void enterLoopCounter(pompp_dslParser.LoopCounterContext ctx);
	/**
	 * Exit a parse tree produced by {@link pompp_dslParser#loopCounter}.
	 * @param ctx the parse tree
	 */
	void exitLoopCounter(pompp_dslParser.LoopCounterContext ctx);
}
