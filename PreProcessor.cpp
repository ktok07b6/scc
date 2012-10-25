#include "PreProcessor.hpp"
#include "token.hpp"
#include <cstring>
#include "debug.h"


void
ptokens(const List<Token> &tokens)
{
	DBG("ptokens");
	List<Token>::const_iterator i = tokens.begin();
	while (i != tokens.end()) {
		DBG("%s", i->val->toString().c_str());
		++i;
	}
}

PreProcessor::PreProcessor()
	: evaluator(this)
{
}

void
PreProcessor::preprocess(const char *infile, const char *outfile)
{
	resultString.clear();
	
	if (!scanner.scan(infile, tokens)) {
		return;
	}
	tokenit.setTokens(&tokens);

	pp();

	//token to source
	bool ppskip = false;
	List<Token>::iterator it = tokens.begin();
	while (it != tokens.end()) {
		Token t = *it;
		if (ppskip) {
			if (t.id == _PP_END) {
				ppskip = false;
				resultString += "*/";
			}
			resultString += t.val->toString();
		} else {
			if (_PP_DEFINE <= t.id && t.id <= _PP_PRAGMA) {
				ppskip = true;
				//resultString += "\n";
				resultString += "/*#";
				resultString += t.val->toString();
			} else {
				resultString += t.val->toString();
			}
		}
		++it;
	}
	DBG("\n%s", resultString.c_str());

	//save pre-processed source
	FILE *ofile = fopen(outfile, "wb");
	if (ofile) {
		fwrite(resultString.c_str(), 1, resultString.size(), ofile);
		fclose(ofile);
	}
}

bool 
PreProcessor::pp()
{
	bool success = true;
	while (!tokenit.eof()) {
		const Token &t = tokenit.get(0);
		switch (t.id) {
		case _IDENT:
			identifier();
			break;
		case _PP_DEFINE:
			success = define_directive();
			break;
		case _PP_UNDEF:
			success = undef_directive();
			break;
		case _PP_INCLUDE:
			success = include_directive();
			break;
		case _PP_INCLUDE_NEXT:
			success = include_directive();
			break;
		case _PP_IF:
			success = if_directive();
			break;
		case _PP_ELIF:
			success = elif_directive();
			break;
		case _PP_IFDEF:
			success = ifdef_directive();
			break;
		case _PP_IFNDEF:
			success = ifndef_directive();
			break;
		case _PP_ELSE:
			success = else_directive();
			break;
		case _PP_ENDIF:
			success = endif_directive();
			break;
		case _PP_ERROR:
			success = error_directive();
			break;
		case _PP_WARNING:
			success = warning_directive();
			break;
		case _PP_LINE:
			success = line_directive();
			break;
		case _PP_PRAGMA:
			success = pragma_directive();
			break;
		default:
			tokenit.next();
			break;
		}
		if (!success) {
			break;
		}
	}
	if (!ifBlockEnables.empty()) {
		ERROR("expected 'endif'");
		success = false;
	}
	return success;
}

bool 
PreProcessor::define_directive()
{
	assert(tokenit.is(_PP_DEFINE));
	tokenit.next();
	int found = tokenit.skipUntil({_IDENT}, true);
	if (found != _IDENT) {
		ERROR("define directive expects identifier");
		return false;
	}

	ref<MacroDef> macroDef = new MacroDef();
	const String &ident = tokenit.val()->toString();
	tokenit.next(false);

	//function macro check	
	bool is_function = false;
	if (tokenit.eat(_LPAREN)) {
		int found = tokenit.skipUntil({_RPAREN, _PP_END}, false);
		if (found == _RPAREN) {
			is_function = true;
			parseMacroParams(macroDef->params);
		} else {
			ERROR("function macro parenthesis is not close");
			return false;
		}
	}
	tokenit.skip(_SPC);

	if (is_function) {
		while (!tokenit.eof() && !tokenit.is(_PP_END)) {
			Token &t = tokenit.get(0);
			if (t.id == _IDENT) {
				int index = macroDef->isParam(t.val->toString());
				if (index != -1) {
					t.id = _PP_MACRO_PARAM;
					char p[3] = {'@', (char)('0'+index), '\0'};
					t.val = new TokenInt(p, index);
					DBG("macro param %s %d", t.val->toString().c_str(), index);
				}
				macroDef->tokens.push_back(t);
				tokenit.next(false);
			} else if (t.id == _PP_OP_CONCAT) {
				while (!macroDef->tokens.empty()) {
					const Token &tt = macroDef->tokens.back();
					if (tt.id == _SPC) {
						macroDef->tokens.pop_back();
					} else {
						break;
					}
				}
				tokenit.next();//skip "##" & spaces
				continue;
			} else if (t.id == _PP_OP_TO_STR) {
				tokenit.next();//skip "#" & spaces
				Token &tt = tokenit.get(0);
				int index = macroDef->isParam(tt.val->toString());
				if (tt.id != _IDENT || index == -1) {
					ERROR("'#' is not followed by a macro parameter");
					return false;
				}
				//For stringized param, '"' are inserted in the current position front and the back. 
				Token quote;
				quote.id = _SOURCE_STRING;
				quote.val = new TokenString("\"");
				tokenit.insert(quote, 0);
				tokenit.insert(quote, 1);
				//first '"' is added to macroDef->tokens at next loop
				tokenit.prev();
				continue;
			} else {
				macroDef->tokens.push_back(t);
				tokenit.next(false);
			}
		}
	} else {
		while (!tokenit.eof() && !tokenit.is(_PP_END)) {
			const Token &t = tokenit.get(0);
			macroDef->tokens.push_back(t);
			tokenit.next(false);
		}
	}

	defMacro(ident, macroDef);

	if (!tokenit.is(_PP_END)) {
		//TODO: warning
	}
	tokenit.next();
	return true;
}

bool 
PreProcessor::undef_directive()
{
	assert(tokenit.is(_PP_UNDEF));
	tokenit.next();

	int found = tokenit.skipUntil({_IDENT}, true);
	if (found != _IDENT) {
		ERROR("define directive expects identifier");
		return false;
	}

	const String &ident = tokenit.val()->toString();
	tokenit.next();
	
	undefMacro(ident);

	if (!tokenit.is(_PP_END)) {
		//TODO: warning
		found = tokenit.skipUntil({_PP_END}, true);
	}
	tokenit.next();
	return true;
}

bool 
PreProcessor::include_directive()
{
	assert(tokenit.is(_PP_INCLUDE));
	tokenit.next();

	if (!tokenit.is(_PP_HEADER_NAME)) {
		ERROR("'include' is not followed by header-name");
		return false;
	}

	const char *headername = tokenit.val()->toString().c_str();
	DBG("try open %s", headername);
	FILE *f = fopen(headername, "r");
	if (!f) {
		//TODO:retry another include directory
		ERROR("%s is not found", headername);
		return false;
	}
	fclose(f);

	int found = tokenit.skipUntil({_PP_END}, true);
	if (found != _PP_END) {
		ERROR("??");
		return false;
	}
	tokenit.next();
	List<Token> includedTokens;
	if (scanner.scan(headername, includedTokens)) {
		tokenit.insert(includedTokens);
	} else {
		ERROR("???");
	}

	return true;
}

bool 
PreProcessor::if_directive()
{
	assert(tokenit.is(_PP_IF));
	tokenit.next();

	AST *ast = expression();
	if (!ast) {
		return false;
	}

	if (eval(ast)) {
		ifBlockEnables.push_back(true);
	} else {
		ifBlockEnables.push_back(false);
		TokenIterator tag = tokenit;
		skipUntilElseOrEnd();
		tag.erase(tokenit-tag);
	}
	return true;
}

bool 
PreProcessor::elif_directive()
{
	assert(tokenit.is(_PP_ELIF));
	tokenit.next();

	if (ifBlockEnables.empty()) {
		ERROR("'if' directive is not found");
		return false;
	}

	bool if_is_enable = ifBlockEnables.back();
	if (if_is_enable) {
		TokenIterator tag = tokenit;
		skipUntilElseOrEnd();
		tag.erase(tokenit-tag);
	} else {
		AST *ast = expression();
		if (!ast) {
			return false;
		}
		if (eval(ast)) {
			ifBlockEnables.pop_back();
			ifBlockEnables.push_back(true);
		} else {
			TokenIterator tag = tokenit;
			skipUntilElseOrEnd();
			tag.erase(tokenit-tag);
		}
	}
	return true;
}

bool 
PreProcessor::ifdef_directive()
{
	assert(tokenit.is(_PP_IFDEF));
	tokenit.next();
	
	if (!tokenit.is(_IDENT)){
		ERROR("'ifdef' is not followed by a identifier");
		return false;
	}
	bool defined = isDefined(tokenit.val()->toString());
	tokenit.next();

	ifBlockEnables.push_back(defined);
	return true;
}

bool 
PreProcessor::ifndef_directive()
{
	assert(tokenit.is(_PP_IFNDEF));
	tokenit.next();

	if (!tokenit.is(_IDENT)){
		ERROR("'ifdef' is not followed by a identifier");
		return false;
	}
	bool defined = isDefined(tokenit.val()->toString());
	tokenit.next();

	ifBlockEnables.push_back(!defined);
	return true;
}
bool 
PreProcessor::else_directive()
{
	assert(tokenit.is(_PP_ELSE));
	tokenit.next();
	if (!tokenit.is(_PP_END)) {
		//TODO: warning
	} 
	tokenit.skipUntil({_PP_END}, true);
	tokenit.next();

	if (ifBlockEnables.empty()) {
		ERROR("'if' directive is not found (before 'else')");
		return false;
	}
	bool if_is_enable = ifBlockEnables.back();
	if (if_is_enable) {
		TokenIterator tag = tokenit;
		skipUntilElseOrEnd();
		tag.erase(tokenit-tag);
	} else {
		ifBlockEnables.pop_back();
		ifBlockEnables.push_back(true);
	}

	return true;
}

bool 
PreProcessor::endif_directive()
{
	assert(tokenit.is(_PP_ENDIF));
	tokenit.next();

	if (ifBlockEnables.empty()) {
		ERROR("'if' directive is not found (before 'endif')");
		return false;
	}
	ifBlockEnables.pop_back();
	return true;
}

bool 
PreProcessor::error_directive()
{
	assert(tokenit.is(_PP_ERROR));
	tokenit.next();
	//TODO: error
	ERROR("#error");
	return false;
}

bool 
PreProcessor::warning_directive()
{
	assert(tokenit.is(_PP_WARNING));
	tokenit.next();
	//TODO: warning
	return true;
}

bool 
PreProcessor::line_directive()
{	
	assert(tokenit.is(_PP_LINE));
	tokenit.next();
	//TODO: line
	return true;
}

bool 
PreProcessor::pragma_directive()
{
	assert(tokenit.is(_PP_PRAGMA));
	tokenit.next();
	//TODO: pragma
	return true;
}

bool 
PreProcessor::identifier()
{
	assert(tokenit.is(_IDENT));
	bool replaced = false;
	const String &ident = tokenit.val()->toString();
	MacroMap::iterator it = macros.find(ident);
	if (it == macros.end()) {
		tokenit.next();
	} else {
		applyMacro(ident, it->second);
		replaced = true;
	}
	return replaced;
}

AST *
PreProcessor::expression()
{
	List<Token> exps;
	while (!tokenit.eof() && !tokenit.is(_PP_END)) {
		const Token &t = tokenit.get(0);
		switch (t.id) {
		case _IDENT: {
			bool replaced = identifier();
			if (!replaced) {
				exps.push_back(t);
			}
			continue;
		}
			//'defined' expression is pre-evaluated here
		case _PP_DEFINED: {
			tokenit.next();
			bool lparen = tokenit.eat(_LPAREN);
			if (!tokenit.is(_IDENT)){
				ERROR("'defined' error");
			}
			bool defined = isDefined(tokenit.val()->toString());
			tokenit.next();
			if (lparen && !tokenit.eat(_RPAREN)) {
				ERROR("'defined' error");
				return false;
			}
			Token newt;
			newt.id = _DECIMAL_CONSTANT;
			if (defined) {
				newt.val = new TokenInt("<defined 1>", 1);
			} else {
				newt.val = new TokenInt("<defined 0>", 0);
			}
			exps.push_back(newt);
			continue;
		}
		case _SPC:
			//don't push spaces
			break;
		default:
			exps.push_back(t);
		}
		tokenit.next();
	}
	DBG("expression %d", exps.size());
	ptokens(exps);
	DBG("---------");
	if (tokenit.is(_PP_END)) {
		tokenit.next();
	}
	
	return parser.parseConstantExpression(exps);
}

bool
PreProcessor::eval(AST *ast)
{
	DBG("\n\n%s", ast->toString().c_str());
	return ast->accept(&evaluator);
}

bool 
PreProcessor::isDefined(const String &ident)
{
	return (macros.find(ident) != macros.end());
}

void 
PreProcessor::defMacro(const String &ident, const ref<MacroDef> &def)
{
	macros.insert(std::make_pair(ident, def));
}

void 
PreProcessor::undefMacro(const String &ident)
{
	macros.erase(ident);
}

void
PreProcessor::applyMacro(const String &ident, const ref<MacroDef> &macro)
{
	assert(tokenit.is(_IDENT));
	
	DBG("apply macro: %s", ident.c_str());
	if (macro->params.empty()) {
		tokenit.erase(1);//erase identifier
		tokenit.insert(macro->tokens);
	} else {
		TokenIterator tag = tokenit;
		tokenit.next();
		//get caller params
		if (tokenit.is(_LPAREN)) {
			tokenit.next();
			List<List<Token> > caller_params;
			if (!parseFunctionParams(caller_params)) {
				ERROR("failed to parse function macro");
				return;
			}
			if (macro->params.size() != caller_params.size()) {
				ERROR("macro %s requres %d parameters", 
					  ident.c_str(), macro->params.size());
				return;
			}
			tag.erase(tokenit-tag);//erase function call tokens

			//replace macro params
			List<Token> replace_tokens = replaceMacroParams(macro->tokens, caller_params);
			tokenit.insert(replace_tokens);
			//tokenit.next();
		}
	}
}

void 
PreProcessor::parseMacroParams(List<String> &params)
{
	if (tokenit.eat(_RPAREN)) {
		//no parameter
		return;
	}
	while (true) {
		const Token &t = tokenit.get(0);
		if (tokenit.eat(_IDENT)) {
			params.push_back(t.val->toString());
			if (tokenit.eat(_COMMA)) {
				continue;
			} else if (tokenit.eat(_RPAREN)) {
				break;
			} else {
				ERROR("failed to parse macro parameters %s", 
					  tokenit.val()->toString().c_str());
				break;
			}
		} else {
			ERROR("macro parameter is not identifier");
			return;
		}
	}
}

bool 
PreProcessor::parseFunctionParams(List<List<Token> > &params)
{
	List<Token> param;
	bool finish = false;
	while (!finish) {
		if (tokenit.eof()) {
			return false;
		}
		const Token &t = tokenit.get(0);
		if (tokenit.is(_COMMA)) {
			params.push_back(param);
			param.clear();
		} else if (tokenit.is(_RPAREN)) {
			params.push_back(param);
			param.clear();
			finish = true;
		} else if (tokenit.is(_IDENT)) {
			bool replaced = identifier();
			if (!replaced) {
				param.push_back(t);
				//we can continue without tokenit.next(),
				//because it has already been called by identifier()
			}
			continue;
		} else {
			param.push_back(t);
		}
		tokenit.next(false);
	}

	return true;
}

List<Token> 
PreProcessor::replaceMacroParams(const List<Token> &mtokens, const List<List<Token>> &caller_params)
{
	//FIXME: nested macro
	//i.e. FUNC(x, FUNC(x,y))

	List<Token> replace = mtokens;
	TokenIterator it;
	it.setTokens(&replace);
	while (!it.eof()) {
		if (it.is(_PP_MACRO_PARAM)) {
			int index = it.val()->toInt();
			const List<Token> &param = caller_params.at(index);
			it.erase(1);
			it.insert(param);
		}
		it.next();
	}
	return replace;
}

void 
PreProcessor::skipUntilElseOrEnd()
{
	int level = 0;
	while (!tokenit.eof()) {
		int found = tokenit.skipUntil({_PP_IF, _PP_ELIF, _PP_IFDEF, _PP_IFNDEF, _PP_ELSE, _PP_ENDIF}, true);
		switch (found) {
		case _PP_IF:
		case _PP_IFDEF:
		case _PP_IFNDEF:
			++level;
			tokenit.next();
			break;
		case _PP_ELIF:
		case _PP_ELSE:
			if (level == 0) {
				return;
			} else {
				tokenit.next();
			}
			break;
		case _PP_ENDIF:
			if (level == 0) {
				return;
			} else {
				--level;
				tokenit.next();
			}
			break;
		}
	}
}
