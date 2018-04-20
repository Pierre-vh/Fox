////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Parser.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements methods that aren't tied to Expression,
//	Statements or Declarations.
////------------------------------------------------------////

#include "Parser.hpp"

#include <sstream>
#include <cassert>
#include <iostream>
#include "Moonshot/Fox/AST/IdentifierTable.hpp"
#include "Moonshot/Fox/Basic/Context.hpp"
#include "Moonshot/Fox/Basic/Exceptions.hpp"

using namespace Moonshot;

Parser::Parser(Context& c, ASTContext& astctxt, TokenVector& l) : context_(c), astcontext_(astctxt), tokens_(l)
{

}

UnitParsingResult Parser::parseUnit()
{
	// <fox_unit>	= {<declaration>}1+
	auto unit = std::make_unique<ASTUnit>();

	// Create recovery "lock" object, since recovery is disabled for top level declarations. 
	// It'll be re-enabled by parseFunctionDeclaration
	auto lock = createRecoveryDisabler();

	// Gather some flags
	const bool showRecoveryMessages = context_.flagsManager.isSet(FlagID::parser_showRecoveryMessages);

	// Parse declarations 
	while (true)
	{
		// Parse a declaration
		auto decl = parseDecl();
		// If the declaration was parsed successfully : continue the cycle.
		if (decl.isUsable())
		{
			unit->addDecl(std::move(decl.result));
			continue;
		}
		else
		{
			// Parsing was successful & EOF -> Job's done !
			if (hasReachedEndOfTokenStream())
				break;
			// No EOF? There's an unexpected token on the way, report it & recover!
			else
			{
				// Unlock, so we're allowed to recover here.
				auto unlock = createRecoveryEnabler();

				// Report an error in case of "not found";
				if (decl.wasSuccessful())	
					errorExpected("Expected a declaration");

				if (showRecoveryMessages)
					genericError("Attempting recovery to next declaration.");

				if (resyncToNextDecl())
				{
					if(showRecoveryMessages)
						genericError("Recovered successfully."); // Note : add a position, like "Recovered successfuly at line x"
					continue;
				}
				else
				{
					if(showRecoveryMessages)
						genericError("Couldn't recover.");
					break;
				}
			}
		}

	}

	if (parserState_.curlyBracketsCount)
		genericError(std::to_string(parserState_.curlyBracketsCount) + " '}' still missing after parsing this unit.");

	if (parserState_.roundBracketsCount)
		genericError(std::to_string(parserState_.roundBracketsCount) + " ')' still missing after parsing this unit.");

	if (parserState_.squareBracketsCount)
		genericError(std::to_string(parserState_.squareBracketsCount) + " ']' still missing after parsing this unit.");

	if (unit->getDeclCount() == 0)
	{
		// Unit reports an error if notfound, because it's a mandatory rule.
		genericError("Expected one or more declaration in unit.");
		// Return empty result
		return UnitParsingResult();
	}
	else
		return UnitParsingResult(std::move(unit));
}

IdentifierInfo* Parser::matchID()
{
	Token t = getToken();
	if (t.isIdentifier())
	{
		consumeToken();

		IdentifierInfo* ptr = t.getIdentifierInfo();;
		assert(ptr && "Token's an identifier but contains a nullptr IdentifierInfo?");
		return ptr;
	}
	return nullptr;
}

bool Parser::matchSign(const SignType & s)
{
	assert(!isBracket(s) && "This method shouldn't be used to match brackets ! Use matchBracket instead!");
	if (getToken().is(s))
	{
		consumeToken();
		return true;
	}
	return false;
}

bool Parser::matchBracket(const SignType & s)
{
	assert(isBracket(s) && "This method should only be used on brackets ! Use matchSign to match instead!");
	auto tok = getToken();
	if (tok.isSign())
	{
		if (!tok.is(s))
			return false;
		switch (s)
		{
			case SignType::S_CURLY_OPEN:
				parserState_.curlyBracketsCount++;
				break;
			case SignType::S_CURLY_CLOSE:
				if (parserState_.curlyBracketsCount)		// Don't let unbalanced parentheses create an underflow.
					parserState_.curlyBracketsCount--;
				break;
			case SignType::S_SQ_OPEN:
				parserState_.squareBracketsCount++;
				break;
			case SignType::S_SQ_CLOSE:
				if (parserState_.squareBracketsCount)		// Don't let unbalanced parentheses create an underflow.
					parserState_.squareBracketsCount--;
				break;
			case SignType::S_ROUND_OPEN:
				parserState_.roundBracketsCount++;
				break;
			case SignType::S_ROUND_CLOSE:
				if (parserState_.roundBracketsCount)		// Don't let unbalanced parentheses create an underflow.
					parserState_.roundBracketsCount--;
				break;
			default:
				throw std::exception("Unknown bracket type"); // Should be unreachable.
		}
		consumeToken();
		return true;
	}
	return false;
}

bool Parser::matchKeyword(const KeywordType & k)
{
	if (getToken().is(k))
	{
		consumeToken();
		return true;
	}
	return false;
}

bool Parser::isBracket(const SignType & s) const
{
	switch (s)
	{
		case SignType::S_CURLY_OPEN:
		case SignType::S_CURLY_CLOSE:
		case SignType::S_SQ_OPEN:
		case SignType::S_SQ_CLOSE:
		case SignType::S_ROUND_OPEN:
		case SignType::S_ROUND_CLOSE:
			return true;
		default:
			return false;
	}
}

const Type* Parser::parseBuiltinTypename()
{
	// <builtin_type_name> 	= "int" | "float" | "bool" | "string" | "char"
	Token t = getToken();
	if (t.isKeyword())
	{
		consumeToken();
		switch (t.getKeywordType())
		{
			case KeywordType::KW_INT:	return  astcontext_.getPrimitiveIntType();
			case KeywordType::KW_FLOAT:	return  astcontext_.getPrimitiveFloatType();
			case KeywordType::KW_CHAR:	return	astcontext_.getPrimitiveCharType();
			case KeywordType::KW_STRING:return	astcontext_.getPrimitiveStringType();
			case KeywordType::KW_BOOL:	return	astcontext_.getPrimitiveBoolType();
		}
		revertConsume();
	}
	return nullptr;
}

std::pair<const Type*, bool> Parser::parseType()
{
	// <type> = <builtin_type_name> { '[' ']' }
	// <builtin_type_name> 
	if (auto ty = parseBuiltinTypename())
	{
		//  { '[' ']' }
		while (matchBracket(SignType::S_SQ_OPEN))
		{
			// Set ty to the ArrayType of ty.
			ty = astcontext_.getArrayTypeForType(ty);
			// ']'
			if (!matchBracket(SignType::S_SQ_CLOSE))
			{
				errorExpected("Expected ']'");
				// Try to recover
				if (resyncToSign(SignType::S_SQ_CLOSE,/*stopAtSemi */ true ,/*shouldConsumeToken*/ true))
					continue;
				// else, return an error.
				return { nullptr , false };
			}
		}
		// found, return
		return { ty, true };
	}
	// notfound, return
	return { nullptr, true };
}

Token Parser::getToken() const
{
	return getToken(parserState_.pos);
}

Token Parser::getToken(const size_t & d) const
{
	if (d < tokens_.size())
		return tokens_.at(d);
	else
		return Token();
}

std::size_t Parser::getCurrentPosition() const
{
	return parserState_.pos;
}

void Parser::consumeToken()
{
	parserState_.pos+=1;
}

void Parser::setPosition(const std::size_t & pos)
{
	parserState_.pos = pos;
}

void Parser::revertConsume()
{
	parserState_.pos-=1;
}

bool Parser::resyncToSign(const SignType & sign, const bool & stopAtSemi, const bool & shouldConsumeToken)
{
	return resyncToSign(std::vector<SignType>({ sign }), stopAtSemi, shouldConsumeToken);
}

bool Parser::resyncToSign(const std::vector<SignType>& signs, const bool & stopAtSemi, const bool & shouldConsumeToken)
{
	// Note, this function is heavily based on CLang's http://clang.llvm.org/doxygen/Parse_2Parser_8cpp_source.html#l00245

	// Return immediately if recovery is not allowed
	if (!parserState_.isRecoveryAllowed)
		return false;

	// Always skip the first token if it's not in signs
	bool isFirst = true;
	// Keep going until we reach EOF.
	for(;!hasReachedEndOfTokenStream();consumeToken())
	{
		// Check curtok
		auto tok = getToken();
		for (auto it = signs.begin(); it != signs.end(); it++)
		{
			if (tok.is(*it))
			{
				if (shouldConsumeToken)
				{
					// if it's a bracket, pay attention to it!
					if (isBracket(*it))
						matchBracket(*it);
					else
						consumeToken();
				}
				return true;
			}
		}
		// Check isFirst
		if (isFirst)
		{
			isFirst = false;
			continue;
		}

		// Check if it's a sign for special behaviours
		if (tok.isSign())
		{
			switch (tok.getSignType())
			{
				// If we find a '(', '{' or '[', call this function recursively to match it's counterpart
				case SignType::S_CURLY_OPEN:
					resyncToSign(SignType::S_CURLY_CLOSE, false, true);
					break;
				case SignType::S_SQ_OPEN:
					resyncToSign(SignType::S_SQ_CLOSE, false, true);
					break;
				case SignType::S_ROUND_OPEN:
					resyncToSign(SignType::S_ROUND_CLOSE, false, true);
					break;
				// If we find a ')', '}' or ']' we  :
					// Check if it belongs to a unmatched counterpart, if so, stop resync attempt.
					// If it doesn't have an opening counterpart skip it.
				case SignType::S_CURLY_CLOSE:
					if (parserState_.curlyBracketsCount)
						return false;
					matchBracket(SignType::S_CURLY_CLOSE);
					break;
				case SignType::S_SQ_CLOSE:
					if (parserState_.squareBracketsCount)
						return false;
					matchBracket(SignType::S_SQ_CLOSE);
					break;
				case SignType::S_ROUND_CLOSE:
					if (parserState_.roundBracketsCount)
						return false;
					matchBracket(SignType::S_ROUND_CLOSE);
					break;
				case SignType::S_SEMICOLON:
					if (stopAtSemi)
						return false;
					break;
			}
		}
	}
	// If reached eof, die & return false.
	die();
	return false;
}

bool Parser::resyncToNextDecl()
{
	// This method skips everything until it finds a "let" or a "func".

	// Return immediately if recovery is not allowed
	if (!parserState_.isRecoveryAllowed)
		return false;

	// Keep on going until we find a "func" or "let"
	for (; !hasReachedEndOfTokenStream(); consumeToken())
	{
		auto tok = getToken();
		// if it's let/func, return.
		if (tok.isKeyword())
		{
			if ((tok.getKeywordType() == KeywordType::KW_FUNC) && (tok.getKeywordType() == KeywordType::KW_LET))
				return true;
		}

		// Check if it's a sign for special brackets-related actions
		if (tok.isSign())
		{
			switch (tok.getSignType())
			{
				// If we find a '(', '{' or '[', call resyncToSign to match it's counterpart
				case SignType::S_CURLY_OPEN:
					resyncToSign(SignType::S_CURLY_CLOSE, false, true);
					break;
				case SignType::S_SQ_OPEN:
					resyncToSign(SignType::S_SQ_CLOSE, false, true);
					break;
				case SignType::S_ROUND_OPEN:
					resyncToSign(SignType::S_ROUND_CLOSE, false, true);
					break;
				// If we find a ')', '}' or ']' we match (consume) it.
				case SignType::S_CURLY_CLOSE:
					matchBracket(SignType::S_CURLY_CLOSE);
					break;
				case SignType::S_SQ_CLOSE:
					matchBracket(SignType::S_SQ_CLOSE);
					break;
				case SignType::S_ROUND_CLOSE:
					matchBracket(SignType::S_ROUND_CLOSE);
					break;
			}
		}
	}
	// If reached eof, die & return false.
	die();
	return false;
}

void Parser::die()
{
	if(context_.flagsManager.isSet(FlagID::parser_showRecoveryMessages))
		genericError("Couldn't recover from error, stopping parsing.");

	parserState_.pos = tokens_.size();
	parserState_.isAlive = false;
}

void Parser::errorUnexpected()
{
	if (!parserState_.isAlive) return;

	context_.setOrigin("Parser");

	std::stringstream output;
	auto tok = getToken();
	if (tok)
	{
		if (tok.getAsString().size() == 1)
			output << "Unexpected char '" << tok.getAsString() << "' at line " << tok.getPosition().line;
		else
			output << "Unexpected Token \xAF" << tok.getAsString() << "\xAE at line " << tok.getPosition().line;
		context_.reportError(output.str());
	}
	context_.resetOrigin();
}

void Parser::errorExpected(const std::string & s)
{
	if (!parserState_.isAlive) return;

	const auto lastTokenPos = parserState_.pos - 1;

	// If needed, print unexpected error message
	if (lastUnexpectedTokenPosition_ != parserState_.pos)
	{
		lastUnexpectedTokenPosition_ = parserState_.pos;
		errorUnexpected();
	}

	context_.setOrigin("Parser");

	std::stringstream output;
	auto tok = getToken(lastTokenPos);
	output << s << " after \"" << tok.getAsString() << "\" at line " << tok.getPosition().line;

	context_.reportError(output.str());
	context_.resetOrigin();
}

void Parser::genericError(const std::string & s)
{
	if (!parserState_.isAlive) return;

	context_.setOrigin("Parser");
	context_.reportError(s);
	context_.resetOrigin();
}

bool Parser::hasReachedEndOfTokenStream() const
{
	return (parserState_.pos >= tokens_.size());
}

bool Parser::isAlive() const
{
	return parserState_.isAlive;
}

Parser::ParserState Parser::createParserStateBackup() const
{
	return parserState_;
}

void Parser::restoreParserStateFromBackup(const Parser::ParserState & st)
{
	parserState_ = st;
}

// ParserState
Parser::ParserState::ParserState() : isAlive(true), isRecoveryAllowed(false), pos(0)
{

}

// RAIIRecoveryManager
Parser::RAIIRecoveryManager::RAIIRecoveryManager(Parser &parser, const bool & allowsRecovery) : parser_(parser)
{
	recoveryAllowedBackup_ = parser_.parserState_.isRecoveryAllowed;
	parser_.parserState_.isRecoveryAllowed = allowsRecovery;
}

Parser::RAIIRecoveryManager::~RAIIRecoveryManager()
{
	parser_.parserState_.isRecoveryAllowed = recoveryAllowedBackup_;
}

Parser::RAIIRecoveryManager Parser::createRecoveryEnabler()
{
	return RAIIRecoveryManager(*this,true);
}

Parser:: RAIIRecoveryManager Parser::createRecoveryDisabler()
{
	return RAIIRecoveryManager(*this, false);
}