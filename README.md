# text/richtext processor

Utility for processing formatted text content in `text/richtext` format, as
defined in [RFC 1341](https://datatracker.ietf.org/doc/html/rfc1341), section
7.1.3.

## Differences from the format specification

This implementation, while compatible with the specification, is slightly more
lenient as for what kind of input it tolerates, but also supports a more strict
behavior.

The differences are the following:

- Command names may be of any length and contain any characters except for `<`
  and `>`, while the RFC states that a command name must be up to 40
  characters, all of which must be 'in US-ASCII, restricted to the alphanumeric
  and hyphen ("-") characters' (these constraints can be enforced by providing
  a custom command hook if desired).
- Command names can be processed in a case-sensitive way if desired. The
  default behavior is to treat command names as case-insensitive, so that the
  implementation is compliant with the standard.
