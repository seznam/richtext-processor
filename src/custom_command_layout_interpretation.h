#ifndef CUSTOM_COMMAND_LAYOUT_INTERPRETATION_HEADER_FILE
#define CUSTOM_COMMAND_LAYOUT_INTERPRETATION_HEADER_FILE 1

/**
 * Enum representing various ways in which a custom command may be interpreted
 * when determining the abstract layout of a richtext document. These values
 * are returned by a custom command abstract layout interpreter provided to the
 * abstract layout resolver.
 */
typedef enum CustomCommandLayoutInterpretation {
	/*
	 * Can be used for implementing additional content areas, for example an
	 * aside column.
	 */
	CustomCommandLayoutInterpretation_NEW_BLOCK,

	/*
	 * Starts a new implicit paragraph that continues after the causing
	 * command's end. The command's end will not terminate the current line
	 * either (use the <nl> command if desired), but will start a new line
	 * segment.
	 */
	CustomCommandLayoutInterpretation_NEW_PARAGRAPH,

	/*
	 * Starts a new implicit paragraph that ends after the causing command's
	 * end.
	 */
	CustomCommandLayoutInterpretation_NEW_ISOLATED_PARAGRAPH,

	/*
	 * Starts a new line that continues after the causing command's end.
	 * The command's end will start a new line segment.
	 */
	CustomCommandLayoutInterpretation_NEW_LINE,

	/*
	 * Starts a new line that ends afters the causing command's end.
	 */
	CustomCommandLayoutInterpretation_NEW_ISOLATED_LINE,

	/*
	 * Starts a new line segment that ends after the causing command's end.
	 */
	CustomCommandLayoutInterpretation_NEW_LINE_SEGMENT,

	/*
	 * The command will be appended to the current line segment's content.
	 * Note that the content of the command will be processed too, with its
	 * inline content appended to the current line segment's content - this
	 * is to enable processing of line segments' content in a linear
	 * fashion instead of having to traverse more tree structures.
	 */
	CustomCommandLayoutInterpretation_INLINE_CONTENT,

	/*
	 * The command has no effect, layout, formatting or otherwise, and will
	 * be replaced by its children. This is the default behavior for custom
	 * commands if no custom command interpreter is provided.
	 */
	CustomCommandLayoutInterpretation_NO_OP,

	/*
	 * The command is not accepted by the custom command layout interpreter.
	 * This can be used to reject invalid, incorrectly placed, or forbidden
	 * commands, or can be used to more strictly validate the command names.
	 *
	 * Note that implementations should treat all unknown commands as No-op,
	 * and a custom command layout interpreter should conform to specified
	 * behavior as well. Custom command implementors are recommended to use
	 * this interpretation for stricter command name and hierarchy
	 * validation only.
	 */
	CustomCommandLayoutInterpretation_INVALID_COMMAND
} CustomCommandLayoutInterpretation;

#endif
