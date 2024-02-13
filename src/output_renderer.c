#include <stdlib.h>
#include "output_renderer.h"
#include "string.h"

void OutputRendererResult_free(result)
OutputRendererResult *result;
{
	if (result == NULL) {
		return;
	}

	if (result->type == OutputRendererResultType_SUCCESS) {
		string_free(result->result.output);
	}
	OutputRendererWarningVector_free(result->warnings);
	free(result);
}
