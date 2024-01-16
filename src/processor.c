#include <stdlib.h>
#include "bool.h"
#include "custom_command_layout_interpretation.h"
#include "layout_post_processor.h"
#include "layout_resolver.h"
#include "output_renderer.h"
#include "parser.h"
#include "processor.h"
#include "tokenizer.h"

ProcessorResult *process(richtext, isUtf8,
			 caseInsensitiveCommands,
			 customCommandInterpreter,
			 layoutPostProcessor,
			 outputRenderer, outputRendererConfiguration)
string *richtext;
bool isUtf8;
bool caseInsensitiveCommands;
CustomCommandLayoutInterpretation customCommandInterpreter(ASTNode *, bool);
LayoutPostProcessor *layoutPostProcessor;
OutputRenderer *outputRenderer;
void *outputRendererConfiguration;
{
	ProcessorResult *result = NULL;
	ProcessorError OOM_POST_PROCESSOR_ERROR =
	    ProcessorError_OUT_OF_MEMORY_FOR_LAYOUT_POST_PROCESSOR_RESULT;
	TokenizerResult *tokenizerResult = NULL;
	ParserResult *parserResult = NULL;
	LayoutResolverResult *layoutResolverResult = NULL;
	LayoutPostProcessorResult *layoutPostProcessorResult = NULL;
	OutputRendererResult *outputRendererResult = NULL;

	result = malloc(sizeof(ProcessorResult));
	if (result == NULL) {
		return NULL;
	}

	result->type = ProcessorResultType_SUCCESS;
	result->result.output = NULL;
	result->tokenizerWarnings = NULL;
	result->layoutResolverWarnings = NULL;
	result->layoutPostProcessorWarnings = NULL;
	result->outputRendererWarnings = NULL;

	if (richtext == NULL) {
		result->type = ProcessorResultType_PROCESSOR_ERROR;
		result->result.processorError =
		    ProcessorError_NULL_INPUT_RICHTEXT;
		return result;
	}
	if (outputRenderer == NULL) {
		result->type = ProcessorResultType_PROCESSOR_ERROR;
		result->result.processorError =
		    ProcessorError_NULL_OUTPUT_RENDERER;
		return result;
	}

	tokenizerResult = tokenize(richtext, caseInsensitiveCommands, isUtf8);
	if (tokenizerResult == NULL) {
		result->type = ProcessorResultType_PROCESSOR_ERROR;
		result->result.processorError =
		    ProcessorError_OUT_OF_MEMORY_FOR_TOKENIZER_RESULT;
		return result;
	}
	result->tokenizerWarnings = tokenizerResult->warnings;
	tokenizerResult->warnings = NULL;	/* prevent early free */
	if (tokenizerResult->type != TokenizerResultType_SUCCESS) {
		result->type = ProcessorResultType_TOKENIZER_ERROR;
		result->result.tokenizerError = tokenizerResult->result.error;
		TokenizerResult_free(tokenizerResult);
		return result;
	}

	parserResult =
	    parse(tokenizerResult->result.tokens, caseInsensitiveCommands);
	if (parserResult == NULL) {
		result->type = ProcessorResultType_PROCESSOR_ERROR;
		result->result.processorError =
		    ProcessorError_OUT_OF_MEMORY_FOR_PARSER_RESULT;
		TokenizerResult_free(tokenizerResult);
		return result;
	}
	if (parserResult->type != ParserResultType_SUCCESS) {
		result->type = ProcessorResultType_PARSER_ERROR;
		result->result.parserError = parserResult->result.error;
		TokenizerResult_free(tokenizerResult);
		ParserResult_free(parserResult);
		return result;
	}

	layoutResolverResult =
	    resolveLayout(parserResult->result.nodes,
			  customCommandInterpreter, caseInsensitiveCommands);
	if (layoutResolverResult == NULL) {
		result->type = ProcessorResultType_PROCESSOR_ERROR;
		result->result.processorError =
		    ProcessorError_OUT_OF_MEMORY_FOR_LAYOUT_RESOLVER_RESULT;
		TokenizerResult_free(tokenizerResult);
		ParserResult_free(parserResult);
		return result;
	}
	result->layoutResolverWarnings = layoutResolverResult->warnings;
	layoutResolverResult->warnings = NULL;	/* prevent early free */
	if (layoutResolverResult->type != LayoutResolverResultType_SUCCESS) {
		result->type = ProcessorResultType_LAYOUT_RESOLVER_ERROR;
		result->result.layoutResolverError =
		    layoutResolverResult->result.error;
		TokenizerResult_free(tokenizerResult);
		ParserResult_free(parserResult);
		LayoutResolverResult_free(layoutResolverResult);
		return result;
	}

	if (layoutPostProcessor != NULL) {
		layoutPostProcessorResult =
		    layoutPostProcessor(layoutResolverResult->result.blocks);
		if (layoutPostProcessorResult == NULL) {
			result->type = ProcessorResultType_PROCESSOR_ERROR;
			result->result.processorError =
			    OOM_POST_PROCESSOR_ERROR;
			TokenizerResult_free(tokenizerResult);
			ParserResult_free(parserResult);
			LayoutResolverResult_free(layoutResolverResult);
			return result;
		}
		result->layoutPostProcessorWarnings =
		    layoutPostProcessorResult->warnings;
		/* prevent early free */
		layoutPostProcessorResult->warnings = NULL;
		if (layoutPostProcessorResult->error != NULL) {
			result->type = ProcessorResultType_POST_PROCESSOR_ERROR;
			result->result.layoutPostProcessorError =
			    *layoutPostProcessorResult->error;
			TokenizerResult_free(tokenizerResult);
			ParserResult_free(parserResult);
			LayoutResolverResult_free(layoutResolverResult);
			LayoutPostProcessorResult_free
			    (layoutPostProcessorResult);
			return result;
		}
		LayoutPostProcessorResult_free(layoutPostProcessorResult);
	}

	outputRendererResult =
	    outputRenderer(layoutResolverResult->result.blocks,
			   outputRendererConfiguration);
	if (outputRendererResult == NULL) {
		result->type = ProcessorResultType_PROCESSOR_ERROR;
		result->result.processorError =
		    ProcessorError_OUT_OF_MEMORY_FOR_OUTPUT_RENDERER_RESULT;
		TokenizerResult_free(tokenizerResult);
		ParserResult_free(parserResult);
		LayoutResolverResult_free(layoutResolverResult);
		return result;
	}
	result->outputRendererWarnings = outputRendererResult->warnings;
	outputRendererResult->warnings = NULL;	/* prevent early free */
	if (outputRendererResult->type != OutputRendererResultType_SUCCESS) {
		result->type = ProcessorResultType_OUTPUT_RENDERER_ERROR;
		result->result.outputRendererError =
		    outputRendererResult->result.error;
		TokenizerResult_free(tokenizerResult);
		ParserResult_free(parserResult);
		LayoutResolverResult_free(layoutResolverResult);
		OutputRendererResult_free(outputRendererResult);
		return result;
	}

	result->result.output = outputRendererResult->result.output;
	outputRendererResult->result.output = NULL;	/* prevent early free */

	/*
	 * We're freeing this near the very end, because the structures refer
	 * each other for easier debugging.
	 */
	TokenizerResult_free(tokenizerResult);
	ParserResult_free(parserResult);
	LayoutResolverResult_free(layoutResolverResult);
	OutputRendererResult_free(outputRendererResult);

	return result;
}
