#include <stdlib.h>
#include "layout_post_processor.h"

void LayoutPostProcessorResult_free(result)
LayoutPostProcessorResult * result;
{
	if (result == NULL) {
		return;
	}

	if (result->error != NULL) {
		/* The referenced structures are not ours. */
		free(result->error);
	}
	LayoutPostProcessorWarningVector_free(result->warnings);
	free(result);
}
