CREATE SCHEMA "rich_text";

CREATE FUNCTION "rich_text"."_rich_text_to_postgresql_encoding"(
  "encoding_name" character varying,
  "case_insensitive" boolean DEFAULT true
)
RETURNS character varying
RETURNS NULL ON NULL INPUT
LANGUAGE plpgsql
AS $$
DECLARE
  "normalized_encoding_name" character varying;
BEGIN
  IF "case_insensitive" THEN
    "normalized_encoding_name" := upper("encoding_name");
  ELSE
    "normalized_encoding_name" := "encoding_name";
  END IF;

  -- See https://www.postgresql.org/docs/current/multibyte.html and
  -- https://datatracker.ietf.org/doc/html/rfc1341.
  CASE
    WHEN "normalized_encoding_name" = 'US-ASCII' THEN
      RETURN 'UTF8';
    WHEN "normalized_encoding_name" ~ '^ISO-8859-[1-49]$' THEN
      RETURN 'ISO8859' || substring("normalized_encoding_name" FROM 10);
    WHEN "normalized_encoding_name" ~ '^ISO-8859-[5-8]$' THEN
      RETURN 'ISO_8859_' || substring("normalized_encoding_name" FROM 10);
    ELSE
      RAISE EXCEPTION
        'A character encoding not supported by text/richtext was provided: %',
        "encoding_name";
  END CASE;
END;
$$;

CREATE FUNCTION "rich_text"."_normalize_encoding"(
  "rich_text" bytea,
  "case_insensitive_commands" boolean DEFAULT true
)
RETURNS character varying
RETURNS NULL ON NULL INPUT
LANGUAGE plpgsql
AS $$
DECLARE
  "COMMAND_START" bytea DEFAULT CAST('<' AS bytea);
  "COMMAND_END" bytea DEFAULT CAST('>' AS bytea);
  "is_command" boolean DEFAULT false;
  "token" bytea DEFAULT '';
  "command" character varying;
  "normalized_command" character varying;
  "chunk" bytea DEFAULT '';
  "result" character varying DEFAULT '';
  "chunk_encoding_stack" character varying[] DEFAULT ARRAY['US-ASCII'];
  "i" integer;
  "current_character" bytea;
BEGIN
  IF pg_client_encoding() <> 'UTF8' THEN
    RAISE EXCEPTION
      'The rich_text._normalize_encoding function can be used only if client encoding is UTF8';
  END IF;

  FOR "i" IN 1..octet_length("rich_text") LOOP
    "current_character" := substring("rich_text" FROM "i" FOR 1);
    CASE "current_character"
      WHEN "COMMAND_START" THEN
        IF "is_command" THEN
          RAISE EXCEPTION
            'Commands cannot contain the < character, found at index %',
            "i";
        END IF;
        "is_command" := true;
        "chunk" := "chunk" || "token";
        "token" := '';
      WHEN "COMMAND_END" THEN
        IF "is_command" THEN
          "is_command" := false;
          -- Preserve all commands including encoding ones to preserve
          -- formatting command location indexes in the source text.
          "chunk" := "chunk" || "COMMAND_START" || "token" || "COMMAND_END";
          "command" := convert_from("token", 'ISO88591');
          IF "case_insensitive_commands" THEN
            "normalized_command" := upper("command");
          ELSE
            "normalized_command" := "command";
          END IF;
          "token" := '';
          CASE
            WHEN "normalized_command" ~ '^(US-ASCII|ISO-8859-[1-9])$' THEN
              "result" :=
                "result" ||
                convert_from(
                  "chunk",
                  "rich_text"."_rich_text_to_postgresql_encoding"(
                    "chunk_encoding_stack"[1],
                    "case_insensitive_commands"
                  )
                );
              "chunk" := '';
              "chunk_encoding_stack" := "command" || "chunk_encoding_stack";
            WHEN "normalized_command" ~ '^/(US-ASCII|ISO-8859-[1-9])$' THEN
              IF array_length("chunk_encoding_stack", 1) = 1 THEN
                RAISE EXCEPTION
                  'The provided text/richtext is not correctly balanced and nested, encountered unexpected <%> at index % with no starting % before it',
                  "command",
                  "i" - character_length("command") - 2,
                  substring("command" FROM 2);
              END IF;
              IF
                '/' || "chunk_encoding_stack"[1] <> "command" AND
                (
                  NOT "case_insensitive_commands" OR
                  upper('/' || "chunk_encoding_stack"[1]) <>
                    "normalized_command"
                )
              THEN
                RAISE EXCEPTION
                  'The provided text/richtext is not correctly nested, encountered unexpected <%> at index % after <%>',
                  "command",
                  "i" - character_length("command") - 2,
                  "chunk_encoding_stack"[1];
              END IF;
              "result" :=
                "result" ||
                convert_from(
                  "chunk",
                  "rich_text"."_rich_text_to_postgresql_encoding"(
                    "chunk_encoding_stack"[1],
                    "case_insensitive_commands"
                  )
                );
              "chunk" := '';
              "chunk_encoding_stack" := "chunk_encoding_stack"[2:];
            ELSE
          END CASE;
        ELSE
          "token" := "token" || "current_character";
        END IF;
      ELSE
        "token" := "token" || "current_character";
    END CASE;
  END LOOP;

  IF array_length("chunk_encoding_stack", 1) > 1 THEN
    RAISE EXCEPTION
      'The provided text/richtext is not correctly balanced, the last encoding command <%> is not terminated',
      "chunk_encoding_stack"[1];
  END IF;

  IF "is_command" THEN
    RAISE EXCEPTION
      'The provided text/richtext is malformed, encountered an unterminated (missing the ">" character) command <%> at index %',
      convert_from("token", 'ISO88591'),
      octet_length("rich_text") - octet_length("token");
  END IF;

  "chunk" := "chunk" || "token";
  "result" :=
    "result" ||
    convert_from(
      "chunk",
      "rich_text"."_rich_text_to_postgresql_encoding"(
        "chunk_encoding_stack"[1],
        "case_insensitive_commands"
      )
    );

  RETURN "result";
END;
$$;

CREATE TYPE "rich_text"."_token_type" AS ENUM (
  'COMMAND_START',
  'COMMAND_END',
  'TEXT',
  'WHITESPACE'
);

CREATE TYPE "rich_text"."_token" AS (
  "index" integer,
  "type" "rich_text"."_token_type",
  "token" character varying
);

CREATE FUNCTION "rich_text"."_tokenize"("rich_text" character varying)
RETURNS SETOF "rich_text"."_token"
RETURNS NULL ON NULL INPUT
LANGUAGE plpgsql
AS $$
DECLARE
  "COMMAND_TOKEN_START" character(1) DEFAULT '<';
  "COMMAND_TOKEN_END" character(1) DEFAULT '>';
  "is_command" boolean DEFAULT false;
  "i" integer;
  "current_character" character varying;
  "token" "rich_text"."_token"
    DEFAULT CAST(ROW(1, 'TEXT', '') AS "rich_text"."_token");
BEGIN
  FOR "i" IN 1..character_length("rich_text") LOOP
    "current_character" := substring("rich_text" FROM "i" FOR 1);
    CASE
      WHEN "current_character" = "COMMAND_TOKEN_START" THEN
        IF "is_command" THEN
          RAISE EXCEPTION
            'Commands cannot contain the < character, found at index %',
            "i";
        END IF;
        "is_command" := true;
        IF ("token")."token" <> '' THEN
          "token"."index" := "i" - character_length(("token")."token");
          RETURN NEXT "token";
          "token"."token" := '';
        END IF;
      WHEN "current_character" = "COMMAND_TOKEN_END" THEN
        IF "is_command" THEN
          "is_command" := false;
          "token"."index" := "i" - character_length(("token")."token") - 1;
          IF substring(("token")."token" FOR 1) = '/' THEN
            "token"."type" := 'COMMAND_END';
            "token"."token" := substring(("token")."token" FROM 2);
          ELSE
            "token"."type" := 'COMMAND_START';
          END IF;
          RETURN NEXT "token";
          "token"."type" := 'TEXT';
          "token"."token" := '';
        ELSE
          "token"."token" := ("token")."token" || "current_character";
        END IF;
      WHEN "current_character" ~ '^[ \r\n\t]$' THEN
        IF ("token")."token" <> '' THEN
          "token"."index" := "i" - character_length(("token")."token");
          RETURN NEXT "token";
        END IF;
        "token"."index" := "i";
        "token"."type" := 'WHITESPACE';
        "token"."token" := "current_character";
        RETURN NEXT "token";
        "token"."type" := 'TEXT';
        "token"."token" := '';
      ELSE
        "token"."token" := ("token")."token" || "current_character";
    END CASE;
  END LOOP;

  IF "is_command" THEN
    RAISE EXCEPTION
      'The provided text/richtext is malformed, encountered an unterminated (missing the ">" character) command <%> at index %',
      ("token")."token",
      character_length("rich_text") - character_length(("token")."token");
  END IF;

  IF ("token")."token" <> '' THEN
    "token"."type" := 'TEXT';
    "token"."index" :=
      character_length("rich_text") - character_length(("token")."token") + 1;
    RETURN NEXT "token";
  END IF;
END;
$$;

CREATE TYPE "rich_text"."node_type" AS ENUM (
  'COMMAND',
  'TEXT',
  'WHITESPACE'
);

CREATE TYPE "rich_text"."node" AS (
  "index" integer,
  "type" "rich_text"."node_type",
  "value" character varying,
  "children" jsonb[]
);

CREATE FUNCTION "rich_text"."_parse"(
  "rich_text" "rich_text"."_token"[],
  "case_insensitive_commands" boolean DEFAULT true
)
RETURNS SETOF "rich_text"."node"
RETURNS NULL ON NULL INPUT
LANGUAGE plpgsql
AS $$
DECLARE
  "command_stack" "rich_text"."node"[]
    DEFAULT CAST(ARRAY[] AS "rich_text"."node"[]);
  "i" integer;
  "token" "rich_text"."_token";
  "normalized_command" character varying;
  "node" "rich_text"."node";
BEGIN
  FOR "i" IN 1..array_length("rich_text", 1) LOOP
    "token" = "rich_text"["i"];
    CASE ("token")."type"
      WHEN 'COMMAND_START' THEN
        "node" := CAST(ROW(
          ("token")."index",
          'COMMAND',
          ("token")."token", CAST(ARRAY[] AS jsonb[])
        ) AS "rich_text"."node");
        IF "case_insensitive_commands" THEN
          "normalized_command" := lower(("token")."token");
        ELSE
          "normalized_command" := ("token")."token";
        END IF;
        IF
          "normalized_command" = 'lt' OR
          "normalized_command" = 'nl' OR
          "normalized_command" = 'np'
        THEN
          IF array_length("command_stack", 1) IS NOT NULL THEN
            "command_stack"[1]."children" :=
              ("command_stack"[1])."children" || to_jsonb("node");
          ELSE
            RETURN NEXT "node";
          END IF;
        ELSE
          "command_stack" := "node" || "command_stack";
        END IF;
      WHEN 'COMMAND_END' THEN
        IF "case_insensitive_commands" THEN
          "normalized_command" := lower(("token")."token");
        ELSE
          "normalized_command" := ("token")."token";
        END IF;
        IF
          "normalized_command" = 'lt' OR
          "normalized_command" = 'nl' OR
          "normalized_command" = 'np'
        THEN
          RAISE EXCEPTION
            'No balancing </%> is allowed by text/richtext, but was encountered at index %',
            ("token")."token",
            ("token")."index";
        END IF;
        IF array_length("command_stack", 1) IS NULL THEN
          RAISE EXCEPTION
            'The provided text/richtext is not correctly balanced and nested, encountered unexpected </%> at index % with no matching <%> before it',
            ("token")."token",
            ("token")."index",
            ("token")."token";
        END IF;
        IF
          ("token")."token" <> ("command_stack"[1])."value" AND
          (
            NOT "case_insensitive_commands" OR
            "normalized_command" <> lower(("command_stack"[1])."value")
          )
        THEN
          RAISE EXCEPTION
            'The provided text/richtext is not correctly balanced and nested, encountered unexpected </%> at index %, but the last started command was % at index %',
            ("token")."token",
            ("token")."index",
            ("command_stack"[1])."value",
            ("command_stack"[1])."index";
        END IF;
        "node" := "command_stack"[1];
        "command_stack" := "command_stack"[2:];
        IF array_length("command_stack", 1) IS NOT NULL THEN
          "command_stack"[1]."children" :=
            ("command_stack"[1])."children" || to_jsonb("node");
        ELSE
          RETURN NEXT "node";
        END IF;
      WHEN 'TEXT', 'WHITESPACE' THEN
        "node" := CAST(
          ROW(
            ("token")."index",
            CAST(("token")."type" AS character varying),
            ("token")."token",
            CAST(ARRAY[] AS jsonb[])
          )
          AS "rich_text"."node"
        );
        IF array_length("command_stack", 1) IS NOT NULL THEN
          "command_stack"[1]."children" :=
            ("command_stack")[1]."children" || to_jsonb("node");
        ELSE
          RETURN NEXT "node";
        END IF;
      ELSE
        RAISE EXCEPTION
          'Encountered an unknown or unsupported token type: %',
          ("token")."type";
    END CASE;
  END LOOP;

  IF array_length("command_stack", 1) IS NOT NULL THEN
    RAISE EXCEPTION
      'The provided text/richtext is not correctly balanced and nested, encountered an unbalanced <%> at index %',
      ("command_stack"[1])."value",
      ("command_stack"[1])."index";
  END IF;
END;
$$;

CREATE TYPE "rich_text"."_command_layout_interpretation" AS ENUM (
   -- The contents of the command will be interpreted as page heading content.
  'HEADING_BLOCK',

  -- The contents of the command will be interpreted as page footing content.
  'FOOTING_BLOCK',

  -- Can be used for implemeting additional content areas, for example an aside
  -- column.
  'NEW_BLOCK',

  -- Move the subsequent content to a new page.
  'NEW_PAGE',

  -- The content of the command should be, if possible, layed out on a single
  -- page.
  'SAME_PAGE',

  -- Starts a new implicit paragraph that continues after the causing command's
  -- end.
  'NEW_IMPLICIT_PARAGRAPH',

  -- Starts a new implicit paragraph that ends after the causing command's end.
  'NEW_ISOLATED_IMPLICIT_PARAGRAPH',

  -- Starts a new line of content.
  'NEW_LINE',

  -- Start a new line that ends afters the causing command's end.
  'NEW_ISOLATED_LINE',

  -- The command will be appended to the current line's content.
  'INLINE_CONTENT',

  -- Same as INLINE_CONTENT, except any children of the command will just be
  -- added to the current line's content with the command itself, and the
  -- children will not have any effect on the computed layout blocks.
  'COMMENT',

  -- The command has no effect, layout, formating or otherwise, and will be
  -- replaced by its children. This is the default behavior for custom commands
  -- if no custom command hook is provided.
  'NO_OP',

  -- The command is not a standard richtext formatting command but a custom
  -- one. Its layout interpretation will be determined by a custom command hook
  -- function, if provided.
  'CUSTOM'
);

CREATE FUNCTION "rich_text"."_get_standard_command_layout_interpretation"(
  "node" "rich_text"."node",
  "case_insensitive_commands" boolean DEFAULT true
)
RETURNS "rich_text"."_command_layout_interpretation"
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT CAST(CASE
    WHEN
      ("node")."value" ~ '^(Bold|Italic|Fixed|Smaller|Bigger|Underline|Subscript|Superscript|Indent|IndentRight|Outdent|OutdentRight|Excerpt|Signature|lt)$' OR
      (
        "case_insensitive_commands" AND
        ("node")."value" ~* '^(Bold|Italic|Fixed|Smaller|Bigger|Underline|Subscript|Superscript|Indent|IndentRight|Outdent|OutdentRight|Excerpt|Signature|lt)$'
      )
    THEN
      'INLINE_CONTENT'
    WHEN
      ("node")."value" ~ '^(Center|FlushLeft|FlushRight|Paragraph)$' OR
      (
        "case_insensitive_commands" AND
        ("node")."value" ~* '^(Center|FlushLeft|FlushRight|Paragraph)$'
      )
    THEN
      'NEW_ISOLATED_IMPLICIT_PARAGRAPH'
    WHEN
      ("node")."value" = 'SamePage' OR
      ("case_insensitive_commands" AND upper(("node")."value") = 'SAMEPAGE')
    THEN
      'SAME_PAGE'
    WHEN
      ("node")."value" = 'Heading' OR
      ("case_insensitive_commands" AND upper(("node")."value") = 'HEADING')
    THEN
      'HEADING_BLOCK'
    WHEN
      ("node")."value" = 'Footing' OR
      ("case_insensitive_commands" AND upper(("node")."value") = 'FOOTING')
    THEN
      'FOOTING_BLOCK'
    WHEN
      ("node")."value" ~ '^(ISO-8859-[1-9]|US-ASCII|No-op)$' OR
      (
        "case_insensitive_commands" AND
        ("node")."value" ~* '^(ISO-8859-[1-9]|US-ASCII|NO-OP)$'
      )
    THEN
      'NO_OP'
    WHEN
      ("node")."value" = 'Comment' OR
      ("case_insensitive_commands" AND upper(("node")."value") = 'COMMENT')
    THEN
      'COMMENT'
    WHEN
      ("node")."value" = 'nl' OR
      ("case_insensitive_commands" AND upper(("node")."value") = 'NL')
    THEN
      'NEW_LINE'
    WHEN
      ("node")."value" = 'np' OR
      ("case_insensitive_commands" AND upper(("node")."value") = 'NP')
    THEN
      'NEW_PAGE'
    ELSE
      'CUSTOM'
  END AS "rich_text"."_command_layout_interpretation");
$$;

CREATE TYPE "rich_text"."custom_command_layout_interpretation" AS ENUM (
  -- Can be used for implemeting additional content areas, for example an aside
  -- column.
  'NEW_BLOCK',

  -- Starts a new implicit paragraph that continues after the causing command's
  -- end.
  'NEW_IMPLICIT_PARAGRAPH',

  -- Starts a new implicit paragraph that ends after the causing command's end.
  'NEW_ISOLATED_IMPLICIT_PARAGRAPH',

  -- Starts a new line that continues after the causing command's end.
  'NEW_LINE',

  -- Start a new line that ends afters the causing command's end.
  'NEW_ISOLATED_LINE',

   -- The command will be appended to the current line's content.
  'INLINE_CONTENT',

  -- The command has no effect, layout, formating or otherwise, and will be
  -- replaced by its children. This is the default behavior for custom commands
  -- if no custom command hook is provided.
  'NO_OP'
);

CREATE FUNCTION "rich_text"."_get_command_layout_interpretation"(
  "node" "rich_text"."node",

  -- Optional callback to execute whenever the processor encounters a custom
  -- command. The callback is specified as a string containing a quoted
  -- function identifier. The function will be invoked with the following
  -- arguments:
  --   - command_node rich_text.node - the richtext node representing the
  --     custom command.
  --   - case_insensitive_commands boolean - the value of the
  --     case_insensitive_commands flag passed to this function.
  -- The function must return a rich_text.custom_command_layout_interpretation
  -- enum constant.
  -- Use an empty string to treat all custom commands as inline content (just
  -- like the No-op command).
  "custom_command_hook" character varying DEFAULT '',

  "case_insensitive_commands" boolean DEFAULT true
)
RETURNS "rich_text"."_command_layout_interpretation"
RETURNS NULL ON NULL INPUT
LANGUAGE plpgsql
AS $$
DECLARE
  "command_interpretation" "rich_text"."_command_layout_interpretation";
  "custom_command_interpretation"
    "rich_text"."custom_command_layout_interpretation";
BEGIN
  "command_interpretation" :=
    "rich_text"."_get_standard_command_layout_interpretation"(
      "node",
      "case_insensitive_commands"
    );
  IF "command_interpretation" = 'CUSTOM' THEN
    IF "custom_command_hook" <> '' THEN
      EXECUTE 'SELECT ' || quote_ident("custom_command_hook") || '($1, $2)'
      INTO STRICT "custom_command_interpretation"
      USING "node", "case_insensitive_commands";
      "command_interpretation" := "custom_command_interpretation";
    ELSE
      "command_interpretation" := 'NO_OP';
    END IF;
  END IF;
  RETURN "command_interpretation";
END;
$$;
