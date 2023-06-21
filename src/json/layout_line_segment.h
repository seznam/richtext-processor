#ifndef JSON_LAYOUT_LINE_SEGMENT_HEADER_FILE
#define JSON_LAYOUT_LINE_SEGMENT_HEADER_FILE 1

#include "../layout_line_segment.h"
#include "../layout_line_segment_vector.h"
#include "json_value.h"

JSONValue *LayoutLineSegment_toJSON(LayoutLineSegment * segment);

JSONValue *LayoutLineSegmentVector_toJSON(LayoutLineSegmentVector * segments);

#endif
