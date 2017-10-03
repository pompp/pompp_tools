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
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.misc.*;
import org.antlr.v4.runtime.tree.*;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class pompp_dslParser extends Parser {
	static { RuntimeMetaData.checkVersion("4.7", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__0=1, T__1=2, T__2=3, T__3=4, T__4=5, T__5=6, T__6=7, T__7=8, T__8=9, 
		T__9=10, T__10=11, T__11=12, T__12=13, T__13=14, T__14=15, T__15=16, T__16=17, 
		T__17=18, T__18=19, T__19=20, T__20=21, Type=22, Property=23, Source_Language=24, 
		POMPP_Library_Call_App=25, POMPP_Library_Call_Sec=26, Integer=27, Floating=28, 
		String=29, Absolute_Path=30, Options=31, NL=32, LINE_COMMENT=33, WS=34;
	public static final int
		RULE_statements = 0, RULE_createStatement = 1, RULE_deleteStatement = 2, 
		RULE_addStatement = 3, RULE_removeStatement = 4, RULE_listStatement = 5, 
		RULE_getStatement = 6, RULE_setStatement = 7, RULE_switchStatement = 8, 
		RULE_submitStatement = 9, RULE_insertStatement = 10, RULE_commentOutStatement = 11, 
		RULE_object = 12, RULE_arrayAccessor = 13, RULE_arrayNotation = 14, RULE_loopStatement = 15, 
		RULE_loopCounter = 16;
	public static final String[] ruleNames = {
		"statements", "createStatement", "deleteStatement", "addStatement", "removeStatement", 
		"listStatement", "getStatement", "setStatement", "switchStatement", "submitStatement", 
		"insertStatement", "commentOutStatement", "object", "arrayAccessor", "arrayNotation", 
		"loopStatement", "loopCounter"
	};

	private static final String[] _LITERAL_NAMES = {
		null, "'CREATE'", "'DELETE'", "'ADD'", "'REMOVE'", "'LIST'", "'ALL'", 
		"'GET'", "'SET'", "'SWITCH'", "'SUBMIT'", "'INSERT'", "'COMMENTOUT'", 
		"'['", "']'", "'LOOP'", "'('", "')'", "'DO'", "'END'", "'IN'", "'->'"
	};
	private static final String[] _SYMBOLIC_NAMES = {
		null, null, null, null, null, null, null, null, null, null, null, null, 
		null, null, null, null, null, null, null, null, null, null, "Type", "Property", 
		"Source_Language", "POMPP_Library_Call_App", "POMPP_Library_Call_Sec", 
		"Integer", "Floating", "String", "Absolute_Path", "Options", "NL", "LINE_COMMENT", 
		"WS"
	};
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}

	@Override
	public String getGrammarFileName() { return "pompp_dsl.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public ATN getATN() { return _ATN; }

	public pompp_dslParser(TokenStream input) {
		super(input);
		_interp = new ParserATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}
	public static class StatementsContext extends ParserRuleContext {
		public List<LoopStatementContext> loopStatement() {
			return getRuleContexts(LoopStatementContext.class);
		}
		public LoopStatementContext loopStatement(int i) {
			return getRuleContext(LoopStatementContext.class,i);
		}
		public List<TerminalNode> NL() { return getTokens(pompp_dslParser.NL); }
		public TerminalNode NL(int i) {
			return getToken(pompp_dslParser.NL, i);
		}
		public List<CreateStatementContext> createStatement() {
			return getRuleContexts(CreateStatementContext.class);
		}
		public CreateStatementContext createStatement(int i) {
			return getRuleContext(CreateStatementContext.class,i);
		}
		public List<AddStatementContext> addStatement() {
			return getRuleContexts(AddStatementContext.class);
		}
		public AddStatementContext addStatement(int i) {
			return getRuleContext(AddStatementContext.class,i);
		}
		public List<DeleteStatementContext> deleteStatement() {
			return getRuleContexts(DeleteStatementContext.class);
		}
		public DeleteStatementContext deleteStatement(int i) {
			return getRuleContext(DeleteStatementContext.class,i);
		}
		public List<RemoveStatementContext> removeStatement() {
			return getRuleContexts(RemoveStatementContext.class);
		}
		public RemoveStatementContext removeStatement(int i) {
			return getRuleContext(RemoveStatementContext.class,i);
		}
		public List<ListStatementContext> listStatement() {
			return getRuleContexts(ListStatementContext.class);
		}
		public ListStatementContext listStatement(int i) {
			return getRuleContext(ListStatementContext.class,i);
		}
		public List<GetStatementContext> getStatement() {
			return getRuleContexts(GetStatementContext.class);
		}
		public GetStatementContext getStatement(int i) {
			return getRuleContext(GetStatementContext.class,i);
		}
		public List<SetStatementContext> setStatement() {
			return getRuleContexts(SetStatementContext.class);
		}
		public SetStatementContext setStatement(int i) {
			return getRuleContext(SetStatementContext.class,i);
		}
		public List<SwitchStatementContext> switchStatement() {
			return getRuleContexts(SwitchStatementContext.class);
		}
		public SwitchStatementContext switchStatement(int i) {
			return getRuleContext(SwitchStatementContext.class,i);
		}
		public List<SubmitStatementContext> submitStatement() {
			return getRuleContexts(SubmitStatementContext.class);
		}
		public SubmitStatementContext submitStatement(int i) {
			return getRuleContext(SubmitStatementContext.class,i);
		}
		public List<InsertStatementContext> insertStatement() {
			return getRuleContexts(InsertStatementContext.class);
		}
		public InsertStatementContext insertStatement(int i) {
			return getRuleContext(InsertStatementContext.class,i);
		}
		public List<CommentOutStatementContext> commentOutStatement() {
			return getRuleContexts(CommentOutStatementContext.class);
		}
		public CommentOutStatementContext commentOutStatement(int i) {
			return getRuleContext(CommentOutStatementContext.class,i);
		}
		public StatementsContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_statements; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterStatements(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitStatements(this);
		}
	}

	public final StatementsContext statements() throws RecognitionException {
		StatementsContext _localctx = new StatementsContext(_ctx, getState());
		enterRule(_localctx, 0, RULE_statements);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(70); 
			_errHandler.sync(this);
			_la = _input.LA(1);
			do {
				{
				setState(70);
				_errHandler.sync(this);
				switch (_input.LA(1)) {
				case T__14:
					{
					setState(34);
					loopStatement();
					setState(35);
					match(NL);
					}
					break;
				case T__0:
					{
					setState(37);
					createStatement();
					setState(38);
					match(NL);
					}
					break;
				case T__2:
					{
					setState(40);
					addStatement();
					setState(41);
					match(NL);
					}
					break;
				case T__1:
					{
					setState(43);
					deleteStatement();
					setState(44);
					match(NL);
					}
					break;
				case T__3:
					{
					setState(46);
					removeStatement();
					setState(47);
					match(NL);
					}
					break;
				case T__4:
					{
					setState(49);
					listStatement();
					setState(50);
					match(NL);
					}
					break;
				case T__6:
					{
					setState(52);
					getStatement();
					setState(53);
					match(NL);
					}
					break;
				case T__7:
					{
					setState(55);
					setStatement();
					setState(56);
					match(NL);
					}
					break;
				case T__8:
					{
					setState(58);
					switchStatement();
					setState(59);
					match(NL);
					}
					break;
				case T__9:
					{
					setState(61);
					submitStatement();
					setState(62);
					match(NL);
					}
					break;
				case T__10:
					{
					setState(64);
					insertStatement();
					setState(65);
					match(NL);
					}
					break;
				case T__11:
					{
					setState(67);
					commentOutStatement();
					setState(68);
					match(NL);
					}
					break;
				default:
					throw new NoViableAltException(this);
				}
				}
				setState(72); 
				_errHandler.sync(this);
				_la = _input.LA(1);
			} while ( (((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << T__0) | (1L << T__1) | (1L << T__2) | (1L << T__3) | (1L << T__4) | (1L << T__6) | (1L << T__7) | (1L << T__8) | (1L << T__9) | (1L << T__10) | (1L << T__11) | (1L << T__14))) != 0) );
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class CreateStatementContext extends ParserRuleContext {
		public TerminalNode Type() { return getToken(pompp_dslParser.Type, 0); }
		public ObjectContext object() {
			return getRuleContext(ObjectContext.class,0);
		}
		public CreateStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_createStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterCreateStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitCreateStatement(this);
		}
	}

	public final CreateStatementContext createStatement() throws RecognitionException {
		CreateStatementContext _localctx = new CreateStatementContext(_ctx, getState());
		enterRule(_localctx, 2, RULE_createStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(74);
			match(T__0);
			setState(75);
			match(Type);
			setState(76);
			object();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class DeleteStatementContext extends ParserRuleContext {
		public ObjectContext object() {
			return getRuleContext(ObjectContext.class,0);
		}
		public DeleteStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_deleteStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterDeleteStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitDeleteStatement(this);
		}
	}

	public final DeleteStatementContext deleteStatement() throws RecognitionException {
		DeleteStatementContext _localctx = new DeleteStatementContext(_ctx, getState());
		enterRule(_localctx, 4, RULE_deleteStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(78);
			match(T__1);
			setState(79);
			object();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class AddStatementContext extends ParserRuleContext {
		public ObjectContext object() {
			return getRuleContext(ObjectContext.class,0);
		}
		public TerminalNode Property() { return getToken(pompp_dslParser.Property, 0); }
		public TerminalNode Integer() { return getToken(pompp_dslParser.Integer, 0); }
		public TerminalNode Floating() { return getToken(pompp_dslParser.Floating, 0); }
		public TerminalNode Absolute_Path() { return getToken(pompp_dslParser.Absolute_Path, 0); }
		public TerminalNode String() { return getToken(pompp_dslParser.String, 0); }
		public TerminalNode Options() { return getToken(pompp_dslParser.Options, 0); }
		public AddStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_addStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterAddStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitAddStatement(this);
		}
	}

	public final AddStatementContext addStatement() throws RecognitionException {
		AddStatementContext _localctx = new AddStatementContext(_ctx, getState());
		enterRule(_localctx, 6, RULE_addStatement);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(81);
			match(T__2);
			setState(82);
			object();
			setState(83);
			match(Property);
			setState(84);
			_la = _input.LA(1);
			if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << Integer) | (1L << Floating) | (1L << String) | (1L << Absolute_Path) | (1L << Options))) != 0)) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class RemoveStatementContext extends ParserRuleContext {
		public ObjectContext object() {
			return getRuleContext(ObjectContext.class,0);
		}
		public TerminalNode Property() { return getToken(pompp_dslParser.Property, 0); }
		public RemoveStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_removeStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterRemoveStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitRemoveStatement(this);
		}
	}

	public final RemoveStatementContext removeStatement() throws RecognitionException {
		RemoveStatementContext _localctx = new RemoveStatementContext(_ctx, getState());
		enterRule(_localctx, 8, RULE_removeStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(86);
			match(T__3);
			setState(87);
			object();
			setState(88);
			match(Property);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ListStatementContext extends ParserRuleContext {
		public TerminalNode Type() { return getToken(pompp_dslParser.Type, 0); }
		public ListStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_listStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterListStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitListStatement(this);
		}
	}

	public final ListStatementContext listStatement() throws RecognitionException {
		ListStatementContext _localctx = new ListStatementContext(_ctx, getState());
		enterRule(_localctx, 10, RULE_listStatement);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(90);
			match(T__4);
			setState(91);
			_la = _input.LA(1);
			if ( !(_la==T__5 || _la==Type) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class GetStatementContext extends ParserRuleContext {
		public ObjectContext object() {
			return getRuleContext(ObjectContext.class,0);
		}
		public TerminalNode Property() { return getToken(pompp_dslParser.Property, 0); }
		public GetStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_getStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterGetStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitGetStatement(this);
		}
	}

	public final GetStatementContext getStatement() throws RecognitionException {
		GetStatementContext _localctx = new GetStatementContext(_ctx, getState());
		enterRule(_localctx, 12, RULE_getStatement);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(93);
			match(T__6);
			setState(94);
			object();
			setState(95);
			_la = _input.LA(1);
			if ( !(_la==T__5 || _la==Property) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class SetStatementContext extends ParserRuleContext {
		public ObjectContext object() {
			return getRuleContext(ObjectContext.class,0);
		}
		public TerminalNode Property() { return getToken(pompp_dslParser.Property, 0); }
		public TerminalNode Integer() { return getToken(pompp_dslParser.Integer, 0); }
		public TerminalNode Floating() { return getToken(pompp_dslParser.Floating, 0); }
		public TerminalNode Absolute_Path() { return getToken(pompp_dslParser.Absolute_Path, 0); }
		public TerminalNode String() { return getToken(pompp_dslParser.String, 0); }
		public TerminalNode Options() { return getToken(pompp_dslParser.Options, 0); }
		public SetStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_setStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterSetStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitSetStatement(this);
		}
	}

	public final SetStatementContext setStatement() throws RecognitionException {
		SetStatementContext _localctx = new SetStatementContext(_ctx, getState());
		enterRule(_localctx, 14, RULE_setStatement);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(97);
			match(T__7);
			setState(98);
			object();
			setState(99);
			match(Property);
			setState(100);
			_la = _input.LA(1);
			if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << Integer) | (1L << Floating) | (1L << String) | (1L << Absolute_Path) | (1L << Options))) != 0)) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class SwitchStatementContext extends ParserRuleContext {
		public ObjectContext object() {
			return getRuleContext(ObjectContext.class,0);
		}
		public SwitchStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_switchStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterSwitchStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitSwitchStatement(this);
		}
	}

	public final SwitchStatementContext switchStatement() throws RecognitionException {
		SwitchStatementContext _localctx = new SwitchStatementContext(_ctx, getState());
		enterRule(_localctx, 16, RULE_switchStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(102);
			match(T__8);
			setState(103);
			object();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class SubmitStatementContext extends ParserRuleContext {
		public ObjectContext object() {
			return getRuleContext(ObjectContext.class,0);
		}
		public SubmitStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_submitStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterSubmitStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitSubmitStatement(this);
		}
	}

	public final SubmitStatementContext submitStatement() throws RecognitionException {
		SubmitStatementContext _localctx = new SubmitStatementContext(_ctx, getState());
		enterRule(_localctx, 18, RULE_submitStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(105);
			match(T__9);
			setState(106);
			object();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class InsertStatementContext extends ParserRuleContext {
		public TerminalNode Absolute_Path() { return getToken(pompp_dslParser.Absolute_Path, 0); }
		public TerminalNode Integer() { return getToken(pompp_dslParser.Integer, 0); }
		public TerminalNode Source_Language() { return getToken(pompp_dslParser.Source_Language, 0); }
		public TerminalNode POMPP_Library_Call_App() { return getToken(pompp_dslParser.POMPP_Library_Call_App, 0); }
		public TerminalNode POMPP_Library_Call_Sec() { return getToken(pompp_dslParser.POMPP_Library_Call_Sec, 0); }
		public TerminalNode String() { return getToken(pompp_dslParser.String, 0); }
		public InsertStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_insertStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterInsertStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitInsertStatement(this);
		}
	}

	public final InsertStatementContext insertStatement() throws RecognitionException {
		InsertStatementContext _localctx = new InsertStatementContext(_ctx, getState());
		enterRule(_localctx, 20, RULE_insertStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(108);
			match(T__10);
			setState(109);
			match(Absolute_Path);
			setState(110);
			match(Integer);
			setState(111);
			match(Source_Language);
			setState(115);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case POMPP_Library_Call_App:
				{
				setState(112);
				match(POMPP_Library_Call_App);
				}
				break;
			case POMPP_Library_Call_Sec:
				{
				setState(113);
				match(POMPP_Library_Call_Sec);
				setState(114);
				match(String);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class CommentOutStatementContext extends ParserRuleContext {
		public TerminalNode Absolute_Path() { return getToken(pompp_dslParser.Absolute_Path, 0); }
		public TerminalNode Integer() { return getToken(pompp_dslParser.Integer, 0); }
		public TerminalNode Source_Language() { return getToken(pompp_dslParser.Source_Language, 0); }
		public CommentOutStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_commentOutStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterCommentOutStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitCommentOutStatement(this);
		}
	}

	public final CommentOutStatementContext commentOutStatement() throws RecognitionException {
		CommentOutStatementContext _localctx = new CommentOutStatementContext(_ctx, getState());
		enterRule(_localctx, 22, RULE_commentOutStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(117);
			match(T__11);
			setState(118);
			match(Absolute_Path);
			setState(119);
			match(Integer);
			setState(120);
			match(Source_Language);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ObjectContext extends ParserRuleContext {
		public TerminalNode String() { return getToken(pompp_dslParser.String, 0); }
		public ArrayAccessorContext arrayAccessor() {
			return getRuleContext(ArrayAccessorContext.class,0);
		}
		public ObjectContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_object; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterObject(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitObject(this);
		}
	}

	public final ObjectContext object() throws RecognitionException {
		ObjectContext _localctx = new ObjectContext(_ctx, getState());
		enterRule(_localctx, 24, RULE_object);
		try {
			setState(124);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,3,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(122);
				match(String);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(123);
				arrayAccessor();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ArrayAccessorContext extends ParserRuleContext {
		public TerminalNode String() { return getToken(pompp_dslParser.String, 0); }
		public ArrayNotationContext arrayNotation() {
			return getRuleContext(ArrayNotationContext.class,0);
		}
		public ArrayAccessorContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_arrayAccessor; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterArrayAccessor(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitArrayAccessor(this);
		}
	}

	public final ArrayAccessorContext arrayAccessor() throws RecognitionException {
		ArrayAccessorContext _localctx = new ArrayAccessorContext(_ctx, getState());
		enterRule(_localctx, 26, RULE_arrayAccessor);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(126);
			match(String);
			setState(127);
			arrayNotation();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ArrayNotationContext extends ParserRuleContext {
		public TerminalNode Integer() { return getToken(pompp_dslParser.Integer, 0); }
		public ArrayNotationContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_arrayNotation; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterArrayNotation(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitArrayNotation(this);
		}
	}

	public final ArrayNotationContext arrayNotation() throws RecognitionException {
		ArrayNotationContext _localctx = new ArrayNotationContext(_ctx, getState());
		enterRule(_localctx, 28, RULE_arrayNotation);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(129);
			match(T__12);
			setState(130);
			match(Integer);
			setState(131);
			match(T__13);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class LoopStatementContext extends ParserRuleContext {
		public LoopCounterContext loopCounter() {
			return getRuleContext(LoopCounterContext.class,0);
		}
		public StatementsContext statements() {
			return getRuleContext(StatementsContext.class,0);
		}
		public LoopStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_loopStatement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterLoopStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitLoopStatement(this);
		}
	}

	public final LoopStatementContext loopStatement() throws RecognitionException {
		LoopStatementContext _localctx = new LoopStatementContext(_ctx, getState());
		enterRule(_localctx, 30, RULE_loopStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(133);
			match(T__14);
			setState(134);
			match(T__15);
			setState(135);
			loopCounter();
			setState(136);
			match(T__16);
			setState(137);
			match(T__17);
			setState(138);
			statements();
			setState(139);
			match(T__18);
			setState(140);
			match(T__14);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class LoopCounterContext extends ParserRuleContext {
		public List<TerminalNode> Integer() { return getTokens(pompp_dslParser.Integer); }
		public TerminalNode Integer(int i) {
			return getToken(pompp_dslParser.Integer, i);
		}
		public LoopCounterContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_loopCounter; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).enterLoopCounter(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof pompp_dslListener ) ((pompp_dslListener)listener).exitLoopCounter(this);
		}
	}

	public final LoopCounterContext loopCounter() throws RecognitionException {
		LoopCounterContext _localctx = new LoopCounterContext(_ctx, getState());
		enterRule(_localctx, 32, RULE_loopCounter);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(142);
			match(T__19);
			setState(143);
			match(Integer);
			setState(144);
			match(T__20);
			setState(145);
			match(Integer);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static final String _serializedATN =
		"\3\u608b\ua72a\u8133\ub9ed\u417c\u3be7\u7786\u5964\3$\u0096\4\2\t\2\4"+
		"\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4\13\t"+
		"\13\4\f\t\f\4\r\t\r\4\16\t\16\4\17\t\17\4\20\t\20\4\21\t\21\4\22\t\22"+
		"\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3"+
		"\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2"+
		"\3\2\6\2I\n\2\r\2\16\2J\3\3\3\3\3\3\3\3\3\4\3\4\3\4\3\5\3\5\3\5\3\5\3"+
		"\5\3\6\3\6\3\6\3\6\3\7\3\7\3\7\3\b\3\b\3\b\3\b\3\t\3\t\3\t\3\t\3\t\3\n"+
		"\3\n\3\n\3\13\3\13\3\13\3\f\3\f\3\f\3\f\3\f\3\f\3\f\5\fv\n\f\3\r\3\r\3"+
		"\r\3\r\3\r\3\16\3\16\5\16\177\n\16\3\17\3\17\3\17\3\20\3\20\3\20\3\20"+
		"\3\21\3\21\3\21\3\21\3\21\3\21\3\21\3\21\3\21\3\22\3\22\3\22\3\22\3\22"+
		"\3\22\2\2\23\2\4\6\b\n\f\16\20\22\24\26\30\32\34\36 \"\2\5\3\2\35!\4\2"+
		"\b\b\30\30\4\2\b\b\31\31\2\u0092\2H\3\2\2\2\4L\3\2\2\2\6P\3\2\2\2\bS\3"+
		"\2\2\2\nX\3\2\2\2\f\\\3\2\2\2\16_\3\2\2\2\20c\3\2\2\2\22h\3\2\2\2\24k"+
		"\3\2\2\2\26n\3\2\2\2\30w\3\2\2\2\32~\3\2\2\2\34\u0080\3\2\2\2\36\u0083"+
		"\3\2\2\2 \u0087\3\2\2\2\"\u0090\3\2\2\2$%\5 \21\2%&\7\"\2\2&I\3\2\2\2"+
		"\'(\5\4\3\2()\7\"\2\2)I\3\2\2\2*+\5\b\5\2+,\7\"\2\2,I\3\2\2\2-.\5\6\4"+
		"\2./\7\"\2\2/I\3\2\2\2\60\61\5\n\6\2\61\62\7\"\2\2\62I\3\2\2\2\63\64\5"+
		"\f\7\2\64\65\7\"\2\2\65I\3\2\2\2\66\67\5\16\b\2\678\7\"\2\28I\3\2\2\2"+
		"9:\5\20\t\2:;\7\"\2\2;I\3\2\2\2<=\5\22\n\2=>\7\"\2\2>I\3\2\2\2?@\5\24"+
		"\13\2@A\7\"\2\2AI\3\2\2\2BC\5\26\f\2CD\7\"\2\2DI\3\2\2\2EF\5\30\r\2FG"+
		"\7\"\2\2GI\3\2\2\2H$\3\2\2\2H\'\3\2\2\2H*\3\2\2\2H-\3\2\2\2H\60\3\2\2"+
		"\2H\63\3\2\2\2H\66\3\2\2\2H9\3\2\2\2H<\3\2\2\2H?\3\2\2\2HB\3\2\2\2HE\3"+
		"\2\2\2IJ\3\2\2\2JH\3\2\2\2JK\3\2\2\2K\3\3\2\2\2LM\7\3\2\2MN\7\30\2\2N"+
		"O\5\32\16\2O\5\3\2\2\2PQ\7\4\2\2QR\5\32\16\2R\7\3\2\2\2ST\7\5\2\2TU\5"+
		"\32\16\2UV\7\31\2\2VW\t\2\2\2W\t\3\2\2\2XY\7\6\2\2YZ\5\32\16\2Z[\7\31"+
		"\2\2[\13\3\2\2\2\\]\7\7\2\2]^\t\3\2\2^\r\3\2\2\2_`\7\t\2\2`a\5\32\16\2"+
		"ab\t\4\2\2b\17\3\2\2\2cd\7\n\2\2de\5\32\16\2ef\7\31\2\2fg\t\2\2\2g\21"+
		"\3\2\2\2hi\7\13\2\2ij\5\32\16\2j\23\3\2\2\2kl\7\f\2\2lm\5\32\16\2m\25"+
		"\3\2\2\2no\7\r\2\2op\7 \2\2pq\7\35\2\2qu\7\32\2\2rv\7\33\2\2st\7\34\2"+
		"\2tv\7\37\2\2ur\3\2\2\2us\3\2\2\2v\27\3\2\2\2wx\7\16\2\2xy\7 \2\2yz\7"+
		"\35\2\2z{\7\32\2\2{\31\3\2\2\2|\177\7\37\2\2}\177\5\34\17\2~|\3\2\2\2"+
		"~}\3\2\2\2\177\33\3\2\2\2\u0080\u0081\7\37\2\2\u0081\u0082\5\36\20\2\u0082"+
		"\35\3\2\2\2\u0083\u0084\7\17\2\2\u0084\u0085\7\35\2\2\u0085\u0086\7\20"+
		"\2\2\u0086\37\3\2\2\2\u0087\u0088\7\21\2\2\u0088\u0089\7\22\2\2\u0089"+
		"\u008a\5\"\22\2\u008a\u008b\7\23\2\2\u008b\u008c\7\24\2\2\u008c\u008d"+
		"\5\2\2\2\u008d\u008e\7\25\2\2\u008e\u008f\7\21\2\2\u008f!\3\2\2\2\u0090"+
		"\u0091\7\26\2\2\u0091\u0092\7\35\2\2\u0092\u0093\7\27\2\2\u0093\u0094"+
		"\7\35\2\2\u0094#\3\2\2\2\6HJu~";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}
