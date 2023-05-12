#include <stdlib.h>
#include "lexer.h"
#include "bool.h"
#include "string.h"

/*
   Note: this code was mostly written before applying the code style, that is
   why its design does not match the code style perfectly. Still, it works, and
   that matters the most.
 */

static LexerResult *addWarning(LexerResult * result,
			       LexerWarningVector ** warnings,
			       TokenVector * tokens, unsigned long byteIndex,
			       unsigned long codepointIndex,
			       LexerWarningCode code);

static LexerResult *finalizeToError(LexerResult * result,
				    unsigned long byteIndex,
				    unsigned long codepointIndex,
				    LexerErrorCode code,
				    LexerWarningVector * warnings,
				    TokenVector * tokens);

static void freeTokens(TokenVector * tokens);

static unsigned long getWhitespaceLength(string * input, unsigned long index,
					 bool isUtf8);

LexerResult *tokenize(richtext)
string *richtext;
{
	unsigned long tokenCountEstimate;
	LexerResult *result = malloc(sizeof(LexerResult));
	LexerResult *errorResult;
	TokenVector *tokens;
	TokenVector *resizedTokens;
	LexerWarningVector *warnings = LexerWarningVector_new(0, 0);
	Token token;
	unsigned char *currentByte;
	unsigned char nextByte;
	unsigned long currentByteIndex;
	unsigned long tokenByteIndex = 0;
	unsigned long codepointIndex = 0;
	unsigned long whitespaceLength;
	bool isInsideCommand;

	if (richtext == NULL) {
		LexerWarningVector_free(warnings);
		if (result != NULL) {
			free(result);
		}
		return NULL;
	}
	if (result == NULL) {
		LexerWarningVector_free(warnings);
		return NULL;
	}

	if (warnings == NULL) {
		result->type = LexerResultType_ERROR;
		result->result.error.byteIndex = 0;
		result->result.error.codepointIndex = 0;
		result->result.error.code =
		    LexerErrorCode_OUT_OF_MEMORY_FOR_WARNINGS;
		result->warnings = NULL;
		return result;
	}

	tokenCountEstimate = richtext->length / 1024;
	tokens = TokenVector_new(0, tokenCountEstimate);

	result->type = LexerResultType_SUCCESS;
	result->result.tokens = tokens;
	result->warnings = warnings;

	if (tokens == NULL) {
		return finalizeToError(result, 0, 0,
				       LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
				       warnings, NULL);
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
				return finalizeToError(result, currentByteIndex,
						       codepointIndex,
						       LexerErrorCode_UNEXPECTED_COMMAND_START,
						       warnings, tokens);
			}
			isInsideCommand = true;

			if (currentByteIndex > token.byteIndex) {
				token.value =
				    string_substring(richtext, token.byteIndex,
						     currentByteIndex);
				if (token.value == NULL) {
					return finalizeToError(result,
							       currentByteIndex,
							       codepointIndex,
							       LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING,
							       warnings,
							       tokens);
				}
				resizedTokens =
				    TokenVector_append(tokens, &token);
				if (resizedTokens == NULL) {
					return finalizeToError(result,
							       currentByteIndex,
							       codepointIndex,
							       LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
							       warnings,
							       tokens);
				}
				tokens = resizedTokens;
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
				token.value = string_substring(richtext,
							       token.byteIndex +
							       (token.type ==
								TokenType_COMMAND_END
								? 2 : 1),
							       currentByteIndex);
				if (token.value == NULL) {
					return finalizeToError(result,
							       currentByteIndex,
							       codepointIndex,
							       LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING,
							       warnings,
							       tokens);
				}
				resizedTokens =
				    TokenVector_append(tokens, &token);
				if (resizedTokens == NULL) {
					return finalizeToError(result,
							       currentByteIndex,
							       codepointIndex,
							       LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
							       warnings,
							       tokens);
				}
				tokens = resizedTokens;

				token.byteIndex = currentByteIndex + 1;
				token.codepointIndex = ++codepointIndex;
				token.type = TokenType_TEXT;
			} else {
				codepointIndex++;
			}
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
					tokenByteIndex = token.byteIndex;
					token.value =
					    string_substring(richtext,
							     tokenByteIndex,
							     currentByteIndex);
					if (token.value == NULL) {
						return
						    finalizeToError(result,
								    currentByteIndex,
								    codepointIndex,
								    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING,
								    warnings,
								    tokens);
					}
					resizedTokens =
					    TokenVector_append(tokens, &token);
					if (resizedTokens == NULL) {
						return
						    finalizeToError(result,
								    currentByteIndex,
								    codepointIndex,
								    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
								    warnings,
								    tokens);
					}
					tokens = resizedTokens;
					token.codepointIndex = codepointIndex;
				}

				token.byteIndex = currentByteIndex;
				token.type = TokenType_WHITESPACE;
				token.value =
				    string_substring(richtext,
						     currentByteIndex,
						     currentByteIndex +
						     whitespaceLength);
				if (token.value == NULL) {
					return
					    finalizeToError(result,
							    currentByteIndex,
							    codepointIndex,
							    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING,
							    warnings, tokens);
				}
				resizedTokens =
				    TokenVector_append(tokens, &token);
				if (resizedTokens == NULL) {
					return
					    finalizeToError(result,
							    currentByteIndex,
							    codepointIndex,
							    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
							    warnings, tokens);
				}
				tokens = resizedTokens;

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
					       LexerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE);
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
					       LexerWarningCode_INVALID_UTF8_CHARACTER);
				if (errorResult != NULL) {
					return errorResult;
				}
			}

			codepointIndex++;

			break;
		}
	}

	if (token.byteIndex < richtext->length) {
		if (isInsideCommand) {
			return finalizeToError(result, richtext->length,
					       codepointIndex,
					       LexerErrorCode_UNTERMINATED_COMMAND,
					       warnings, tokens);
		}

		token.value =
		    string_substring(richtext, token.byteIndex,
				     richtext->length);
		tokens = TokenVector_append(tokens, &token);
	}

	result->result.tokens = (TokenVector *) tokens;
	result->warnings = (LexerWarningVector *) warnings;
	return result;
}

void LexerResult_free(result)
LexerResult *result;
{
	if (result == NULL) {
		return;
	}

	LexerWarningVector_free(result->warnings);

	switch (result->type) {
	case LexerResultType_SUCCESS:
		freeTokens(result->result.tokens);
		break;
	case LexerResultType_ERROR:
		break;
	default:
		break;
	}

	free(result);
}

static LexerResult *addWarning(result, warnings, tokens, byteIndex,
			       codepointIndex, code)
LexerResult *result;
LexerWarningVector **warnings;
TokenVector *tokens;
unsigned long byteIndex;
unsigned long codepointIndex;
LexerWarningCode code;
{
	LexerWarning warning;
	LexerWarningVector *resizedWarnings;
	warning.byteIndex = byteIndex;
	warning.codepointIndex = codepointIndex;
	warning.code = code;
	resizedWarnings = LexerWarningVector_append(*warnings, &warning);
	if (resizedWarnings != NULL) {
		*warnings = resizedWarnings;
		return NULL;
	}
	return finalizeToError(result, byteIndex, codepointIndex,
			       LexerErrorCode_OUT_OF_MEMORY_FOR_WARNINGS,
			       *warnings, tokens);
}

static LexerResult *finalizeToError(result, byteIndex, codepointIndex,
				    code, warnings, tokens)
LexerResult *result;
unsigned long byteIndex;
unsigned long codepointIndex;
LexerErrorCode code;
LexerWarningVector *warnings;
TokenVector *tokens;
{
	freeTokens(tokens);
	result->type = LexerResultType_ERROR;
	result->result.error.byteIndex = byteIndex;
	result->result.error.codepointIndex = codepointIndex;
	result->result.error.code = code;
	result->warnings = warnings;
	return result;
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
    Vector_ofTypeImplementation(LexerWarning)
