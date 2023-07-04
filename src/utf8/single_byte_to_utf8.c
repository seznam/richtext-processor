#include <stddef.h>
#include "../string.h"
#include "iso_8859_1.h"
#include "iso_8859_2.h"
#include "iso_8859_3.h"
#include "iso_8859_4.h"
#include "iso_8859_5.h"
#include "iso_8859_6.h"
#include "iso_8859_7.h"
#include "iso_8859_8.h"
#include "iso_8859_9.h"
#include "iso_8859_10.h"
#include "iso_8859_11.h"
#include "iso_8859_13.h"
#include "iso_8859_14.h"
#include "iso_8859_15.h"
#include "iso_8859_16.h"
#include "single_byte_to_utf8.h"
#include "us_ascii.h"

string *transcodeSingleByteEncodedTextToUtf8(inputEncoding, input)
SingleByteEncoding inputEncoding;
string *input;
{
	if (input == NULL) {
		return NULL;
	}

	switch (inputEncoding) {
	case SingleByteEncoding_US_ASCII:
		return usAsciiToUtf8(input);
	case SingleByteEncoding_IS_8859_1:
		return iso88591ToUtf8(input);
	case SingleByteEncoding_IS_8859_2:
		return iso88592ToUtf8(input);
	case SingleByteEncoding_IS_8859_3:
		return iso88593ToUtf8(input);
	case SingleByteEncoding_IS_8859_4:
		return iso88594ToUtf8(input);
	case SingleByteEncoding_IS_8859_5:
		return iso88595ToUtf8(input);
	case SingleByteEncoding_IS_8859_6:
		return iso88596ToUtf8(input);
	case SingleByteEncoding_IS_8859_7:
		return iso88597ToUtf8(input);
	case SingleByteEncoding_IS_8859_8:
		return iso88598ToUtf8(input);
	case SingleByteEncoding_IS_8859_9:
		return iso88599ToUtf8(input);
	case SingleByteEncoding_IS_8859_10:
		return iso885910ToUtf8(input);
	case SingleByteEncoding_IS_8859_11:
		return iso885911ToUtf8(input);
	case SingleByteEncoding_IS_8859_13:
		return iso885913ToUtf8(input);
	case SingleByteEncoding_IS_8859_14:
		return iso885914ToUtf8(input);
	case SingleByteEncoding_IS_8859_15:
		return iso885915ToUtf8(input);
	case SingleByteEncoding_IS_8859_16:
		return iso885916ToUtf8(input);
	default:
		return NULL;
	}
}
