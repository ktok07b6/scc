#ifndef __PREPROCESSOR_HPP__
#define __PREPROCESSOR_HPP__

#include "String.hpp"
#include "List.hpp"
#include "token.hpp"
#include "TokenIterator.hpp"
#include "Scanner.hpp"
#include "SCParser.hpp"
#include "ConstantExpEvaluator.hpp"
#include "ref.hpp"
#include <map>

class PreProcessor
{
public:
	PreProcessor();
	void preprocess(const char *infile, const char *outfile);
	bool isDefined(const String &ident);

private:
	bool pp();

	bool define_directive();
	bool undef_directive();
	bool include_directive();
	bool if_directive();
	bool elif_directive();
	bool ifdef_directive();
	bool ifndef_directive();
	bool else_directive();
	bool endif_directive();
	bool error_directive();
	bool warning_directive();
	bool line_directive();
	bool pragma_directive();
	bool identifier();
	AST *expression();
	bool eval(AST *ast);

	struct MacroDef {
		List<String> params;
		//MacroHandler handler;
		List<Token> tokens;
		//List<List<Token> > caller_params;
		int isParam(const String &p) {
			List<String>::iterator i = params.find(p);
			if (i != params.end()) {
				return std::distance(params.begin(), i);
			} else {
				return -1;
			}
 		}
	};
	typedef std::map<String, ref<MacroDef> > MacroMap;

	void defMacro(const String &, const ref<MacroDef> &);
	void undefMacro(const String &);
	void applyMacro(const String &ident, const ref<MacroDef> &macro);

	void parseMacroParams(List<String> &params);
	bool parseFunctionParams(List<List<Token> > &params);
	List<Token> replaceMacroParams(const List<Token> &mtokens, const List<List<Token> > &caller_pramas);
	//bool eraseThisBlockIfDisabled();
	void skipUntilElseOrEnd();

	MacroMap macros;
	String resultString;
	List<Token> tokens;
	TokenIterator tokenit;
	Scanner scanner;
	SCParser parser;
	ConstantExpEvaluator evaluator;
	List<bool> ifBlockEnables;
};

#endif //__PREPROCESSOR_HPP__
