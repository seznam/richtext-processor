#ifndef JSON_LAYOUT_LINE_HEADER_FILE
#define JSON_LAYOUT_LINE_HEADER_FILE 1

#include "../layout_line.h"
#include "../layout_line_vector.h"
#include "json_value.h"

JSONValue *LayoutLine_toJSON(LayoutLine * line);

JSONValue *LayoutLineVector_toJSON(LayoutLineVector * lines);

#endif
