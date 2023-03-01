#include <stdlib.h>
#include "lexer.h"
#include "bool.h"
#include "string.h"

/*
  Note to self: maybe should have implemented a UTF-8 to Unicode codepoint
	decoder first as a separate function.
 */

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
	unsigned char nextByte, nextNextByte;
	unsigned long currentByteIndex;
	unsigned long tokenByteIndex = 0;
	unsigned long codepointIndex = 0;
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
		case ' ':
		case '\t':	/* horizontal tab */
		case '\n':	/* newline */
		case '\v':	/* vertical tab */
		case '\f':	/* form feed, richtext documents should use <np> instead */
			if (isInsideCommand) {
				codepointIndex++;
				break;
			}

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

			token.byteIndex = currentByteIndex;
			token.type = TokenType_WHITESPACE;
			token.value =
			    string_substring(richtext, currentByteIndex,
					     currentByteIndex + 1);
			if (token.value == NULL) {
				return finalizeToError(result, currentByteIndex,
						       codepointIndex,
						       LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING,
						       warnings, tokens);
			}
			resizedTokens = TokenVector_append(tokens, &token);
			if (resizedTokens == NULL) {
				return finalizeToError(result, currentByteIndex,
						       codepointIndex,
						       LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
						       warnings, tokens);
			}
			tokens = resizedTokens;

			token.byteIndex = currentByteIndex + 1;
			token.codepointIndex = ++codepointIndex;
			token.type = TokenType_TEXT;
			break;
		case '\r':	/* carriage return */
			if (isInsideCommand) {
				codepointIndex++;
				break;
			}

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

			token.byteIndex = currentByteIndex;
			token.type = TokenType_WHITESPACE;

			/*
			   Treat for CRLF as a single whitespace token because
			   it is meant to be formatted as a single space
			   character.
			 */
			if (currentByteIndex + 1 < richtext->length &&
			    *(currentByte + 1) == '\n') {
				token.value = string_substring(richtext,
							       currentByteIndex,
							       currentByteIndex
							       + 2);
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
				currentByte++;
				currentByteIndex++;
				token.codepointIndex = ++codepointIndex;
			} else {
				token.value = string_substring(richtext,
							       currentByteIndex,
							       currentByteIndex
							       + 1);
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
			}

			token.byteIndex = currentByteIndex + 1;
			token.codepointIndex = ++codepointIndex;
			token.type = TokenType_TEXT;
			break;
		default:
			if (isInsideCommand) {
				codepointIndex++;
				break;
			}

			/*
			   Since we are operating on a UTF-8 input, we need to
			   correctly count the codepoints and correctly
			   interpret Unicode whitespace (see
			   https://en.wikipedia.org/wiki/Whitespace_character#Unicode).
			 */

			/*
			   Note: overlong character encodings are not supported
			   because they are not valid UTF-8 representations of a
			   code point.
			 */
			if (*currentByte < 128) {
				/* single-byte UTF-8 codepoints */
				codepointIndex++;
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
				codepointIndex++;
			} else if (*currentByte <= 223) {
				/* Two-byte UTF-8 codepoints */
				nextByte =
				    currentByteIndex + 1 <
				    richtext->length ? *(currentByte + 1) : 0;
				/* Two-byte whitespace characters */
				/* next line */
				if ((*currentByte == 194 && nextByte == 133) ||
				    /* no-break space */
				    (*currentByte == 194 && nextByte == 160)
				    ) {
					if (currentByteIndex > token.byteIndex) {
						tokenByteIndex =
						    token.byteIndex;
						token.value =
						    string_substring(richtext,
								     tokenByteIndex,
								     currentByteIndex);
						if (token.value == NULL) {
							return
							    finalizeToError
							    (result,
							     currentByteIndex,
							     codepointIndex,
							     LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING,
							     warnings, tokens);
						}
						resizedTokens =
						    TokenVector_append(tokens,
								       &token);
						if (resizedTokens == NULL) {
							return
							    finalizeToError
							    (result,
							     currentByteIndex,
							     codepointIndex,
							     LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
							     warnings, tokens);
						}
						tokens = resizedTokens;
						token.codepointIndex =
						    codepointIndex;
					}

					token.byteIndex = currentByteIndex;
					token.type = TokenType_WHITESPACE;
					token.value =
					    string_substring(richtext,
							     currentByteIndex,
							     currentByteIndex +
							     2);
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

					token.byteIndex = currentByteIndex + 2;
					token.codepointIndex = ++codepointIndex;
					token.type = TokenType_TEXT;
				} else {	/* Other two-byte codepoints */
					codepointIndex++;
				}

				/* Skip over the continuation byte */
				currentByteIndex++;
				currentByte++;
			} else if (*currentByte <= 239) {
				/* Three-byte UTF-8 codepoints */
				nextByte =
				    currentByteIndex + 1 <
				    richtext->length ? *(currentByte + 1) : 0;
				nextNextByte =
				    currentByteIndex + 2 <
				    richtext->length ? *(currentByte + 2) : 0;
				/* Three-byte whitespace characters */
				if (
					   /* ogham space mark */
					   (*currentByte == 225
					    && nextByte == 154
					    && nextNextByte == 128) ||
					   /* en quad */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 128) ||
					   /* em quad */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 129) ||
					   /* en space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 130) ||
					   /* em space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 131) ||
					   /* three-per-em space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 132) ||
					   /* four-per-em space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 133) ||
					   /* six-per-em space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 134) ||
					   /* figure space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 135) ||
					   /* punctuation space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 136) ||
					   /* thin space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 137) ||
					   /* hair space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 138) ||
					   /*
					      line separator, richtext documents
					      should use <nl> instead
					    */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 168) ||
					   /*
					      paragraph separator, richtext
					      documents should use <Paragraph>
					      instead
					    */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 169) ||
					   /* narrow no-break space */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 175) ||
					   /* medium mathematical space */
					   (*currentByte == 226
					    && nextByte == 129
					    && nextNextByte == 159) ||
					   /* ideographic space */
					   (*currentByte == 227
					    && nextByte == 128
					    && nextNextByte == 128)
				    ) {
					if (currentByteIndex > token.byteIndex) {
						tokenByteIndex =
						    token.byteIndex;
						token.value =
						    string_substring(richtext,
								     tokenByteIndex,
								     currentByteIndex);
						if (token.value == NULL) {
							return
							    finalizeToError
							    (result,
							     currentByteIndex,
							     codepointIndex,
							     LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING,
							     warnings, tokens);
						}
						resizedTokens =
						    TokenVector_append(tokens,
								       &token);
						if (resizedTokens == NULL) {
							return
							    finalizeToError
							    (result,
							     currentByteIndex,
							     codepointIndex,
							     LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
							     warnings, tokens);
						}
						tokens = resizedTokens;
						token.codepointIndex =
						    codepointIndex;
					}

					token.byteIndex = currentByteIndex;
					token.type = TokenType_WHITESPACE;
					token.value =
					    string_substring(richtext,
							     currentByteIndex,
							     currentByteIndex +
							     3);
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

					token.byteIndex = currentByteIndex + 3;
					token.codepointIndex = ++codepointIndex;
					token.type = TokenType_TEXT;
				} else {
					/* Other three-byte codepoints */
					codepointIndex++;
				}

				/* Skip over the continuation bytes */
				currentByteIndex += 2;
				currentByte += 2;
			} else if (*currentByte <= 247) {
				/* four-byte UTF-8 codepoints */
				codepointIndex++;
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
				codepointIndex++;
			}
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

Vector_ofTypeImplementation(Token)
    Vector_ofTypeImplementation(LexerWarning)
