## File headers
file_header.txt is the text that must be copy-pasted at the beginning of every file of the Moonshot project. 

## Grammar
grammar_x.x.txt contains the current version of the Fox programming language's grammar in EBNF Format. The version number is incremented by .1 
after minor changes are made.

### Grammar Changelog : 
* 0.0 ->	First version that had a lot of error (it was my first try at making a full ebnf grammar).</br>
* 0.1 ->	Reworked expressions, added one-stmt blocks and support for unary operators ! and - and type conversion.</br>
* 0.2 ->	Grammar cleanup</br>
* 0.3 ->	Now using Expression Statements instead of f_call_stmt and var_ass_stmt. </br>
		Added <expr_stmt> which replaces var_assign & f_call statements.</br>
* 0.4 ->	Namespaces removed
* 0.4.1->	\<block> renamed to <compound_statement>. </br>
* 0.4.2->	<return> : <expr> is now optional (to allow "return;" in void functions). </br>
* 0.4.3->	Deleted the <ctrl_flow> non-terminal and added  \<condition> | \<while_loop> to \<stmt> rule.</br>
* 0.5->		Expression rework. They're now far more flexible and support unary operation chaining, like !!false, !-3, etc.</br>
* 0.5.1->	Changed Exponent sign to "**" instead of '^' like in other languages ( python, fortran,.. )</br>
* 0.6->		</Br>
	Final pass on the expressions : right associativity is directly specified in the grammar for exponent and = operators. </br>
			Changed exponent operator's priority to be mathematically correct.</br>
			Renamed <const> to <literal>*</br>
			Added '+' unary op</br>
* 0.6.5-> </br>	
		Changed <compound_statement> rule : removed <statement> as an option</br>
		Changed statement rule: added the <compound_statement> and <rtr_stmt> rule as options</br>
		Changed <compound_statement> in statements that used it before to just <statement></br>
* 0.7.0->	</br>
Removed the <eoi> nonterminal. It was completly useless. I just replaced all of them with ';'</br>
		Changed (and corrected) the grammar for if/else if/else statements. It had a typo, and i've cleaned it up a bit to eliminate redudancy.</br>
		Added the empty ';' statement to the expression statement, like in C and C++.</br>
		Removed the namespace rule and added it to the scrapped idea category</br>
		Removed the <xxx_kw> nonterminal, as it was useless. I replaced every <XXX_kw> rule by the proper keyword (e.g. <wh_kw> -> "while")</br>

* 0.8.0-> </br>
		Balanced the <condition> grammar to use a C-like syntax, which is better for codegen and more stable, and also easier to parse! </br>
		Added a <parens_expr> rule to avoid repetitions. </br>
		Minor changes to function declaration rules (only rule changes, but they work exactly the same) </br>

* 0.8.1 -> </br>
		Changed the <value> rule to use the new <parens_expr> rule.

* 0.9.0	-> </br>
		Removed <compound_statement> as an alternative in <stmt> rule
		Created a new <body> rule that can either be a <stmt> or a <compound_stmt>
		<condition> and <while_loop> now use <body> instead of <stmt>
			Note: the only consequence of theses change is that it's now forbidden to have "free" compound statements. (not tied to a condition,loop,etc.)
		Removed <f_call> rule
		Removed <callable> as an alternative in <value>, now using <id> instead
		Added a <trailer> rule	-> = '(' <expr_list> ')' | ('.' <id>)
		Added a <atom> rule		-> <value> {<trailer>}
		<exp_expr> rule modified to use <atom> instead of <value>

* 0.9.5 -> </br>
	This version is a fix for the 0.9.0's  ill-formed grammar. The changelong is a 0.8.1->0.9.5 changelog.
	Removed useless description text at the top of the grammar
		Removed <compound_statement> as an alternative in <stmt> rule
		Created a new <body> rule that can either be a <stmt> or a <compound_stmt>
		<condition> and <while_loop> now use <body> instead of <stmt>
			Note: the only consequence of theses change is that it's now forbidden to have "free" compound statements. (not tied to a condition,loop,etc.)
		Removed <f_call> rule
		Added <expr_list> rule (it was referenced in other rules, but not defined anywhere!)
			<expr_list> = <expr> {',' <expr> }
		Added <parens_expr_list> = '(' [<expr_list> ] ')'
		Reworked <expr> grammar
			Added <decl_ref> rule to represent both variable and function calls at "primary" level
			<value> renamed to <primary>, removed <id> and added <decl_ref> instead
			Deleted <atom> and <trailer> rules
			Added <member_access> = <primary> { '.' <id> [ <parens_expr_list> ] }
			Replaced the call to <atom> in <exp_expr> to <member_access>