#include <stdlib.h>
#include "utf8/single_byte_encoding.h"
#include "utf8/single_byte_to_utf8.h"
#include "tokenizer.h"
#include "bool.h"
#include "string.h"
#include "token.h"
#include "token_type.h"
#include "token_vector.h"
#include "typed_vector.h"

typedef enum TextEncoding {
	TextEncoding_UNKNOWN,
	TextEncoding_US_ASCII,
	TextEncoding_ISO_8859_1,
	TextEncoding_ISO_8859_2,
	TextEncoding_ISO_8859_3,
	TextEncoding_ISO_8859_4,
	TextEncoding_ISO_8859_5,
	TextEncoding_ISO_8859_6,
	TextEncoding_ISO_8859_7,
	TextEncoding_ISO_8859_8,
	TextEncoding_ISO_8859_9,
	TextEncoding_ISO_8859_10,
	TextEncoding_ISO_8859_11,
	TextEncoding_ISO_8859_13,
	TextEncoding_ISO_8859_14,
	TextEncoding_ISO_8859_15,
	TextEncoding_ISO_8859_16,
	TextEncoding_UTF8
} TextEncoding;

Vector_ofType(TextEncoding)
static TokenizerResult *addWarning(TokenizerResult *result,
				   TokenizerWarningVector **warnings,
				   TokenVector *tokens,
				   unsigned long byteIndex,
				   unsigned long codepointIndex,
				   TokenizerWarningCode code);

static TokenizerResult *finalizeToError(TokenizerResult *result,
					unsigned long byteIndex,
					unsigned long codepointIndex,
					TokenizerErrorCode code,
					TokenizerWarningVector *warnings,
					TokenVector *tokens);

static TokenizerErrorCode addToken(TokenVector **tokens, Token *token,
				   string *input,
				   unsigned long valueStartIndex,
				   unsigned long valueEndIndex,
				   TextEncoding valueEncoding);

static void freeTokens(TokenVector *tokens);

static unsigned long getWhitespaceLength(string *input, unsigned long index,
					 bool isUtf8);

static TokenizerErrorCode updateEncodingStack(TextEncodingVector
					      **encodingStack,
					      TextEncoding *currentEncoding,
					      Token *token,
					      bool caseInsensitiveCommands);

static TextEncoding tokenToEncoding(Token *token, bool caseInsensitive);

static string *transcodeToUtf8(string *input, TextEncoding inputEncoding);

static bool string_equals(string *string1, string *string2,
			  bool caseInsensitive);

TokenizerResult *tokenize(richtext, caseInsensitiveCommands, isUtf8)
string *richtext;
bool caseInsensitiveCommands;
bool isUtf8;
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
	bool caseInsensitive = caseInsensitiveCommands;
	TextEncodingVector *encodingStack = NULL;
	TextEncoding currentEncoding =
	    isUtf8 ? TextEncoding_UTF8 : TextEncoding_US_ASCII;
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

	encodingStack = TextEncodingVector_new(0, 0);
	if (encodingStack == NULL) {
		result->type = TokenizerResultType_ERROR;
		result->result.error.byteIndex = 0;
		result->result.error.codepointIndex = 0;
		result->result.error.code =
		    TokenizerErrorCode_OUT_OF_MEMORY_FOR_ENCODING_STACK;
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
				    addToken(&tokens, &token, richtext,
					     token.byteIndex, currentByteIndex,
					     currentEncoding);
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
				    addToken(&tokens, &token, richtext,
					     valueStartIndex, currentByteIndex,
					     currentEncoding);
				if (errorCode != TokenizerErrorCode_OK) {
					break;
				}
				errorCode =
				    updateEncodingStack(&encodingStack,
							&currentEncoding,
							&token,
							caseInsensitive);
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
			   Since we are operating on an input potentially
			   containing UTF-8, we need to correctly count the
			   codepoints and correctly interpret Unicode
			   whitespace (see getWhitespaceLength's
			   implementation).
			 */

			/*
			   Note: overlong character encodings are not supported
			   because they are not valid UTF-8 representations of a
			   code point.
			 */

			whitespaceLength =
			    getWhitespaceLength(richtext, currentByteIndex,
						currentEncoding ==
						TextEncoding_UTF8);

			if (!isInsideCommand && whitespaceLength > 0) {
				if (currentByteIndex > token.byteIndex) {
					TokenizerErrorCode errorOk =
					    TokenizerErrorCode_OK;
					errorCode =
					    addToken(&tokens, &token, richtext,
						     token.byteIndex,
						     currentByteIndex,
						     currentEncoding);
					if (errorCode != errorOk) {
						break;
					}
					token.codepointIndex = codepointIndex;
				}

				token.byteIndex = currentByteIndex;
				token.type = TokenType_WHITESPACE;
				errorCode =
				    addToken(&tokens, &token, richtext,
					     token.byteIndex,
					     currentByteIndex +
					     whitespaceLength, currentEncoding);
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

			if (currentEncoding != TextEncoding_UTF8) {
				/* Nothing to do for single-byte encodings */
			} else if (*currentByte < 128) {
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
				   valueEndIndex, valueEncoding)
TokenVector **tokens;
Token *token;
string *input;
unsigned long valueStartIndex;
unsigned long valueEndIndex;
TextEncoding valueEncoding;
{
	TokenVector *resizedTokens;
	string *tokenValue;

	tokenValue = string_substring(input, valueStartIndex, valueEndIndex);
	if (tokenValue == NULL) {
		return TokenizerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
	}

	if (valueEncoding == TextEncoding_UTF8) {
		token->value = tokenValue;
	} else {
		token->value = transcodeToUtf8(tokenValue, valueEncoding);
		string_free(tokenValue);
		if (token->value == NULL) {
			return TokenizerErrorCode_TEXT_DECODING_FAILURE;
		}
	}

	resizedTokens = TokenVector_append(*tokens, token);
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
		/* ISO-8859-* has only one extra white-space above 127 */
		if (byte1 == 160) {	/* Non-breaking space */
			return 1;
		}
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

static TokenizerErrorCode updateEncodingStack(encodingStack, currentEncoding,
					      token, caseInsensitiveCommands)
TextEncodingVector **encodingStack;
TextEncoding *currentEncoding;
Token *token;
bool caseInsensitiveCommands;
{
	TextEncoding tokenEncoding =
	    tokenToEncoding(token, caseInsensitiveCommands);
	TextEncodingVector *changedVector;

	if (tokenEncoding == TextEncoding_UNKNOWN) {
		/* The token is not an encoding-related command. */
		return TokenizerErrorCode_OK;
	}

	if (token->type == TokenType_COMMAND_START) {
		changedVector =
		    TextEncodingVector_append(*encodingStack, currentEncoding);
		if (changedVector == NULL) {
			return
			    TokenizerErrorCode_OUT_OF_MEMORY_FOR_ENCODING_STACK;
		}
		*encodingStack = changedVector;
		*currentEncoding = tokenEncoding;
	} else {
		if (*currentEncoding != tokenEncoding) {
			return TokenizerErrorCode_UNBALANCED_ENCODING_COMMANDS;
		}
		changedVector =
		    TextEncodingVector_pop(*encodingStack, currentEncoding);
		if (changedVector == NULL) {
			return TokenizerErrorCode_ENCODING_STACK_UNDERFLOW;
		}
		*encodingStack = changedVector;
	}

	return TokenizerErrorCode_OK;
}

static TextEncoding tokenToEncoding(token, caseInsensitive)
Token *token;
bool caseInsensitive;
{
	unsigned char encodingValue[11];
	string encoding;
	unsigned char codepage;

	if (token == NULL) {
		return TextEncoding_UNKNOWN;
	}

	encoding.content = encodingValue;

	encodingValue[0] = 'U';
	encodingValue[1] = 'S';
	encodingValue[2] = '-';
	encodingValue[3] = 'A';
	encodingValue[4] = 'S';
	encodingValue[5] = 'C';
	encodingValue[6] = 'I';
	encodingValue[7] = 'I';
	encoding.length = 8;

	if (string_equals(token->value, &encoding, caseInsensitive)) {
		return TextEncoding_US_ASCII;
	}

	encodingValue[0] = 'I';
	encodingValue[1] = 'S';
	encodingValue[2] = 'O';
	encodingValue[3] = '-';
	encodingValue[4] = '8';
	encodingValue[5] = '8';
	encodingValue[6] = '5';
	encodingValue[7] = '9';
	encodingValue[8] = '-';
	encoding.length = 10;

	for (codepage = 1; codepage <= 9; codepage++) {
		encodingValue[9] = '0' + codepage;
		if (string_equals(token->value, &encoding, caseInsensitive)) {
			return TextEncoding_ISO_8859_1 + codepage - 1;
		}
	}

	encodingValue[9] = '1';
	encoding.length = 11;
	for (codepage = 0; codepage <= 6; codepage++) {
		if (codepage == 2) {
			continue;
		}

		encodingValue[10] = '0' + codepage;
		if (string_equals(token->value, &encoding, caseInsensitive)) {
			return TextEncoding_ISO_8859_10 + codepage - (codepage >
								      2 ? 1 :
								      0);
		}
	}

	return TextEncoding_UNKNOWN;
}

static string *transcodeToUtf8(input, inputEncoding)
string *input;
TextEncoding inputEncoding;
{
	SingleByteEncoding encoding;

	switch (inputEncoding) {
	case TextEncoding_US_ASCII:
		encoding = SingleByteEncoding_US_ASCII;
		break;
	case TextEncoding_ISO_8859_1:
		encoding = SingleByteEncoding_ISO_8859_1;
		break;
	case TextEncoding_ISO_8859_2:
		encoding = SingleByteEncoding_ISO_8859_2;
		break;
	case TextEncoding_ISO_8859_3:
		encoding = SingleByteEncoding_ISO_8859_3;
		break;
	case TextEncoding_ISO_8859_4:
		encoding = SingleByteEncoding_ISO_8859_4;
		break;
	case TextEncoding_ISO_8859_5:
		encoding = SingleByteEncoding_ISO_8859_5;
		break;
	case TextEncoding_ISO_8859_6:
		encoding = SingleByteEncoding_ISO_8859_6;
		break;
	case TextEncoding_ISO_8859_7:
		encoding = SingleByteEncoding_ISO_8859_7;
		break;
	case TextEncoding_ISO_8859_8:
		encoding = SingleByteEncoding_ISO_8859_8;
		break;
	case TextEncoding_ISO_8859_9:
		encoding = SingleByteEncoding_ISO_8859_9;
		break;
	case TextEncoding_ISO_8859_10:
		encoding = SingleByteEncoding_ISO_8859_10;
		break;
	case TextEncoding_ISO_8859_11:
		encoding = SingleByteEncoding_ISO_8859_11;
		break;
	case TextEncoding_ISO_8859_13:
		encoding = SingleByteEncoding_ISO_8859_13;
		break;
	case TextEncoding_ISO_8859_14:
		encoding = SingleByteEncoding_ISO_8859_14;
		break;
	case TextEncoding_ISO_8859_15:
		encoding = SingleByteEncoding_ISO_8859_15;
		break;
	case TextEncoding_ISO_8859_16:
		encoding = SingleByteEncoding_ISO_8859_16;
		break;
	default:
		return NULL;
	}

	return transcodeSingleByteEncodedTextToUtf8(encoding, input);
}

static bool string_equals(string1, string2, caseInsensitive)
string *string1;
string *string2;
bool caseInsensitive;
{
	if (caseInsensitive) {
		return string_caseInsensitiveCompare(string1, string2) == 0;
	}
	return string_compare(string1, string2) == 0;
}

Vector_ofTypeImplementation(TokenizerWarning)
    Vector_ofTypeImplementation(TextEncoding)
