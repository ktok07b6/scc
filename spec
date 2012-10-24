A.1 Keywords
typedef-name ::=
	identifier
namespace-name ::=
	original-namespace-name
	| namespace-alias
original-namespace-name ::=
	identifier
namespace-alias ::=
	identifier
class-name ::=
	identifier
	| template-id
enum-name ::=
	identifier
template-name ::=
	identifier

A.2 Lexical conventions
Synthesis tools are able to reject a character literal and a universal character name. A string literal is used only for the name of submodule.

hex-quad ::=
	hexadecimal-digit hexadecimal-digit hexadecimal-digit hexadecimal-digit
//universal-character-name ::=
//	\u hex-quad
//	\U hex-quad hex-quad

preprocessing-token ::=
	header-name
	| identifier
	| pp-number
	| character-literal
	| string-literal
	| preprocessing-op-or-punc
	(each non-white-space character that cannot be one of the above)

token ::=
	identifier
	| keyword
	| literal
	| operator
	| punctuator

header-name ::=
	< h-char-sequence >
	| "q-char-sequence"

h-char-sequence ::=
	h-char
	| h-char-sequence h-char
	
h-char ::=
	any member of the source character set except new-line and >

q-char-sequence ::=
	q-char
	| q-char-sequence q-char

q-char ::=
	any member of the source character set except new-line 

pp-number ::=
	digit
	| . digit
	| pp-number digit
	| pp-number nondigit
	| pp-number e sign
	| pp-number E sign
	| pp-number .

identifier ::=
	nondigit
	| identifier nondigit
	| identifier digit

nondigit ::= one of	universal_character_name
	_ a b c d e f g h i j k l m
	n o p q r s t u v w x y z
	A B C D E F G H I J K L M
	N O P Q R S T U V W X Y Z

digit ::= one of
	0 1 2 3 4 5 6 7 8 9

preprocessing-op-or-punc ::= one of
     { } [ ] # ## ( )
	<: :> <% %> %: %:%: ; : ...
	new delete ? :: . .*
	+ - * / % ^ & | ~
	! = < > += -= *= /= %=
	^= &= |= << >> >>= <<= == !=
	<= >= && || ++ -- , ->* ->
	and and_eq bitand bitor compl not not_eq
	or or_eq xor xor_eq

literal ::=
	integer-literal
	| character-literal
	| floating-literal
	| string-literal
	| boolean-literal

integer-literal ::=
	decimal-literal [ integer-suffix ]
	| octal-literal [ integer-suffix ]
	| hexadecimal-literal [ integer-suffix ]
decimal-literal ::=
	nonzero-digit
	| decimal-literal digit

octal-literal ::=
	0
	| octal-literal octal-digit
hexadecimal-literal ::=
	0x hexadecimal-digit
	| 0X hexadecimal-digit
	| hexadecimal-literal hexadecimal-digit
nonzero-digit ::= one of
	1 2 3 4 5 6 7 8 9
octal-digit ::= one of	
	1 2 3 4 5 6 7
hexadecimal-digit ::= one of
	1 2 3 4 5 6 7 8 9
	a b c d e f
	A B C D E F
integer-suffix ::=
	unsigned-suffix [ long-suffix ]
	| long-suffix [ unsigned-suffix ]
unsigned-suffix ::= one of
	u U
long-suffix ::= one of
	l L

floating-literal ::=
	fractional-constant [ exponent-part ] [ floating-suffix ]
	| digit-sequence exponent-part [ floating-suffix ]
fractional-constant ::=
	[ digit-sequence ] . digit-sequence
	| digit-sequence .
exponent-part ::=
	e [ sign ] digit-sequence
	| E [ sign ] digit-sequence
sign ::= one of
	+ -
digit-sequence ::=
	digit
	| digit-sequence digit
floating-suffix ::= one of
	f l F L
string-literal ::=
	"[s-char-sequence]"
	| L"[s-char-sequence]"

s-char-sequence ::=
	s-char
	| s-char_sequence s-char

s-char ::=
	   any member of the source character set except the double_quote ",
	   bachslash \, or new_line character

boolean-literal ::=
	false
	| true


A.3 Basic concepts
translation-unit ::=
	[ declaration-seq ] [ sc-main-definition ]

A.4 Expressions
primary-expression ::=
	literal
	| this
	| ( expression )
	| id-expression
id-expression ::=
	unqualified-id
	| qualified-id
unqualified-id ::=
	identifier
	| operator-function-id
	| conversion-function-id
	//| ~class-name
	| template-id
qualified-id ::=
	[ :: ] nested-namespace-specifier [ template ] unqualified-id
	| :: identifier
	| :: operator-function-id
	| :: template-id
nested-name-specifier ::=
	 class-or-namespace-name :: [ nested-name-specifier ]
	 | class-or-namespace-name :: template nested-name-specifier
class-or-namespace-name ::=
	class-name
	| name-space-name
postfix-expression ::=
	primary-expression
	| postfix-expression [ expression ]
	| postfix-expression ( [ expression-list ] )
	| simple-type-specifier ( [ expression-list ] )
	| typename [ :: ] nested-name-specifier identifier ( [ expression-list ] )
	| typename [ :: ] nested-name-specifier [template] template-id ( [ expression-list ] )
	| postfix-expression . [ template ] id-expression
	| postfix-expression -> [ template ] id-expression
	//| postfix-expression . pseudo-destructor-name
	//| postfix-expression -> pseudo-destructor-name
	| postfix-expression ++
	| postfix-expression --
	| dynamic-cast < type-id > ( expression )
	| static-cast < type-id > ( expression )
	| reinterpret-cast < type-id > ( expression )
	| const-cast < type-id > ( expression )
	//| typeid ( expression )
	//| typeid ( type-id )

expression-list ::=
	assignment-expression
	| expression-list , assignment-expression
unary-expression ::=
	postfix-expression
	| ++ cast-expression
	| -- cast-expression
	| unary-operator cast-expression
	//| sizeof unary-expression
	//| sizeof ( type-id )
	//| new-expression
	//| delete-expression
unary-operator ::= one of
	* & + - ! ~
cast-expression ::=
	unary-expression
	| ( type_id ) cast-expression
pm-expression ::=
	cast-expression
	//| pm-expression .* cast-expression
	//| pm-expression ->* cast-expression

multiplicative-expression ::=
	pm-expression
	| multiplicative-expression * pm-expression
	| multiplicative-expression / pm-expression
	| multiplicative-expression % pm-expression

additive-expression ::=
	multiplicative-expression
	| additive-expression + multiplicative-expression
	| additive-expression . multiplicative-expression
shift-expression ::=
	additive-expression
	| shift-expression << additive-expression
	| shift-expression >> additive-expression
relational-expression ::=
	shift-expression
	| relational-expression < shift-expression
	| relational-expression > shift-expression
	| relational-expression <= shift-expression
	| relational-expression >= shift-expression
equality-expression ::=
	relational-expression ::=
	| equality-expression == relational-expression
	| equality-expression != relational-expression
and-expression ::=
	equality-expression
	| and-expression & equality-expression
exclusive-or-expression ::=
	and-expression
	| exclusive-or-expression ^ and-expression
inclusive-or-expression ::=
	exclusive-or-expression
	| inclusive-or-expression | exclusive-or-expression
logical-and-expression ::=
	inclusive-or-expression
	| logical-and-expression && inclusive-or-expression
logical-or-expression ::=
	logical-and-expression
	| logical-or-expression || logical-and-expression
conditional-expression ::=
	logical-or-expression
	| logical-or-expression ? expression : assignment-expression
assignment-expression ::=
	conditional-expression
	| logical-or-expression assignment-operator assignment-expression
	//| throw-expression
assignment-operator ::= one of
    = *= /= %= += -= >>= <<= &= ^= |=
expression ::=
    assignment-expression
	| expression , assignment-expression
constant-expression ::=
	conditional-expression

A.5 Statements
statement ::=
	labeled-statement
	| expression-statement
	| compound-statement
	| wait-statement
	| signal-assignment-statement
	| selection-statement
	| iteration-statement
	| jump-statement
	| declaration-statement
	//| try-block
labeled-statement ::=
	identifier : statement
	| case constant-expression : statement
	| default : statement
expression_statement ::=
	[ expression ] ;
compound-statement ::=
	{ [ statement_seq ] }
statement-seq ::=
	statement
	| statement-seq statement
wait-statement ::=
	wait ( ) ;
	| wait ( constant-expression ) ;
signal-assignment-statement ::=
	signal-or-port-identifier . write ( expression ) ;
	| signal-or-port-identifier = expression ;
selection_statement ::=
	if ( condition ) statement
	| if ( condition ) statement else statement
	| switch ( condition ) statement
condition ::=
	expression
	| type_specifier_seq declarator = assignment_expression
iteration-statement ::=
	while ( condition ) statement
	| do statement while ( expression ) ;
	| for ( for-init-statement [ condition ] ; [ expression ] ) statement
for-init-statement ::=
	expression-statement
	| simple-declaration
jump-statement ::=
	break ;
	| continue ;
	| return [ expression] ;
	| goto label-name ;
declaration-statement ::=
	block-declaration

A.6 Declarations
declaration-seq ::=
	declaration
	| declaration-seq declaration
declaration ::=
	block-declaration
	| function-definition
	| template-declaration
	| explicit-instantiation
	| explicit-specialization
	//| linkage-specification
	| namespace-definition
	| sc-process-definition
block-declaration ::=
	simple-declaration
	//| asm-definition
	| namespace-alias-definition
	| using-declaration
	| using-directive
simple-declaration ::=
	[ decl-specifier-seq ] [ init-declarator-list ] ;
decl-specifier ::=
	storage-class-specifier
	| type-specifier
	| function-specifier
	| friend
	| typedef
decl-specifier-seq ::=
	[ decl-specifier-seq ] decl-specifier
storage-class-specifier ::=
	auto
	| register
	| static
	| extern
	| mutable
function-specifier ::=
	inline
	| virtual
	| explicit
typedef-name ::=
	identifier
type-specifier ::=
	simple-type-specifier
	| class-specifier
	| enum-specifier
	| elaborated-type-specifier
	| cv-qualifier
	| sc-type-specifier
	| sc-module-specifier
simple-type-specifier ::=
	[ :: ] [ nested-name-specifier ] type-name
	| [ :: ] nested-name-specifier template template-id
	| char
	//| wchar_t
	| bool
	| short
	| int
	| long
	| signed
	| unsigned
	//| float
	//| double
	| void
type-name ::=
	class-name
	| enum-name
	| typedef-name
elaborated-type-specifier ::=
	class_key [ :: ] [ nested_name_specifier ] identifier
	| enum [ :: ] [ nested_name_specifier ] identifier
	| typename [ :: ] nested_name_specifier identifier
	| typename [ :: ] nested_name_specifier [ template ] template_id
enum-name ::=
	identifier
enum-specifier ::=
	enum [ identifier ] { [ enumerator-list ] }
enumerator-ist ::=
	enumerator-difinition
	| enumerator-list , enumerator-difinition
enumerator-difinition ::=
	enumerator
	| enumerator = constant-expression
enumerator ::=
	identifier
namespace-name ::=
	original-namespace-name
	| namespace-alias
original-namespace-name ::=
	identifier
namespace-definition ::=
	named-namespace-definition
	| unnamed-namespace-definition
named-namespace-definition ::=
	original-namespace-definition
	| extension-namespace-definition
original-namespace-definition ::=
	namespace identifier [ namespace-body ]
extension-namespace-definition ::=
	namespace original-namespace-name [ namespace-body ]
unnamed-namespace-definition ::=
	namespace [ namespace-body ]
namespace-body ::=
	[ declaration-seq ]
namespace-alias ::=
	identifier
namespace-alias-definition ::=
	namespace identifier = qualified-namespace-specifier ;
qualified-namespace-specifier ::=
	[ :: ] [ nested-name-specifier ] namespace-name
using-declaration ::=
	using [ typename ] [ :: ] nested-name-specifier unqualified-id ;
	| using :: unqualified-id ;
using-directive ::=
	using namespace [ :: ] [ nested-name-specifier ] namespace-name ;

A.6-1 SystemC Type Specifiers
sc-type-specifier ::=
	sc_int < constant-expression >
	| sc_uint < constant-expression >
	| sc_bigint < constant_expression >
	| sc_biguint < constant_expression >
	| sc_logic
	| sc_lv < constant_expression >
	| sc_bit
	| sc_bv < constant_expression >
	| sc_fixed < constant_expression , constant_expression
	[ , sc-quantization-mode-specifier ] [ , sc-overflow-mode-specifier ]
	[ , constant-expression ] >
	| sc_ufixed < constant_expression , constant_expression
	[ , sc-quantization-mode-specifier ] [ , sc-overflow-mode-specifier ] [ , constant-expression ] >
sc-quantization-mode-specifier ::=
	SC_RND
	| SC_RND_ZERO
	| SC_RND_MIN_INF
	| SC_RND_INF
	| SC_RND_CONV
	| SC_TRN
	| SC_TRN_ZERO
sc-overflow-mode-specifier ::=
	SC_SAT
	| SC_SAT_ZERO
	| SC_SAT_SYN
	| SC_WRAP
	| SC_WRAP_SM

A.7 Declarators
init-declarator-list ::=
	init-declarator
	| init-declarator-list , init-declarator
init-declarator ::=
	declarator [ initializer ]
declarator ::=
	direct-declarator
	| ptr-operator declarator
direct-declarator ::=
	declarator-id
	| direct-declarator ( parameter-declaration-clause ) [ cv-qualifier-seq ]
	//[ exception-specification ]
	| direct-declaration [ [ constant-expression ] ]
	| ( declarator )
ptr-operator ::=
	//* [ cv-qualifier-seq ]
	| &
	//| [ :: ] [ nested-name-specifier ] * [ cv-qualifier-seq ]
cv-qualifier-seq ::=
	cv-qualifier [ cv-qualifier-seq ]
cv-qualifier ::=
	const
	| volatile
declarator-id ::=
	id-expression
	| [ :: ] [ nested-name-specifier ] type-name
type-id ::=
	type-specifier-seq [ abstract-declarator ]
type-specifier-seq ::=
	type-specifier [type-specifier-seq ]
abstract-declarator ::=
	ptr_operator [ abstract-declarator ]
	| direct-abstract-declarator
direct-abstract-declarator ::=
	[ direct-abstract-declarator ] ( parameter-declaration-clause ) [ cv-qualifier-seq ]
	//[exception-specification ]
	| [direct-abstract-declarator ] [ [ constant-expression ] ]
	| ( abstract-declarator )
parameter-declaration-clause ::=
	[ parameter-declaration-list ] [ ... ]
	| parameter-declaration-list , ...
parameter-declaration-list ::=
	parameter-declaration
	| parameter-declaration-list , parameter-declaration
parameter-declaration ::=
	decl-specifier-seq declarator
	| decl-specifier-seq declarator = assignment-expression
	| decl-specifier-seq [ abstract-declarator ]
	| decl-specifier-seq [ abstract-declarator ] = assignment-expression
	| [ const ] sc-signal-declaration & identifier
function-definition ::=
	[ decl-specifier-seq ] declarator [ ctor-initializer ] function_body
	//| [decl-specifier-seq ] declarator function-try-block
function-body ::=
	compound-statement
initializer ::=
	= initializer-clause
	| ( expression-list )
initializer-clause ::=
	assignment-expression
	| { initializer-list [ , ] }
	| { }
initializer-list :=
	initializer-clause
	| initializer-list , iitializer-clause

A.8 Classes
Classes are regarded as a module or a user defined type in SystemC synthesis. The syntax for module is described in A.8-1 section. There is many limitation for a user defined type.

class-name ::=
	identifier
	| template-id
class-specifier ::=
	class-head { [ member-specification ] }
	-
class-head ::=
	class-key [ identifier ] [ base_clause ]
	| class-key nested-name-specifier identifier [ base-clause ]
	| class-key [ nested-name-specifier ] template-id [ base-clause ]
class-key ::=
	class
	| struct
	//| union
member-specification ::=
	member-declaration [ member-specification ]
	| access-specifier : [ member-specification ]
member-declaration ::=
	[ decl-specifier-seq ] [ member-declarator-list ] ;
	| function-definition [ ; ]
	| [ :: ] nested-name-specifier [ template ] unqualified-id ;
	| using-declaration
	| template-declaration
member-declarator-list ::=
	member-declarator
	| member-declarator-list [ , ] member-declarator
member-declarator ::=
	declarator [ pure-specifier ]
	| declarator [ constant-initializer ]
	| [ identifier ] : constant-expression
pure-specifier ::=
	= 0
constant-initializer ::=
	= constant-expression

A.8-1 Module Declaration
sc-module-specifier ::=
	sc-module-head { [ module-member-specification ] }
sc-module-head ::=
	SC_MODULE( identifier )
	| class-key [ nested-name-specifier ] identifier : [ public ] sc_module

sc-module-member-specification ::=
	sc-module-member-declaration [ sc-module-member-specification ]
	| access-specifier : [ sc-module-member-specification ]
sc-module-member-declaration ::=
	member-declaration
	| sc-signal-dclaration
	| sc-sub-module-declaration
	| sc-module-constructor-definition
	| sc-module-constructor-declaration
	| sc-has-process-declaration
sc-signal-declaration ::=
	sc-signal-key < type-specifier > signal-declarator-list ;
	| sc-resolved-key signal -declarator-list ;
	| sc-resolved-vector-key < constant-expression > signal -declarator-list ;
signal-declarator-list ::=
	identifier
	| signal-declarator-list , identifier
	| sc_in_clk
	| sc_out_clk
	| sc_inout_clk
sc-resolved-key ::=
	sc_signal_resolved
	| sc_in_resolved
	| sc_out_resolved
	| sc_inout_resolved
sc-resolved-vector-key ::=
	sc_signal_rv
	| sc_in_rv
	| sc_out_rv
	| sc_inout_rv
sc-sub-module-declaration ::=
	id-expression [ * ] identifier ;
sc-module-constractor-declaration ::=
	SC_CTOR( identifier ) ;
	| identifier ( sc_module_name [ identifier ] [ , parameter-declaration-list ] ) ;
sc-module-constructor-definition ::=
	SC_CTOR( identifier ) [ ctor-initializer ] sc-module-constructor-body
	| identifier ( sc_module_name identifier [ , parameter-declaration-list ] ) : sc_module ( identifier ) [ , mem-initializer-list ] sc-module-constructor-body
sc-module-constractor-body ::=
	{ [ sc-module-constractor-element-seq ] }
sc-module-constractor-element-seq ::=
	sc-module-constractor-element
	| sc-module-constractor-element-seq sc-module-constractor-element
sc-module-constractor-element ::=
	sc-module-instantiation-statement
	| sc-port-binding-statement
	| sc-process-statement
sc-module-instantiation-statement ::=
	identifier = new [ :: ] [ nested-name-specifier ] class-name ( string_literal ) ;
sc-port-binding-statement ::=
	sc-named-port-binding-statement ;
	| sc-positional-port-binding-statement ;
sc-named-port-binding-statement ::=
	identifier -> id-expression ( id-expression ) ;
	| identifier . id-expression ( id-expression ) ;
sc-positional-port-binding ::=
	[ * ] identifier ( identifier-list )
identifier-list ::=
	id-expression
	| identifier-list id-expression
sc-process-statement ::=
	SC_METHOD ( identifier ) ; sensitivity-list| SC_CTHREAD ( identifier , sc-event ) ; [ sc-watching-statement ]
sc-process-definition ::=
	void sc-process-id ( ) sc-process-body
sc-process-id ::=
	identifier
	| template-id
	| [ :: ] nested-name-specifier [ template ] identifier
	| [ :: ] nested-name-specifier [ template ] template-id
sc-process-body ::=
	sc-method-body
	| sc-cthread-body
sc-method-body ::=
	compound-statement
sc-sensitivity-list ::=
	sc-sensitivity-clause
	| sc-sensitivity-list sc-sensitivity-clause
sc-sensitivity-clause ::=
	sensitive ( sc-event ) ;
	| sensitive_pos ( identifier ) ;
	| sensitive_neg ( identifier ) ;
	| sensitive sc-event-stream ;
	| sensitive_pos sc-event-stream ;
	| sensitive_neg sc-event-stream ;
sc-event-stream ::=
	<< sc-event
	| sc-event-stream << sc-event
sc-identifier-stream ::=
	<< identifier
	| sc-identifier-stream << identifier
sc-event ::=
	identifier
	| identifier . pos ( )
	| identifier . neg ( )
sc-watching-satement ::=
	watching ( identifier . delayed ( ) == sc-watching-condition ) ;
sc-watching-condition ::=
	boolean-literal
	| logic-literal
sc-cthread-body ::=
	compound-statement wait ( ) ; while ( true ) { compound-statement }
sc-has-process-declaration ::=
	SC_HAS_PROCESS( identifier ) ;

A.9 Derived classes
base-clause ::=
	: base-specifier-list
base-specifier-list ::=
	base-specifier
	| base-specifier-list , base-specifier
base-specifier ::=
	[ :: ] [ nested-name-specifier ] class-name
	| virtual [ access-specifier ] [ :: ] [ nested-name-specifier ] class-name
	| access-specifier [ virtual ] [ :: ] [ nested-name-specifier ] class-name
access-specifier ::=
	private
	| protected
	| public

A.10 Special member functions
conversion-function-id ::=
	operator conversion-type-id
converion-type-id ::=
	type-specifier-seq [ conversion-declarator ]
conversion-declarator ::=
	ptr-operator [ conversion-declarator ]
ctor-initializer ::=
	: mem-initializer-list
mem-initializer-list ::=
	mem-initializer
	| mem-initializer , mem-initializer-list
mem-initializer ::=
	mem-initializer-id ( [ expression-list ] )
mem-initializer-id ::=
	[ :: ] [ nested-name-specifier ] class-name
	| identifier

A.11 Overloading
operator_function_id ::=
	operator operator
operator ::= one of
	//new delete new[] delete[]
	+ - * / % ^ & | ~
	! = < > += -= *= /= %=
	^= &= |= << >> >>= <<= == !=
	<= >= && || ++ -- , ->* ->
	() []


A.12 Templates

template-declaration ::=
	[ export ] template < template-parameter-list > declaration
template-parameter-list ::=
	template-parameter
	| template-parameter-list , template-parameter
template-parameter ::=
	type-parameter
	| parameter-declaration
type-parameter ::=
	class [ identifier ]
	| class [ identifier ] = type-id
	| typename [ identifier ]
	| typename [ identifier ] = type-id
	| template < template-parameter-list > class [identifier ]
	| template < template-parameter-list > class [identifier ] = id-expression
template-id ::=
	template-name < [ template-argument-list ] >
template-name ::=
	identifier
template-argument-list ::=
	template-argument
	| template-argument-list , template-argument
template-argument ::=
	assignment-expression
	| type-id
	| id-expression
explicit-instantiation ::=
	template declaration
explicit-specification ::=
	template < > declaration

A.13 Exception handling

A.14 Preprocessing directivesb
All preprocessing directives of C++ are acceptable for synthesis coding.

preprocessing-file ::=
	[ group ]
group ::=
	group-part
	| group group-part
group-part ::=
	[ pp-token ] new-line
	| if-section
	| control-line
if-section ::=
	if-group [ elif-groups ] [ else-group ] endif-line
if-group ::=
	# if constant-expression new-line [ group ]
	| # ifdef identifier new-line [ group ]
	| # ifndef identifier new-line [ group ]
elif-groups ::=
	elif-group
	| elif-groups elif-group
elif-group ::=
	# elif constant-expression new-line [ group ]
else-group ::=
	# else new-line [ group ]
endif-line ::=
	# endif new-line
control-line ::=
	# include pp-tokens new-line
	| # define identifier replacement-list new-line
	| # define identifier lparen [ identifier-list ] replacement-list new-line
	| # undef identifier new-line
	| # line pp-tokens new-line
	| # error [ pp-tokens ] new-line
	| # pragma [ pp-tokens ] new-line
	| # new-line
lparen ::=
	the left_parenthesis character without preceding white_space
replacement-list ::=
	[ pp-tokens ]
pp-tokens ::=
	preprocessing-token
	| pp-tokens preprocessing-token
new-line ::=
	the new_line character
