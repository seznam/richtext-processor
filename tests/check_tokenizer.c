#include <string.h>
#include "../src/bool.h"
#include "../src/tokenizer.h"
#include "../src/string.h"
#include "../src/token.h"
#include "../src/token_type.h"
#include "unit.h"

/*
   This file does not bother to free heap-allocated memory because it's a suite
	 of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

int main(void);

static char *_assertWarning(char *fileName, unsigned int lineOfCode,
			    unsigned long warningOrdinalNumber,
			    TokenizerWarning * warning, unsigned long byteIndex,
			    unsigned long codepointIndex,
			    TokenizerWarningCode code);

static char *_assertToken(char *fileName, unsigned int lineOfCode,
			  unsigned long tokenOrdinalNumber, Token * token,
			  unsigned long byteIndex, unsigned long codepointIndex,
			  TokenType type, char *value);

static char *STRINGIFIED_TOKEN_TYPE[] = {
	"TokenType_COMMAND_START",
	"TokenType_COMMAND_END",
	"TokenType_TEXT",
	"TokenType_WHITESPACE"
};

static char *STRINGIFIED_TOKENIZER_WARNING[] = {
	"TokenizerWarningCode_INVALID_UTF8_CHARACTER",
	"TokenizerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE"
};

#define assertWarning(warningOrdinalNumber, warning, byteIndex, codepointIndex, code)\
do {\
	char *_assertError = _assertWarning(__FILE__, __LINE__, warningOrdinalNumber, warning, byteIndex, codepointIndex, code);\
	if (_assertError != NULL) {\
		return _assertError;\
	}\
} while (0)

#define assertToken(tokenOrdinalNumber, token, byteIndex, codepointIndex, type, value)\
do {\
	char *_assertError = _assertToken(__FILE__, __LINE__, tokenOrdinalNumber, token, byteIndex, codepointIndex, type, value);\
	if (_assertError != NULL) {\
    return _assertError;\
  }\
} while (0)

START_TEST(tokenize_returnsNullForNullInput)
{
	assert(tokenize(NULL, false, false) == NULL,
	       "Expected NULL returned for NULL input");
END_TEST}

START_TEST(tokenize_processesSingleWord)
{
	TokenizerResult *result = tokenize(string_from("foo"), true, true);
	Token token;
	assert(result != NULL, "Expected non-NULL result");
	assert(result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL && result->warnings->size.length == 0,
	       "Expected empty vector of warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == 1,
	       "Expected a single token");
	token = *result->result.tokens->items;
	assert(token.byteIndex == 0
	       && token.codepointIndex == 0,
	       "Expected the token to be at 0 index");
	assert(token.type == TokenType_TEXT,
	       "Expected the token to have type TEXT");
	assert(string_compare(token.value, string_from("foo")) == 0,
	       "Expected the token's value to be 'foo'");
END_TEST}

START_TEST(tokenize_processMultipleWordsSeparatedByRegularSpaces)
{
	TokenizerResult *result =
	    tokenize(string_from("foo bar baz"), true, true);
	Token *token;
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL
	       && result->warnings->size.length == 0,
	       "Expected empty vector of warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == 5, "Expected 5 tokens");
	token = result->result.tokens->items;
	assert(token->byteIndex == 0
	       && token->codepointIndex == 0,
	       "Expected the first token to be at index 0");
	assert(token->type == TokenType_TEXT,
	       "Expected the first token to be of type TEXT");
	assert(string_compare(token->value, string_from("foo")) == 0,
	       "Expected the first token's value to be 'foo'");
	token++;
	assert(token->byteIndex == 3
	       && token->codepointIndex == 3,
	       "Expected the second token to be at index 3");
	assert(token->type == TokenType_WHITESPACE,
	       "Expected the second token to be of type WHITESPACE");
	assert(string_compare(token->value, string_from(" ")) == 0,
	       "Expected the second token's value to be ' '");
	token++;
	assert(token->byteIndex == 4
	       && token->codepointIndex == 4,
	       "Expected the third token to be at index 4");
	assert(token->type == TokenType_TEXT,
	       "Expected the third token to be of type TEXT");
	assert(string_compare(token->value, string_from("bar")) == 0,
	       "Expected the third token's value to be 'bar'");
	token++;
	assert(token->byteIndex == 7
	       && token->codepointIndex == 7,
	       "Expected the fourth token to be at index 7");
	assert(token->type == TokenType_WHITESPACE,
	       "Expected the fourth token to be of type WHITESPACE");
	assert(string_compare(token->value, string_from(" ")) == 0,
	       "Expected the fourth token's value to be ' '");
	token++;
	assert(token->byteIndex == 8
	       && token->codepointIndex == 8,
	       "Expected the fifth token to be at index 8");
	assert(token->type == TokenType_TEXT,
	       "Expected the fifth token to be of type TEXT");
	assert(string_compare(token->value, string_from("baz")) == 0,
	       "Expected the fifth token's value to be 'baz'");
END_TEST}

START_TEST(tokenize_emitsTokensForEveryIndividualAsciiWhitespaceExceptCrLf)
{
	TokenizerResult *result =
	    tokenize(string_from("\r \t\n\v\f"), true, true);
	Token *token;
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL
	       && result->warnings->size.length == 0,
	       "Expected empty vector of warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == 6, "Expected 6 tokens");
	token = result->result.tokens->items;

	assertToken(1, token, 0, 0, TokenType_WHITESPACE, "\r");
	token++;
	assertToken(2, token, 1, 1, TokenType_WHITESPACE, " ");
	token++;
	assertToken(3, token, 2, 2, TokenType_WHITESPACE, "\t");
	token++;
	assertToken(4, token, 3, 3, TokenType_WHITESPACE, "\n");
	token++;
	assertToken(5, token, 4, 4, TokenType_WHITESPACE, "\v");
	token++;
	assertToken(6, token, 5, 5, TokenType_WHITESPACE, "\f");
END_TEST}

START_TEST(tokenize_emitsCrLfInSingleWhitespaceToken)
{
	TokenizerResult *result = tokenize(string_from(" \r\n "), true, true);
	Token *token;
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL
	       && result->warnings->size.length == 0,
	       "Expected empty vector of warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == 3, "Expected 3 tokens");
	token = result->result.tokens->items;

	assertToken(1, token, 0, 0, TokenType_WHITESPACE, " ");
	token++;
	assertToken(2, token, 1, 1, TokenType_WHITESPACE, "\r\n");
	token++;
	assertToken(3, token, 3, 3, TokenType_WHITESPACE, " ");
END_TEST}

START_TEST(tokenize_recognizesAllTwoByteWhitespaceAsSingleTokens)
{
	TokenizerResult *result =
	    tokenize(string_from("\302\205\302\240"), true, true);
	Token *token;
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL
	       && result->warnings->size.length == 0,
	       "Expected empty vector of warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == 2, "Expected 2 tokens");
	token = result->result.tokens->items;
	assertToken(1, token, 0, 0, TokenType_WHITESPACE, "\302\205");
	assert(token->value->length == 2,
	       "Expected the token to be two-byte long");
	token++;
	assertToken(2, token, 2, 1, TokenType_WHITESPACE, "\302\240");
	assert(token->value->length == 2,
	       "Expected the token to be two-byte long");
END_TEST}

START_TEST(tokenize_recognizesAllThreeByteWhitespaceAsSingleTokens)
{
	Vector *whitespace = Vector_new(sizeof(char[3]), 0, 0);
	char *whitespaceString;
	TokenizerResult *result;
	Token *token;
	unsigned long index;

	whitespace = Vector_append(whitespace, "\341\232\200");
	whitespace = Vector_append(whitespace, "\342\200\200");
	whitespace = Vector_append(whitespace, "\342\200\201");
	whitespace = Vector_append(whitespace, "\342\200\202");
	whitespace = Vector_append(whitespace, "\342\200\203");
	whitespace = Vector_append(whitespace, "\342\200\204");
	whitespace = Vector_append(whitespace, "\342\200\205");
	whitespace = Vector_append(whitespace, "\342\200\206");
	whitespace = Vector_append(whitespace, "\342\200\207");
	whitespace = Vector_append(whitespace, "\342\200\210");
	whitespace = Vector_append(whitespace, "\342\200\211");
	whitespace = Vector_append(whitespace, "\342\200\212");
	whitespace = Vector_append(whitespace, "\342\200\250");
	whitespace = Vector_append(whitespace, "\342\200\251");
	whitespace = Vector_append(whitespace, "\342\200\257");
	whitespace = Vector_append(whitespace, "\342\201\237");
	whitespace = Vector_append(whitespace, "\343\200\200");

	whitespaceString =
	    calloc(sizeof(char),
		   whitespace->size.length * whitespace->size.itemSize + 1);
	for (index = 0; index < whitespace->size.length; index++) {
		char *character = calloc(sizeof(char), 4);
		Vector_get(whitespace, index, character);
		strcat(whitespaceString, character);
	}

	result = tokenize(string_from(whitespaceString), true, true);
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL
	       && result->warnings->size.length == 0,
	       "Expected empty vector of warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == whitespace->size.length,
	       "Expected 17 tokens");
	token = result->result.tokens->items;

	for (index = 0; index < whitespace->size.length; index++) {
		char *character = calloc(sizeof(char), 4);
		Vector_get(whitespace, index, character);
		assertToken(index + 1, token + index, index * 3, index,
			    TokenType_WHITESPACE, character);
	}
END_TEST}

START_TEST(tokenize_processesCommandStarts)
{
	TokenizerResult *result = tokenize(string_from("<abc>"), true, true);
	Token *token;
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL
	       && result->warnings->size.length == 0,
	       "Expected empty vector of warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == 1, "Expected 1 token");
	token = result->result.tokens->items;

	assertToken(1, token, 0, 0, TokenType_COMMAND_START, "abc");
END_TEST}

START_TEST(tokenize_processesCommandEnd)
{
	TokenizerResult *result = tokenize(string_from("</abc>a"), true, true);
	Token *token;
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL
	       && result->warnings->size.length == 0,
	       "Expected empty vector of warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == 2, "Expected 2 token");
	token = result->result.tokens->items;

	assertToken(1, token, 0, 0, TokenType_COMMAND_END, "abc");
	assertToken(2, token + 1, 6, 6, TokenType_TEXT, "a");
END_TEST}

START_TEST(tokenize_emitsWarningsForInvalidUtf8CharactersButTreatsThemAsText)
{
	TokenizerResult *result =
	    tokenize(string_from("a\370\373\377b"), true, true);
	TokenizerWarning *warning;
	Token *token;
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL
	       && result->warnings->size.length == 3, "Expected 3 warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == 1, "Expected 1 token");
	warning = result->warnings->items;
	token = result->result.tokens->items;

	assertWarning(1, warning, 1, 1,
		      TokenizerWarningCode_INVALID_UTF8_CHARACTER);
	assertWarning(2, warning + 1, 2, 2,
		      TokenizerWarningCode_INVALID_UTF8_CHARACTER);
	assertWarning(3, warning + 2, 3, 3,
		      TokenizerWarningCode_INVALID_UTF8_CHARACTER);

	assertToken(1, token, 0, 0, TokenType_TEXT, "a\370\373\377b");
END_TEST}

START_TEST(tokenize_emitsWarningsForUnexpectedContinuationBytes)
{
	TokenizerResult *result =
	    tokenize(string_from("a\201b\226\300c"), true, true);
	TokenizerWarning *warning;
	Token *token;
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	assert(result->warnings != NULL
	       && result->warnings->size.length == 3, "Expected 3 warnings");
	assert(result->result.tokens != NULL
	       && result->result.tokens->size.length == 1, "Expected 1 token");
	warning = result->warnings->items;
	token = result->result.tokens->items;

	assertWarning(1, warning, 1, 1,
		      TokenizerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE);
	assertWarning(2, warning + 1, 3, 3,
		      TokenizerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE);
	assertWarning(3, warning + 2, 4, 4,
		      TokenizerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE);

	assertToken(1, token, 0, 0, TokenType_TEXT, "a\201b\226\300c");
END_TEST}

START_TEST(tokenize_rejectsCommandStartDelimiterInsideACommand)
{
	TokenizerResult *result = tokenize(string_from("<a<b>"), true, true);
	assert(result != NULL
	       && result->type == TokenizerResultType_ERROR,
	       "Expected an error result");
	assert(result->result.error.byteIndex == 2,
	       "Expected the error's byteIndex to 2");
	assert(result->result.error.codepointIndex == 2,
	       "Expected the error's codepointIndex to be 2");
	assert(result->result.error.code ==
	       TokenizerErrorCode_UNEXPECTED_COMMAND_START,
	       "Expected the error's code to be UNEXPECTED_COMMAND_START");
END_TEST}

START_TEST(tokenize_rejectsUnterminatedCommand)
{
	TokenizerResult *result = tokenize(string_from("<aa"), true, true);
	assert(result != NULL
	       && result->type == TokenizerResultType_ERROR,
	       "Expected an error result");
	assert(result->warnings->size.length == 0,
	       "Expected an empty vector of warnings");
	assert(result->result.error.byteIndex == 3,
	       "Expected the error's byteIndex to be 3");
	assert(result->result.error.codepointIndex == 3,
	       "Expected the errro's codepointIndex to be 3");
	assert(result->result.error.code ==
	       TokenizerErrorCode_UNTERMINATED_COMMAND,
	       "Expected the error's code to be UNTERMINATED_COMMAND");

	result = tokenize(string_from("</ab"), true, true);
	assert(result != NULL
	       && result->type == TokenizerResultType_ERROR,
	       "Expected an error result");
	assert(result->warnings->size.length == 0,
	       "Expected an empty vector of warnings");
	assert(result->result.error.byteIndex == 4,
	       "Expected the error's byteIndex to be 3");
	assert(result->result.error.codepointIndex == 4,
	       "Expected the errro's codepointIndex to be 3");
	assert(result->result.error.code ==
	       TokenizerErrorCode_UNTERMINATED_COMMAND,
	       "Expected the error's code to be UNTERMINATED_COMMAND");
END_TEST}

START_TEST(tokenize_returnsEncounteredWarningsOnError)
{
	TokenizerResult *result =
	    tokenize(string_from("a\226<b<c"), true, true);
	assert(result != NULL
	       && result->type == TokenizerResultType_ERROR,
	       "Expected an error result");
	assert(result->warnings->size.length == 1, "Expected 1 warning");
	assertWarning(1, result->warnings->items, 1, 1,
		      TokenizerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE);
END_TEST}

START_TEST(tokenize_treatsCommandEndDelimiterInTextAsText)
{
	TokenizerResult *result = tokenize(string_from("ab>c"), true, true);
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected a successful result");
	assert(result->warnings->size.length == 0, "Expected no warnings");
	assert(result->result.tokens->size.length == 1, "Expected 1 token");
	assertToken(1, result->result.tokens->items, 0, 0, TokenType_TEXT,
		    "ab>c");
END_TEST}

START_TEST(tokenize_rejectsUnexpectedEncodingEndCommands)
{
	TokenizerResult *result =
	    tokenize(string_from("text</US-ASCII>"), false, false);
	assert(result != NULL
	       && result->type == TokenizerResultType_ERROR,
	       "Expected an error result");
	assert(result->warnings->size.length == 0, "Expected 0 warnings");
	assert(result->result.error.code ==
	       TokenizerErrorCode_ENCODING_STACK_UNDERFLOW,
	       "Expected the ENCODING_STACK_UNDERFLOW error code");

	result =
	    tokenize(string_from
		     ("<US-ASCII>text<ISO-8859-1></US-ASCII></ISO-8859-1>"),
		     false, false);
	assert(result != NULL
	       && result->type == TokenizerResultType_ERROR,
	       "Expected an error result");
	assert(result->warnings->size.length == 0, "Expected 0 warnings");
	assert(result->result.error.code ==
	       TokenizerErrorCode_UNBALANCED_ENCODING_COMMANDS,
	       "Expected the UNBALANCED_ENCODING_COMMANDS error code");
END_TEST}

START_TEST(tokenize_respectsCommandsCaseSensitivity)
{
	TokenizerResult *result =
	    tokenize(string_from("text</us-ascii>"), false, false);
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
END_TEST}

#define assert_token(ordinal, tokenPtr, expectedValue)\
do {\
	char *errorMessage =\
	    malloc(46 + strlen(ordinal) + strlen(expectedValue) + 1);\
	string *valueString = string_from(expectedValue);\
	sprintf(errorMessage,\
		"Expected the %s token to be \"%s\", but was \"%s\"", ordinal,\
		expectedValue, (tokenPtr)->value->content);\
	assert(string_compare((tokenPtr)->value, valueString) == 0,\
	       errorMessage);\
} while (false)

#define assert_nth_token(index, tokenPtr, expectedValue)\
do {\
	char *ordinal = malloc(23);\
	sprintf(ordinal, index == 0 ? "%dst" : index == 1 ? "%dnd" : index ==\
		2 ? "%drd" : "%dth", index + 1);\
	assert_token(ordinal, (tokenPtr) + index, expectedValue);\
} while (false)

START_TEST(tokenize_normalizesInputToUtf8)
{
	char *input =
	    "ASCII text<ISO-8859-1>\240\243<ISO-8859-2>\243</ISO-8859-2>\253</ISO-8859-1><ISO-8859-3>\241</ISO-8859-3><ISO-8859-4>\242</ISO-8859-4><ISO-8859-5>\242</ISO-8859-5>";
	TokenizerResult *result = tokenize(string_from(input), false, false);
	Token *tokens;
	assert(result != NULL
	       && result->type == TokenizerResultType_SUCCESS,
	       "Expected successful result");
	tokens = result->result.tokens->items;

	assert_nth_token(0, tokens, "ASCII");
	assert_nth_token(4, tokens, " ");
	assert_nth_token(5, tokens, "£");
	assert_nth_token(7, tokens, "Ł");
	assert_nth_token(9, tokens, "«");
	assert_nth_token(12, tokens, "Ħ");
	assert_nth_token(15, tokens, "ĸ");
	assert_nth_token(18, tokens, "Ђ");

	input =
	    "<ISO-8859-6>\306</ISO-8859-6><ISO-8859-7>\304</ISO-8859-7><ISO-8859-8>\272</ISO-8859-8><ISO-8859-9>\272</ISO-8859-9><ISO-8859-10>\272</ISO-8859-10>";
	result = tokenize(string_from(input), false, false);
	tokens = result->result.tokens->items;

	assert_nth_token(1, tokens, "ئ");
	assert_nth_token(4, tokens, "Δ");
	assert_nth_token(7, tokens, "÷");
	assert_nth_token(10, tokens, "º");
	assert_nth_token(13, tokens, "š");

	input =
	    "<ISO-8859-11>\277</ISO-8859-11><ISO-8859-13>\277</ISO-8859-13><ISO-8859-14>\277</ISO-8859-14><ISO-8859-15>\277</ISO-8859-15><ISO-8859-16>\277</ISO-8859-16>";
	result = tokenize(string_from(input), false, false);
	tokens = result->result.tokens->items;

	assert_nth_token(1, tokens, "ฟ");
	assert_nth_token(4, tokens, "æ");
	assert_nth_token(7, tokens, "ṡ");
	assert_nth_token(10, tokens, "¿");
	assert_nth_token(13, tokens, "ż");

	input = "😀😃😄😁<ISO-8859-1>\243</ISO-8859-1>😊";
	result = tokenize(string_from(input), false, true);
	tokens = result->result.tokens->items;

	assert_nth_token(0, tokens, "😀😃😄😁");
	assert_nth_token(2, tokens, "£");
	assert_nth_token(4, tokens, "😊");
END_TEST}

#undef assert_nth_token
#undef assert_token

START_TEST(TokenizerResult_free_acceptsNull)
{
	TokenizerResult_free(NULL);
END_TEST}

START_TEST(TokenizerResult_free_freesSuccessfulResult)
{
	TokenizerResult *result = tokenize(string_from("a"), true, true);
	TokenizerResult_free(result);
END_TEST}

static void all_tests()
{
	runTest(tokenize_returnsNullForNullInput);
	runTest(tokenize_processesSingleWord);
	runTest(tokenize_processMultipleWordsSeparatedByRegularSpaces);
	runTest
	    (tokenize_emitsTokensForEveryIndividualAsciiWhitespaceExceptCrLf);
	runTest(tokenize_emitsCrLfInSingleWhitespaceToken);
	runTest(tokenize_recognizesAllTwoByteWhitespaceAsSingleTokens);
	runTest(tokenize_recognizesAllThreeByteWhitespaceAsSingleTokens);
	runTest(tokenize_processesCommandStarts);
	runTest(tokenize_processesCommandEnd);
	runTest
	    (tokenize_emitsWarningsForInvalidUtf8CharactersButTreatsThemAsText);
	runTest(tokenize_emitsWarningsForUnexpectedContinuationBytes);
	runTest(tokenize_rejectsCommandStartDelimiterInsideACommand);
	runTest(tokenize_rejectsUnterminatedCommand);
	runTest(tokenize_returnsEncounteredWarningsOnError);
	runTest(tokenize_treatsCommandEndDelimiterInTextAsText);
	runTest(tokenize_rejectsUnexpectedEncodingEndCommands);
	runTest(tokenize_respectsCommandsCaseSensitivity);
	runTest(tokenize_normalizesInputToUtf8);
	runTest(TokenizerResult_free_acceptsNull);
	runTest(TokenizerResult_free_freesSuccessfulResult);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}

static char *_assertWarning(fileName, lineOfCode, warningOrdinalNumber, warning,
			    byteIndex, codepointIndex, code)
char *fileName;
unsigned int lineOfCode;
unsigned long warningOrdinalNumber;
TokenizerWarning *warning;
unsigned long byteIndex;
unsigned long codepointIndex;
TokenizerWarningCode code;
{
	char *errorMessage;
	char *assertError;
	errorMessage = malloc(sizeof(char) * (56 + 23 + 50 + 50 + 1));
	sprintf(errorMessage,
		"Expected the %lu. warning's byteIndex to be %lu, but was %lu",
		warningOrdinalNumber, byteIndex, warning->byteIndex);
	assertError =
	    unit_assert(fileName, lineOfCode, warning->byteIndex == byteIndex,
			errorMessage);
	if (assertError != NULL) {
		return assertError;
	}
	sprintf(errorMessage,
		"Expected the %lu. warning's codepointIndex to be %lu, but was %lu",
		warningOrdinalNumber, codepointIndex, warning->codepointIndex);
	assertError =
	    unit_assert(fileName, lineOfCode,
			warning->codepointIndex == codepointIndex,
			errorMessage);
	if (assertError != NULL) {
		return assertError;
	}
	sprintf(errorMessage,
		"Expected the %lu. warning's code to be %s, but was %s",
		warningOrdinalNumber, STRINGIFIED_TOKENIZER_WARNING[code],
		STRINGIFIED_TOKENIZER_WARNING[warning->code]);
	return unit_assert(fileName, lineOfCode, warning->code == code,
			   errorMessage);
}

static char *_assertToken(fileName, lineOfCode, tokenOrdinalNumber, token,
			  byteIndex, codepointIndex, type, value)
char *fileName;
unsigned int lineOfCode;
unsigned long tokenOrdinalNumber;
Token *token;
unsigned long byteIndex;
unsigned long codepointIndex;
TokenType type;
char *value;
{
	char *errorMessage;
	char *assertError;
	errorMessage =
	    malloc(sizeof(char) *
		   (54 + 23 + 23 + strlen(value) + token->value->length + 1));
	sprintf(errorMessage,
		"Expected the %lu. token's byteIndex to be %lu, but was %lu",
		tokenOrdinalNumber, byteIndex, token->byteIndex);
	assertError =
	    unit_assert(fileName, lineOfCode, token->byteIndex == byteIndex,
			errorMessage);
	if (assertError != NULL) {
		return assertError;
	}
	sprintf(errorMessage,
		"Expected the %lu. token's codepointIndex to be %lu, but was %lu",
		tokenOrdinalNumber, codepointIndex, token->codepointIndex);
	assertError = unit_assert(fileName, lineOfCode,
				  token->codepointIndex == codepointIndex,
				  errorMessage);
	if (assertError != NULL) {
		return assertError;
	}
	sprintf(errorMessage,
		"Expected the %lu. token's type to be %s, but was %s",
		tokenOrdinalNumber, STRINGIFIED_TOKEN_TYPE[type],
		STRINGIFIED_TOKEN_TYPE[token->type]);
	assertError =
	    unit_assert(fileName, lineOfCode, token->type == type,
			errorMessage);
	if (assertError != NULL) {
		return assertError;
	}
	sprintf(errorMessage,
		"Expected the %lu. token's value to be '%s', but was '%s'",
		tokenOrdinalNumber, value, token->value->content);
	return unit_assert(fileName, lineOfCode,
			   string_compare(token->value,
					  string_from(value)) == 0,
			   errorMessage);
}
