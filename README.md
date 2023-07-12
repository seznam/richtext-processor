# text/richtext processor

Utility for processing formatted text content in `text/richtext` format, as
defined in [RFC 1341](https://www.rfc-editor.org/rfc/rfc1341), section
[7.1.3](https://www.rfc-editor.org/rfc/rfc1341#page-23).

## Differences from the format specification

This implementation, while compatible with the specification, is slightly more
lenient as for what kind of input it tolerates (as recommended by the
specification itself), but also supports a more strict behavior.

### Additional features

- The processor supports the UTF8 encoding, and uses UTF8 internally and in its
  output. The default input encoding can be set to either US-ASCII or UTF8.
  Sections marked by the `ISO-8859-X` and `US-ASCII` commands are fully
  supported, including arbitrary nesting, and will have their contents
  converted to UTF8 during tokenization.

  While the RFC states that 'As of the publication of this document, the
  legitimate values for "X" [in "ISO-8859-X"] are the digits 1 through 9.', all
  [ISO-8859](https://en.wikipedia.org/wiki/ISO/IEC_8859) parts are supported,
  that is including 10, 11, 13, 14, 15 and 16 (The work in making ISO-8859-12
  for Devanagari was officially abandoned in 1997).
- The processor is extensible through custom command processing interpreters,
  enabling easier implementation of extensions to the standard set of
  text/richtext commands.

### Supported (optional) extensions to text/richtext syntax

- Command names may be of any length and contain any characters except for `<`
  and `>`, while the RFC states that a command name must be up to 40
  characters, all of which must be 'in US-ASCII, restricted to the alphanumeric
  and hyphen ("-") characters' (these constraints can be enforced by providing
  a custom command interpreter if desired).
- Command names can be processed in a case-sensitive way if desired. The
  specification-recommended (implied, actually) behavior is to treat command
  names as case-insensitive, and this implementation supports this. When in
  case-sensitive mode, the standard command names are recognized in
  capitalization used in their definitions on pages 23 and 24 of RFC 1341.

  Please note that the C implementation is capable of interpreting only the A-Z
  letters of english alphabet in case-insensitive mode (which is compliant with
  the RFC 1341) due to complexity of turning unicode text to upper/lower case.

### Behavior based on vagueness or ambiguity in the specification

- Comment content, if containing commands, must be well-formed. The
  specification does not state how should the commands insides comments be
  treated and validated.
- While the specification states that 'Those implementations that can do so are
  encouraged to deal reasonably with improperly nested richtext', this
  implementation rejects such content with an error. The specification does not
  state how should such situation be resolved (the two obvious choices are
  terminate all unterminated commands and ignoring extra terminations, and
  preserving the behavior of the command not terminated before terminating its
  ancestor until the command at hand is terminated; and various commands may
  be preferred to be treated differently). Because of this, this implementation
  does not attempt to resolve it and rejects the content. If there is a need
  for accepting unbalanced and improperly nested commands, the recommendation
  is to implement the desired balancing and re-nesting and pass the repaired
  content to the processor.
- The &lt;SamePage> command acts as layout block separator and is processed
  into a pair of blocks marking the start and end of the same-page content. The
  content is therefore placed inside a non-explicit paragraph. This still
  allows the renderer to process it as inline content if desired, even though
  that makes the layout computation lot more complex (requiring possible
  bail-out and backtracking) instead of than treating such content as
  stand-alone content. The renderers are therefore not required to render
  same-page content as continuation of the current line because of this.

## Code portability, compatibility and style

This project aims to maximize portability and compatibility with various
compilers. To accomplish this, it is written in ANSI C (C89) with the
`-Wtraditional` gcc flag set. While this does not guarantee compatibility with
every C compiler out there (the original K&R C compiler, for example), if you
are using an open source, freeware, or free for non-commercial use C compiler
that cannot compile this project, please let us know, we might be able to fix
this.

The project is verified to be compilable in the following compilers:

- gcc 9.4

The code itself uses no dependencies apart from the standard lib C, using only
functions from ANSI C era.

The code style is borrowed from Linux kernel and is enforced using
[`indent`](https://www.gnu.org/software/indent/manual/indent.html).

## Use of assistive AI technologies in this project

This project has been developed without the use of assistive AI technologies,
coding or other. This is primarily due to unresolved (at the time of writing
this) legal concerns regarding any content generated by an AI/ML system.

This can and will be reconsidered as the situation further unfolds.
