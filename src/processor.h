#ifndef PROCESSOR_HEADER_FILE
#define PROCESSOR_HEADER_FILE 1

#include "ast_node.h"
#include "bool.h"
#include "custom_command_layout_interpretation.h"
#include "layout_post_processor.h"
#include "layout_resolver.h"
#include "output_renderer.h"
#include "parser.h"
#include "string.h"
#include "tokenizer.h"

typedef enum ProcessorResultType {
	ProcessorResultType_SUCCESS,
	ProcessorResultType_TOKENIZER_ERROR,
	ProcessorResultType_PARSER_ERROR,
	ProcessorResultType_LAYOUT_RESOLVER_ERROR,
	ProcessorResultType_POST_PROCESSOR_ERROR,
	ProcessorResultType_OUTPUT_RENDERER_ERROR,
	ProcessorResultType_PROCESSOR_ERROR
} ProcessorResultType;

typedef enum ProcessorError {
	ProcessorError_NULL_INPUT_RICHTEXT,
	ProcessorError_NULL_OUTPUT_RENDERER,
	ProcessorError_OUT_OF_MEMORY_FOR_TOKENIZER_RESULT,
	ProcessorError_OUT_OF_MEMORY_FOR_PARSER_RESULT,
	ProcessorError_OUT_OF_MEMORY_FOR_LAYOUT_RESOLVER_RESULT,
	ProcessorError_OUT_OF_MEMORY_FOR_LAYOUT_POST_PROCESSOR_RESULT,
	ProcessorError_OUT_OF_MEMORY_FOR_OUTPUT_RENDERER_RESULT
} ProcessorError;

typedef struct ProcessorResult {
	ProcessorResultType type;
	union {
		string *output;
		TokenizerError tokenizerError;
		ParserError parserError;
		LayoutResolverError layoutResolverError;
		LayoutPostProcessorError layoutPostProcessorError;
		OutputRendererError outputRendererError;
		ProcessorError processorError;
	} result;
	TokenizerWarningVector *tokenizerWarnings;
	LayoutResolverWarningVector *layoutResolverWarnings;
	LayoutPostProcessorWarningVector *layoutPostProcessorWarnings;
	OutputRendererWarningVector *outputRendererWarnings;
} ProcessorResult;

ProcessorResult *process(string * richtext, bool isUtf8,
			 bool caseInsensitiveCommands,
			 CustomCommandLayoutInterpretation
			 customCommandInterpreter(ASTNode *, bool),
			 LayoutPostProcessor * layoutPostProcessor,
			 OutputRenderer * outputRenderer,
			 void *outputRendererConfiguration);

#endif
