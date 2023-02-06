#include <stdlib.h>
#include "lexer.h"
#include "bool.h"
#include "string.h"

/*
  Note to self: maybe should have implemented a UTF-8 to Unicode codepoint
	decoder first as a separate function.
 */

LexerResult *tokenize(richtext)
string *richtext;
{
	unsigned long tokenCountEstimate;
	LexerResult *result = malloc(sizeof(LexerResult));
	Vector *tokens;
	Vector *resizedTokens;
	Vector *warnings = Vector_new(sizeof(LexerWarning), 0, 0);
	Vector *resizedWarnings;
	Token token;
	LexerWarning warning;
	unsigned char *currentByte;
	unsigned char nextByte, nextNextByte;
	unsigned long currentByteIndex;
	unsigned long tokenCodepointLength = 0;
	bool isInsideCommand;

	if (richtext == NULL) {
		return NULL;
	}
	if (result == NULL) {
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
	tokens = Vector_new(sizeof(Token), 0, tokenCountEstimate);

	result->type = LexerResultType_SUCCESS;
	result->result.tokens = (TokenVector *) tokens;
	result->warnings = (LexerWarningVector *) warnings;

	if (tokens == NULL) {
		result->type = LexerResultType_ERROR;
		result->result.error.byteIndex = 0;
		result->result.error.codepointIndex = 0;
		result->result.error.code =
		    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
		return result;
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
				Vector_free(tokens);
				result->type = LexerResultType_ERROR;
				result->result.error.byteIndex =
				    currentByteIndex;
				result->result.error.codepointIndex =
				    token.codepointIndex + tokenCodepointLength;
				result->result.error.code =
				    LexerErrorCode_UNEXPECTED_COMMAND_START;
				result->warnings =
				    (LexerWarningVector *) warnings;
				return result;
			}
			isInsideCommand = true;

			if (tokenCodepointLength > 0) {
				token.value =
				    string_substring(richtext, token.byteIndex,
						     currentByteIndex);
				if (token.value == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				resizedTokens = Vector_append(tokens, &token);
				if (resizedTokens == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				tokens = resizedTokens;
				token.codepointIndex += tokenCodepointLength;
				tokenCodepointLength = 0;
			}

			nextByte =
			    currentByteIndex + 1 <
			    richtext->length ? *(currentByte + 1) : 0;
			token.type = nextByte == '/' ?
			    TokenType_COMMAND_END : TokenType_COMMAND_START;
			token.byteIndex = currentByteIndex;
			tokenCodepointLength++;
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
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				resizedTokens = Vector_append(tokens, &token);
				if (resizedTokens == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				tokens = resizedTokens;

				token.byteIndex = currentByteIndex + 1;
				token.codepointIndex +=
				    tokenCodepointLength + 1;
				token.type = TokenType_TEXT;
				tokenCodepointLength = 0;
			} else {
				tokenCodepointLength++;
			}
			break;
		case ' ':
		case '\t':	/* horizontal tab */
		case '\n':	/* newline */
		case '\v':	/* vertical tab */
		case '\f':	/* form feed, richtext documents should use <np> instead */
			if (isInsideCommand) {
				tokenCodepointLength++;
				break;
			}

			if (tokenCodepointLength > 0) {
				token.value =
				    string_substring(richtext, token.byteIndex,
						     currentByteIndex);
				if (token.value == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				resizedTokens = Vector_append(tokens, &token);
				if (resizedTokens == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				tokens = resizedTokens;
				token.codepointIndex += tokenCodepointLength;
				tokenCodepointLength = 0;
			}

			token.byteIndex = currentByteIndex;
			token.type = TokenType_WHITESPACE;
			token.value =
			    string_substring(richtext, currentByteIndex,
					     currentByteIndex + 1);
			if (token.value == NULL) {
				Vector_free(tokens);
				result->type = LexerResultType_ERROR;
				result->result.error.byteIndex =
				    currentByteIndex;
				result->result.error.codepointIndex =
				    token.codepointIndex + tokenCodepointLength;
				result->result.error.code =
				    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
				result->warnings =
				    (LexerWarningVector *) warnings;
				return result;
			}
			resizedTokens = Vector_append(tokens, &token);
			if (resizedTokens == NULL) {
				Vector_free(tokens);
				result->type = LexerResultType_ERROR;
				result->result.error.byteIndex =
				    currentByteIndex;
				result->result.error.codepointIndex =
				    token.codepointIndex + tokenCodepointLength;
				result->result.error.code =
				    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
				result->warnings =
				    (LexerWarningVector *) warnings;
				return result;
			}
			tokens = resizedTokens;

			token.byteIndex = currentByteIndex + 1;
			token.codepointIndex++;
			token.type = TokenType_TEXT;
			break;
		case '\r':	/* carriage return */
			if (isInsideCommand) {
				tokenCodepointLength++;
				break;
			}

			if (tokenCodepointLength > 0) {
				token.value =
				    string_substring(richtext, token.byteIndex,
						     currentByteIndex);
				if (token.value == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				resizedTokens = Vector_append(tokens, &token);
				if (resizedTokens == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				tokens = resizedTokens;

				token.codepointIndex += tokenCodepointLength;
				tokenCodepointLength = 0;
			}

			token.byteIndex = currentByteIndex;
			token.type = TokenType_WHITESPACE;

			/*
			   Treat for CRLF as a single whitespace token because it is meant to be
			   formatted as a single space character.
			 */
			if (currentByteIndex + 1 < richtext->length &&
			    *(currentByte + 1) == '\n') {
				token.value = string_substring(richtext,
							       currentByteIndex,
							       currentByteIndex
							       + 2);
				if (token.value == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				resizedTokens = Vector_append(tokens, &token);
				if (resizedTokens == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				tokens = resizedTokens;
				currentByte++;
				currentByteIndex++;
				token.codepointIndex++;
			} else {
				token.value = string_substring(richtext,
							       currentByteIndex,
							       currentByteIndex
							       + 1);
				if (token.value == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				resizedTokens = Vector_append(tokens, &token);
				if (resizedTokens == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				tokens = resizedTokens;
			}

			token.byteIndex = currentByteIndex + 1;
			token.codepointIndex++;
			token.type = TokenType_TEXT;
			break;
		default:
			if (isInsideCommand) {
				tokenCodepointLength++;
				break;
			}

			/*
			   Since we are operating on a UTF-8 input, we need to correctly count the
			   codepoints and correctly interpret Unicode whitespace (see
			   https://en.wikipedia.org/wiki/Whitespace_character#Unicode).
			 */

			/*
			   Note: overlong character encodings are not supported because they are
			   not valid UTF-8 representations of a code point.
			 */
			if (*currentByte < 128) {	/* single-byte UTF-8 codepoints */
				tokenCodepointLength++;
			} else if (*currentByte <= 192) {	/* unexpected continuation byte */
				warning.byteIndex = currentByteIndex;
				warning.codepointIndex =
				    token.codepointIndex + tokenCodepointLength;
				warning.code =
				    LexerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE;
				resizedWarnings =
				    Vector_append(warnings, &warning);
				if (resizedWarnings == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_WARNINGS;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				warnings = resizedWarnings;
				tokenCodepointLength++;
			} else if (*currentByte <= 223) {	/* Two-byte UTF-8 codepoints */
				nextByte =
				    currentByteIndex + 1 <
				    richtext->length ? *(currentByte + 1) : 0;
				/* Two-byte whitespace characters */
				if ((*currentByte == 194 && nextByte == 133) /* next line */ ||
				    (*currentByte == 194 && nextByte == 160)	/* no-break space */
				    ) {
					if (tokenCodepointLength > 0) {
						token.value =
						    string_substring(richtext,
								     token.byteIndex,
								     currentByteIndex);
						if (token.value == NULL) {
							Vector_free(tokens);
							result->type =
							    LexerResultType_ERROR;
							result->result.
							    error.byteIndex =
							    currentByteIndex;
							result->result.
							    error.codepointIndex
							    =
							    token.codepointIndex
							    +
							    tokenCodepointLength;
							result->result.
							    error.code =
							    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
							result->warnings =
							    (LexerWarningVector
							     *) warnings;
							return result;
						}
						resizedTokens =
						    Vector_append(tokens,
								  &token);
						if (resizedTokens == NULL) {
							Vector_free(tokens);
							result->type =
							    LexerResultType_ERROR;
							result->result.
							    error.byteIndex =
							    currentByteIndex;
							result->result.
							    error.codepointIndex
							    =
							    token.codepointIndex
							    +
							    tokenCodepointLength;
							result->result.
							    error.code =
							    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
							result->warnings =
							    (LexerWarningVector
							     *) warnings;
							return result;
						}
						tokens = resizedTokens;
						token.codepointIndex +=
						    tokenCodepointLength;
						tokenCodepointLength = 0;
					}

					token.byteIndex = currentByteIndex;
					token.type = TokenType_WHITESPACE;
					token.value =
					    string_substring(richtext,
							     currentByteIndex,
							     currentByteIndex +
							     2);
					if (token.value == NULL) {
						Vector_free(tokens);
						result->type =
						    LexerResultType_ERROR;
						result->result.error.byteIndex =
						    currentByteIndex;
						result->result.
						    error.codepointIndex =
						    token.codepointIndex +
						    tokenCodepointLength;
						result->result.error.code =
						    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
						result->warnings =
						    (LexerWarningVector *)
						    warnings;
						return result;
					}
					resizedTokens =
					    Vector_append(tokens, &token);
					if (resizedTokens == NULL) {
						Vector_free(tokens);
						result->type =
						    LexerResultType_ERROR;
						result->result.error.byteIndex =
						    currentByteIndex;
						result->result.
						    error.codepointIndex =
						    token.codepointIndex +
						    tokenCodepointLength;
						result->result.error.code =
						    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
						result->warnings =
						    (LexerWarningVector *)
						    warnings;
						return result;
					}
					tokens = resizedTokens;

					token.byteIndex = currentByteIndex + 2;
					token.codepointIndex++;
					token.type = TokenType_TEXT;
				} else {	/* Other two-byte codepoints */
					tokenCodepointLength++;
				}

				/* Skip over the continuation byte */
				currentByteIndex++;
				currentByte++;
			} else if (*currentByte <= 239) {	/* Three-byte UTF-8 codepoints */
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
					   /* line separator, richtext documents should use <nl> instead */
					   (*currentByte == 226
					    && nextByte == 128
					    && nextNextByte == 168) ||
					   /*
					      paragraph separator, richtext documents should use <Paragraph>
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
					if (tokenCodepointLength > 0) {
						token.value =
						    string_substring(richtext,
								     token.byteIndex,
								     currentByteIndex);
						if (token.value == NULL) {
							Vector_free(tokens);
							result->type =
							    LexerResultType_ERROR;
							result->result.
							    error.byteIndex =
							    currentByteIndex;
							result->result.
							    error.codepointIndex
							    =
							    token.codepointIndex
							    +
							    tokenCodepointLength;
							result->result.
							    error.code =
							    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
							result->warnings =
							    (LexerWarningVector
							     *) warnings;
							return result;
						}
						resizedTokens =
						    Vector_append(tokens,
								  &token);
						if (resizedTokens == NULL) {
							Vector_free(tokens);
							result->type =
							    LexerResultType_ERROR;
							result->result.
							    error.byteIndex =
							    currentByteIndex;
							result->result.
							    error.codepointIndex
							    =
							    token.codepointIndex
							    +
							    tokenCodepointLength;
							result->result.
							    error.code =
							    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
							result->warnings =
							    (LexerWarningVector
							     *) warnings;
							return result;
						}
						tokens = resizedTokens;
						token.codepointIndex +=
						    tokenCodepointLength;
						tokenCodepointLength = 0;
					}

					token.byteIndex = currentByteIndex;
					token.type = TokenType_WHITESPACE;
					token.value =
					    string_substring(richtext,
							     currentByteIndex,
							     currentByteIndex +
							     3);
					if (token.value == NULL) {
						Vector_free(tokens);
						result->type =
						    LexerResultType_ERROR;
						result->result.error.byteIndex =
						    currentByteIndex;
						result->result.
						    error.codepointIndex =
						    token.codepointIndex +
						    tokenCodepointLength;
						result->result.error.code =
						    LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING;
						result->warnings =
						    (LexerWarningVector *)
						    warnings;
						return result;
					}
					resizedTokens =
					    Vector_append(tokens, &token);
					if (resizedTokens == NULL) {
						Vector_free(tokens);
						result->type =
						    LexerResultType_ERROR;
						result->result.error.byteIndex =
						    currentByteIndex;
						result->result.
						    error.codepointIndex =
						    token.codepointIndex +
						    tokenCodepointLength;
						result->result.error.code =
						    LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS;
						result->warnings =
						    (LexerWarningVector *)
						    warnings;
						return result;
					}
					tokens = resizedTokens;

					token.byteIndex = currentByteIndex + 3;
					token.codepointIndex++;
					token.type = TokenType_TEXT;
				} else {	/* Other three-byte codepoints */
					tokenCodepointLength++;
				}

				/* Skip over the continuation bytes */
				currentByteIndex += 2;
				currentByte += 2;
			} else if (*currentByte <= 247) {	/* four-byte UTF-8 codepoints */
				tokenCodepointLength++;
				/* Skip over the continuation bytes */
				currentByteIndex += 3;
				currentByte += 3;
			} else {	/* Invalid UTF-8 character */
				warning.byteIndex = currentByteIndex;
				warning.codepointIndex =
				    token.codepointIndex + tokenCodepointLength;
				warning.code =
				    LexerWarningCode_INVALID_UTF8_CHARACTER;
				resizedWarnings =
				    Vector_append(warnings, &warning);
				if (resizedWarnings == NULL) {
					Vector_free(tokens);
					result->type = LexerResultType_ERROR;
					result->result.error.byteIndex =
					    currentByteIndex;
					result->result.error.codepointIndex =
					    token.codepointIndex +
					    tokenCodepointLength;
					result->result.error.code =
					    LexerErrorCode_OUT_OF_MEMORY_FOR_WARNINGS;
					result->warnings =
					    (LexerWarningVector *) warnings;
					return result;
				}
				warnings = resizedWarnings;
				tokenCodepointLength++;
			}
			break;
		}
	}

	if (token.byteIndex < richtext->length) {
		if (isInsideCommand) {
			result->type = LexerResultType_ERROR;
			result->result.error.byteIndex = richtext->length;
			result->result.error.codepointIndex =
			    token.codepointIndex + tokenCodepointLength;
			result->result.error.code =
			    LexerErrorCode_UNTERMINATED_COMMAND;
			result->warnings = (LexerWarningVector *) warnings;
			return result;
		}

		token.value =
		    string_substring(richtext, token.byteIndex,
				     richtext->length);
		tokens = Vector_append(tokens, &token);
	}

	result->result.tokens = (TokenVector *) tokens;
	result->warnings = (LexerWarningVector *) warnings;
	return result;
}
