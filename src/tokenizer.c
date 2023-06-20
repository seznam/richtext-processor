#include <stdlib.h>
#include "tokenizer.h"
#include "bool.h"
#include "string.h"

static TokenizerResult *addWarning(TokenizerResult * result,
				   TokenizerWarningVector ** warnings,
				   TokenVector * tokens,
				   unsigned long byteIndex,
				   unsigned long codepointIndex,
				   TokenizerWarningCode code);

static TokenizerResult *finalizeToError(TokenizerResult * result,
					unsigned long byteIndex,
					unsigned long codepointIndex,
					TokenizerErrorCode code,
					TokenizerWarningVector * warnings,
					TokenVector * tokens);

static TokenizerErrorCode addToken(TokenVector ** tokens, Token token,
				   string * input,
				   unsigned long valueStartIndex,
				   unsigned long valueEndIndex);

static void freeTokens(TokenVector * tokens);

static unsigned long getWhitespaceLength(string * input, unsigned long index,
					 bool isUtf8);

TokenizerResult *tokenize(richtext)
string *richtext;
{
	unsigned long tokenCountEstimate;
	TokenizerResult *result = malloc(sizeof(TokenizerResult));
	TokenizerResult *errorResult;
	TokenVector *tokens;
	TokenizerWarningVector *warnings = TokenizerWarningVector_new(0, 0);
	Token token;
	unsigned char *currentByte;
	unsigned char nextByte;
	unsigned long currentByteIndex;
	unsigned long codepointIndex = 0;
	unsigned long valueStartIndex = 0;
	unsigned long whitespaceLength;
	bool isInsideCommand;
	TokenizerWarningCode WARN_UNEXPECTED_CONT_BYTE =
	    TokenizerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE;
	TokenizerWarningCode WARN_INVALID_CHARACTER =
	    TokenizerWarningCode_INVALID_UTF8_CHARACTER;
	TokenizerErrorCode errorCode = TokenizerErrorCode_OK;

	if (richtext == NULL) {
		TokenizerWarningVector_free(warnings);
		if (result != NULL) {
			free(result);
		}
		return NULL;
	}
	if (result == NULL) {
		TokenizerWarningVector_free(warnings);
		return NULL;
	}

	if (warnings == NULL) {
		result->type = TokenizerResultType_ERROR;
		result->result.error.byteIndex = 0;
		result->result.error.codepointIndex = 0;
		result->result.error.code =
		    TokenizerErrorCode_OUT_OF_MEMORY_FOR_WARNINGS;
		result->warnings = NULL;
		return result;
	}

	tokenCountEstimate = richtext->length / 1024;
	tokens = TokenVector_new(0, tokenCountEstimate);

	result->type = TokenizerResultType_SUCCESS;
	result->result.tokens = tokens;
	result->warnings = warnings;

	if (tokens == NULL) {
		TokenizerErrorCode oomErrorCode =
		    TokenizerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
		return finalizeToError(result, 0, 0, oomErrorCode, warnings,
				       NULL);
	}

	token.byteIndex = 0;
	token.codepointIndex = 0;
	token.type = TokenType_TEXT;

	for (currentByte = richtext->content,
	     currentByteIndex = 0,
	     isInsideCommand = false;
	     currentByteIndex < richtext->length;
	     currentByteIndex++, currentByte++) {
		switch (*currentByte) {
		case '<':
			if (isInsideCommand) {
				errorCode =
				    TokenizerErrorCode_UNEXPECTED_COMMAND_START;
				break;
			}
			isInsideCommand = true;

			if (currentByteIndex > token.byteIndex) {
				errorCode =
				    addToken(&tokens, token, richtext,
					     token.byteIndex, currentByteIndex);
				if (errorCode != TokenizerErrorCode_OK) {
					break;
				}
				token.codepointIndex = codepointIndex;
			}

			nextByte =
			    currentByteIndex + 1 <
			    richtext->length ? *(currentByte + 1) : 0;
			token.type = nextByte == '/' ?
			    TokenType_COMMAND_END : TokenType_COMMAND_START;
			token.byteIndex = currentByteIndex;
			codepointIndex++;
			break;
		case '>':
			if (isInsideCommand) {
				isInsideCommand = false;
				valueStartIndex = token.byteIndex +
				    (token.type ==
				     TokenType_COMMAND_END ? 2 : 1);
				errorCode =
				    addToken(&tokens, token, richtext,
					     valueStartIndex, currentByteIndex);
				if (errorCode != TokenizerErrorCode_OK) {
					break;
				}

				token.byteIndex = currentByteIndex + 1;
				token.codepointIndex = codepointIndex + 1;
				token.type = TokenType_TEXT;
			}
			codepointIndex++;
			break;
		default:
			/*
			   Since we are operating on a UTF-8 input, we need to
			   correctly count the codepoints and correctly
			   interpret Unicode whitespace (see
			   getWhitespaceLength's implementation).
			 */

			/*
			   Note: overlong character encodings are not supported
			   because they are not valid UTF-8 representations of a
			   code point.
			 */

			whitespaceLength =
			    getWhitespaceLength(richtext, currentByteIndex,
						true);

			if (!isInsideCommand && whitespaceLength > 0) {
				if (currentByteIndex > token.byteIndex) {
					TokenizerErrorCode errorOk =
					    TokenizerErrorCode_OK;
					errorCode =
					    addToken(&tokens, token, richtext,
						     token.byteIndex,
						     currentByteIndex);
					if (errorCode != errorOk) {
						break;
					}
					token.codepointIndex = codepointIndex;
				}

				token.byteIndex = currentByteIndex;
				token.type = TokenType_WHITESPACE;
				errorCode =
				    addToken(&tokens, token, richtext,
					     token.byteIndex,
					     currentByteIndex +
					     whitespaceLength);
				if (errorCode != TokenizerErrorCode_OK) {
					break;
				}

				token.byteIndex =
				    currentByteIndex + whitespaceLength;
				token.codepointIndex = codepointIndex + 1;
				if (*currentByte < 128) {
					if (whitespaceLength > 1) {
						/* CRLF */
						token.codepointIndex +=
						    whitespaceLength - 1;
					}
				}
				token.type = TokenType_TEXT;
			}

			if (*currentByte < 128) {
				/* single-byte UTF-8 codepoints */
				if (whitespaceLength > 1) {	/* CRLF */
					currentByteIndex +=
					    whitespaceLength - 1;
					currentByte += whitespaceLength - 1;
					codepointIndex += whitespaceLength - 1;
				}
			} else if (*currentByte <= 192) {
				/* unexpected continuation byte */
				errorResult =
				    addWarning(result, &warnings, tokens,
					       currentByteIndex,
					       codepointIndex,
					       WARN_UNEXPECTED_CONT_BYTE);
				if (errorResult != NULL) {
					return errorResult;
				}
			} else if (*currentByte <= 223) {
				/* Two-byte UTF-8 codepoints */
				/* Skip over the continuation byte */
				currentByteIndex++;
				currentByte++;
			} else if (*currentByte <= 239) {
				/* Three-byte UTF-8 codepoints */
				/* Skip over the continuation bytes */
				currentByteIndex += 2;
				currentByte += 2;
			} else if (*currentByte <= 247) {
				/* Four-byte UTF-8 codepoints */
				/* Skip over the continuation bytes */
				currentByteIndex += 3;
				currentByte += 3;
			} else {
				/* Invalid UTF-8 character */
				errorResult =
				    addWarning(result, &warnings, tokens,
					       currentByteIndex,
					       codepointIndex,
					       WARN_INVALID_CHARACTER);
				if (errorResult != NULL) {
					return errorResult;
				}
			}

			codepointIndex++;

			break;
		}

		if (errorCode != TokenizerErrorCode_OK) {
			break;
		}
	}

	if (errorCode == TokenizerErrorCode_OK
	    && token.byteIndex < richtext->length) {
		if (isInsideCommand) {
			errorCode = TokenizerErrorCode_UNTERMINATED_COMMAND;
		} else {
			token.value =
			    string_substring(richtext, token.byteIndex,
					     richtext->length);
			tokens = TokenVector_append(tokens, &token);
		}
	}

	if (errorCode != TokenizerErrorCode_OK) {
		return finalizeToError(result, currentByteIndex, codepointIndex,
				       errorCode, warnings, tokens);
	}

	result->result.tokens = (TokenVector *) tokens;
	result->warnings = (TokenizerWarningVector *) warnings;
	return result;
}

void TokenizerResult_free(result)
TokenizerResult *result;
{
	if (result == NULL) {
		return;
	}

	TokenizerWarningVector_free(result->warnings);

	switch (result->type) {
	case TokenizerResultType_SUCCESS:
		freeTokens(result->result.tokens);
		break;
	case TokenizerResultType_ERROR:
		break;
	default:
		break;
	}

	free(result);
}

static TokenizerResult *addWarning(result, warnings, tokens, byteIndex,
				   codepointIndex, code)
TokenizerResult *result;
TokenizerWarningVector **warnings;
TokenVector *tokens;
unsigned long byteIndex;
unsigned long codepointIndex;
TokenizerWarningCode code;
{
	TokenizerWarning warning;
	TokenizerWarningVector *resizedWarnings;
	warning.byteIndex = byteIndex;
	warning.codepointIndex = codepointIndex;
	warning.code = code;
	resizedWarnings = TokenizerWarningVector_append(*warnings, &warning);
	if (resizedWarnings != NULL) {
		*warnings = resizedWarnings;
		return NULL;
	}
	return finalizeToError(result, byteIndex, codepointIndex,
			       TokenizerErrorCode_OUT_OF_MEMORY_FOR_WARNINGS,
			       *warnings, tokens);
}

static TokenizerResult *finalizeToError(result, byteIndex, codepointIndex,
					code, warnings, tokens)
TokenizerResult *result;
unsigned long byteIndex;
unsigned long codepointIndex;
TokenizerErrorCode code;
TokenizerWarningVector *warnings;
TokenVector *tokens;
{
	freeTokens(tokens);
	result->type = TokenizerResultType_ERROR;
	result->result.error.byteIndex = byteIndex;
	result->result.error.codepointIndex = codepointIndex;
	result->result.error.code = code;
	result->warnings = warnings;
	return result;
}

static TokenizerErrorCode addToken(tokens, token, input, valueStartIndex,
				   valueEndIndex)
TokenVector **tokens;
Token token;
string *input;
unsigned long valueStartIndex;
unsigned long valueEndIndex;
{
	TokenVector *resizedTokens;

	token.value = string_substring(input, valueStartIndex, valueEndIndex);
	if (token.value == NULL) {
		return TokenizerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
	}
	resizedTokens = TokenVector_append(*tokens, &token);
	if (resizedTokens == NULL) {
		return TokenizerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
	}
	*tokens = resizedTokens;

	return TokenizerErrorCode_OK;
}

static void freeTokens(tokens)
TokenVector *tokens;
{
	Token *token;
	unsigned long i = 0;

	if (tokens == NULL) {
		return;
	}

	for (token = tokens->items; i < tokens->size.length; i++, token++) {
		string_free(token->value);
	}
	TokenVector_free(tokens);
}

static unsigned long getWhitespaceLength(input, index, isUtf8)
string *input;
unsigned long index;
bool isUtf8;
{
	unsigned char *start;
	unsigned char byte1, byte2, byte3;

	if (index >= input->length) {
		return 0;
	}

	start = (input->content + index);
	byte1 = *start;
	byte2 = index < input->length - 1 ? *(start + 1) : 0;
	byte3 = index < input->length - 2 ? *(start + 2) : 0;

	switch (byte1) {
	case ' ':
	case '\t':		/* horizontal tab */
	case '\n':		/* newline */
	case '\v':		/* vertical tab */
	case '\f':		/* form feed, use <np> instead in richtext */
		return 1;
	case '\r':		/* carriage return */
		/*
		 * Treat for CRLF as a single whitespace token because it is
		 * meant to be formatted as a single space character.
		 */
		return byte2 == '\n' ? 2 : 1;
	}

	if (!isUtf8) {
		return 0;
	}

	/* See https://en.wikipedia.org/wiki/Whitespace_character#Unicode */

	if (byte1 < 128) {
		/* single-byte UTF-8 codepoints */
		return 0;
	} else if (byte1 <= 192) {
		/* unexpected continuation byte */
		return 0;
	} else if (byte1 <= 223) {
		/* Two-byte UTF-8 codepoints */
		if (byte1 == 194) {
			switch (byte2) {
			case 133:	/* next line */
			case 160:	/* no-break space */
				return 2;
			}
		}
	} else if (byte1 <= 239) {
		/* Three-byte UTF-8 codepoints */
		switch (byte1) {
		case 225:
			switch (byte2) {
			case 154:
				/* ogham space mark */
				return byte3 == 128 ? 3 : 0;
			}
			break;
		case 226:
			switch (byte2) {
			case 128:
				switch (byte3) {
				case 128:	/* en quad */
				case 129:	/* em quad */
				case 130:	/* en space */
				case 131:	/* em space */
				case 132:	/* three-per-em space */
				case 133:	/* four-per-em space */
				case 134:	/* six-per-em space */
				case 135:	/* figure space */
				case 136:	/* punctuation space */
				case 137:	/* thin space */
				case 138:	/* hair space */
				case 168:	/* line separator, use <nl> instead in richtext */
				case 169:	/* paragraph separator,use <Paragraph> instead in richtext */
				case 175:	/* narrow no-break space */
					return 3;
				}
				break;
			case 129:
				switch (byte3) {
				case 159:	/* medium mathematical space */
					return 3;
				}
				break;
			}
			break;
		case 227:
			switch (byte2) {
			case 128:
				switch (byte3) {
				case 128:	/* ideographic space */
					return 3;
				}
				break;
			}
			break;
		}
	}
	/* There are no four-byte UTF-8 codepoints for whitespace */
	/* We also ignore invalid UTF-8 characters (byte1 > 247) here */

	return 0;
}

Vector_ofTypeImplementation(Token)
    Vector_ofTypeImplementation(TokenizerWarning)
