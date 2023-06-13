#ifndef JSON_LAYOUT_LINE_SEGMENT_HEADER_FILE
#define JSON_LAYOUT_LINE_SEGMENT_HEADER_FILE 1

#include "../layout_resolver.h"
#include "json_value.h"

JSONValue *LayoutLineSegment_toJSON(LayoutLineSegment * segment);

JSONValue *LayoutLineSegmentVector_toJSON(LayoutLineSegmentVector * segments);

#endif
