#ifndef JSON_ENCODER_HEADER_FILE
#define JSON_ENCODER_HEADER_FILE 1

#include "../string.h"
#include "json_value.h"

/*
 * Encoder for JSON values, as defined in RFC 8259. See
 * https://www.rfc-editor.org/rfc/rfc8259 for details.
 */

string *JSON_encode(JSONValue * value);

#endif
