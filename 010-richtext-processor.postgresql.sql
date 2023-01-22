CREATE SCHEMA "richtext";

CREATE FUNCTION "richtext"."_richtext_to_postgresql_encoding"(
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

CREATE FUNCTION "richtext"."_normalize_encoding"(
  "richtext" bytea,
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
      'The richtext._normalize_encoding function can be used only if client encoding is UTF8';
  END IF;

  FOR "i" IN 1..octet_length("richtext") LOOP
    "current_character" := substring("richtext" FROM "i" FOR 1);
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
                  "richtext"."_richtext_to_postgresql_encoding"(
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
                  "richtext"."_richtext_to_postgresql_encoding"(
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
      octet_length("richtext") - octet_length("token");
  END IF;

  "chunk" := "chunk" || "token";
  "result" :=
    "result" ||
    convert_from(
      "chunk",
      "richtext"."_richtext_to_postgresql_encoding"(
        "chunk_encoding_stack"[1],
        "case_insensitive_commands"
      )
    );

  RETURN "result";
END;
$$;

CREATE TYPE "richtext"."_token_type" AS ENUM (
  'COMMAND_START',
  'COMMAND_END',
  'TEXT',
  'WHITESPACE'
);

CREATE TYPE "richtext"."_token" AS (
  "index" integer,
  "type" "richtext"."_token_type",
  "token" character varying
);

CREATE FUNCTION "richtext"."_tokenize"("richtext" character varying)
RETURNS SETOF "richtext"."_token"
RETURNS NULL ON NULL INPUT
LANGUAGE plpgsql
AS $$
DECLARE
  "COMMAND_TOKEN_START" character(1) DEFAULT '<';
  "COMMAND_TOKEN_END" character(1) DEFAULT '>';
  "is_command" boolean DEFAULT false;
  "i" integer;
  "current_character" character varying;
  "token" "richtext"."_token"
    DEFAULT CAST(ROW(1, 'TEXT', '') AS "richtext"."_token");
BEGIN
  FOR "i" IN 1..character_length("richtext") LOOP
    "current_character" := substring("richtext" FROM "i" FOR 1);
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
      character_length("richtext") - character_length(("token")."token");
  END IF;

  IF ("token")."token" <> '' THEN
    "token"."type" := 'TEXT';
    "token"."index" :=
      character_length("richtext") - character_length(("token")."token") + 1;
    RETURN NEXT "token";
  END IF;
END;
$$;

CREATE TYPE "richtext"."node_type" AS ENUM (
  'COMMAND',
  'TEXT',
  'WHITESPACE'
);

CREATE TYPE "richtext"."node" AS (
  "index" integer,
  "type" "richtext"."node_type",
  "value" character varying,
  "children" jsonb[]
);

CREATE FUNCTION "richtext"."_parse"(
  "richtext" "richtext"."_token"[],
  "case_insensitive_commands" boolean DEFAULT true
)
RETURNS SETOF "richtext"."node"
RETURNS NULL ON NULL INPUT
LANGUAGE plpgsql
AS $$
DECLARE
  "command_stack" "richtext"."node"[]
    DEFAULT CAST(ARRAY[] AS "richtext"."node"[]);
  "i" integer;
  "token" "richtext"."_token";
  "normalized_command" character varying;
  "node" "richtext"."node";
BEGIN
  FOR "i" IN 1..array_length("richtext", 1) LOOP
    "token" = "richtext"["i"];
    CASE ("token")."type"
      WHEN 'COMMAND_START' THEN
        "node" := CAST(ROW(
          ("token")."index",
          'COMMAND',
          ("token")."token", CAST(ARRAY[] AS jsonb[])
        ) AS "richtext"."node");
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
          AS "richtext"."node"
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

CREATE TYPE "richtext"."_command_layout_interpretation" AS ENUM (
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

CREATE FUNCTION "richtext"."_get_standard_command_layout_interpretation"(
  "node" "richtext"."node",
  "case_insensitive_commands" boolean DEFAULT true
)
RETURNS "richtext"."_command_layout_interpretation"
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT CAST(CASE
    WHEN
      ("node")."value" ~ '^(Bold|Italic|Fixed|Smaller|Bigger|Underline|Subscript|Superscript|Center|FlushLeft|FlushRight|Indent|IndentRight|Outdent|OutdentRight|Excerpt|Signature|lt)$' OR
      (
        "case_insensitive_commands" AND
        ("node")."value" ~* '^(Bold|Italic|Fixed|Smaller|Bigger|Underline|Subscript|Superscript|Center|FlushLeft|FlushRight|Indent|IndentRight|Outdent|OutdentRight|Excerpt|Signature|lt)$'
      )
    THEN
      'INLINE_CONTENT'
    WHEN
      ("node")."value" = 'Paragraph' OR
      ("case_insensitive_commands" AND upper(("node")."value") = 'PARAGRAPH')
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
  END AS "richtext"."_command_layout_interpretation");
$$;

CREATE TYPE "richtext"."custom_command_layout_interpretation" AS ENUM (
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

CREATE FUNCTION "richtext"."_get_command_layout_interpretation"(
  "node" "richtext"."node",

  -- Optional callback to execute whenever the processor encounters a custom
  -- command. The callback is specified as a string containing a quoted
  -- function identifier. The function will be invoked with the following
  -- arguments:
  --   - command_node richtext.node - the richtext node representing the
  --     custom command.
  --   - case_insensitive_commands boolean - the value of the
  --     case_insensitive_commands flag passed to this function.
  -- The function must return a richtext.custom_command_layout_interpretation
  -- enum constant.
  -- Use an empty string to treat all custom commands as inline content (just
  -- like the No-op command).
  "custom_command_hook" character varying DEFAULT '',

  "case_insensitive_commands" boolean DEFAULT true
)
RETURNS "richtext"."_command_layout_interpretation"
RETURNS NULL ON NULL INPUT
LANGUAGE plpgsql
AS $$
DECLARE
  "command_interpretation" "richtext"."_command_layout_interpretation";
  "custom_command_interpretation"
    "richtext"."custom_command_layout_interpretation";
BEGIN
  "command_interpretation" :=
    "richtext"."_get_standard_command_layout_interpretation"(
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

CREATE FUNCTION "richtext"."_deserialize_node_children"(
  "node" "richtext"."node"
)
RETURNS "richtext"."node"[]
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT array_agg("child")
  FROM (
    SELECT jsonb_populate_record(
      CAST(NULL AS "richtext"."node"),
      unnest(("node")."children")
    ) AS "child"
  ) AS "children";
$$;

CREATE TYPE "richtext"."layout_block_paragraph_line" AS (
  "index" integer,

  "causing_command" "richtext"."node",

  -- May contain text, whitespace and the following commands:
  --   - Content representing commands: lt
  --   - Inline formatting commands: Bold, Italic, Fixed, Smaller, Bigger,
  --     Underline, Subscript, Superscript
  --   - Alignment commands: Center, FlushLeft, FlushRight, Indent,
  --     IndentRight, Outdent, OutdentRight
  --   - Styling commands: Excerpt, Signature
  --   - Metadata commands: Comment
  --   - Any custom extensions treated as inline content
  "content" "richtext"."node"[]
);

CREATE TYPE "richtext"."layout_block_paragraph" AS (
  "index" integer,

  "causing_command" "richtext"."node",

  -- Only true for the <Paragraph> command and after </Paragraph> if nested
  -- within another <Paragraph>, or custom command representing isolated
  -- paragraph.
  "is_explicit_paragraph" boolean,

  -- Lines are paragraph fragments explicitly separated by the <nl> command and
  -- do not neccessary map 1:1 to rows in rendered document, but every line
  -- should be rendered as one or more rows of content, while not merging
  -- multiple lines into a single row.
  "lines" "richtext"."layout_block_paragraph_line"[]
);

CREATE TYPE "richtext"."layout_block_type" AS ENUM (
  'HEADING',

  'FOOTING',

  'MAIN_CONTENT',

  'PAGE_BREAK',

  'SAME_PAGE_START',

  'SAME_PAGE_END',

  -- Caused only by custom commands for which the processing custom command
  -- hook returns 'NEW_BLOCK'.
  'CUSTOM'
);

CREATE TYPE "richtext"."layout_block" AS (
  "index" integer,
  "type" "richtext"."layout_block_type",
  "causing_command" "richtext"."node",
  "paragraphs" "richtext"."layout_block_paragraph"[]
);

CREATE FUNCTION "richtext"."_complete_paragraph_line_node"(
  "node_stack" "richtext"."node"[]
)
RETURNS "richtext"."node"
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT CASE
    WHEN array_length("node_stack", 1) IS NULL THEN
      NULL
    WHEN array_length("node_stack", 1) = 1 THEN
      "node_stack"[1]
    ELSE -- Recursivelly process the node stack into the root node
      "richtext"."_complete_paragraph_line_node"(CAST(ROW(
        ("node_stack"[2])."index",
        ("node_stack"[2])."type",
        ("node_stack"[2])."value",
        ("node_stack"[2])."children" || to_jsonb("node_stack"[1])
      ) AS "richtext"."node") || "node_stack"[3:])
  END;
$$;

CREATE FUNCTION "richtext"."_complete_paragraph_line"(
  "line" "richtext"."layout_block_paragraph_line",
  "content" "richtext"."node"[]
)
RETURNS "richtext"."layout_block_paragraph_line"
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT CAST(
    ROW(
      ("line")."index",
      ("line")."causing_command",
      ("line")."content" || "content"
    ) AS "richtext"."layout_block_paragraph_line"
  );
$$;

CREATE FUNCTION "richtext"."_complete_paragraph"(
  "paragraph" "richtext"."layout_block_paragraph",
  "line" "richtext"."layout_block_paragraph_line"
)
RETURNS "richtext"."layout_block_paragraph"
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT CASE
    WHEN array_length(("line")."content", 1) IS NOT NULL THEN
      CAST(
        ROW(
          "paragraph"."index",
          "paragraph"."causing_command",
          "paragraph"."content_alignment",
          "paragraph"."is_explicit_paragraph",
          "paragraph"."lines" || "line"
        ) AS "richtext"."layout_block_paragraph"
      )
    ELSE
      "paragraph"
  END;
$$;

CREATE FUNCTION "richtext"."_complete_paragraph"(
  "paragraph" "richtext"."layout_block_paragraph",
  "line" "richtext"."layout_block_paragraph_line",
  "content" "richtext"."node"[]
)
RETURNS "richtext"."layout_block_paragraph"
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT "richtext"."_complete_paragraph"(
    "paragraph",
    "richtext"."_complete_paragraph_line"("line", "content")
  );
$$;

CREATE FUNCTION "richtext"."_complete_layout_block"(
  "block" "richtext"."layout_block",
  "paragraph" "richtext"."layout_block_paragraph"
)
RETURNS "richtext"."layout_block"
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT CASE
    WHEN array_length(("paragraph")."lines", 1) IS NOT NULL THEN
      CAST(
        ROW(
          "block"."index",
          "block"."type",
          "block"."causing_command",
          "block"."paragraphs" || "paragraph"
        ) AS "richtext"."layout_block"
      )
    ELSE
      "block"
  END;
$$;

CREATE FUNCTION "richtext"."_complete_layout_block"(
  "block" "richtext"."layout_block",
  "paragraph" "richtext"."layout_block_paragraph",
  "line" "richtext"."layout_block_paragraph_line",
  "content" "richtext"."node"[]
)
RETURNS "richtext"."layout_block"
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT "richtext"."_complete_layout_block"(
    "block",
    "richtext"."_complete_paragraph"("paragraph", "line", "content")
  );
$$;

CREATE FUNCTION "richtext"."_complete_layout_block"(
  "block" "richtext"."layout_block",
  "paragraph" "richtext"."layout_block_paragraph",
  "line" "richtext"."layout_block_paragraph_line",
  "content" "richtext"."node"[],
  "last_content_node_stack" "richtext"."node"[]
)
RETURNS "richtext"."layout_block"
RETURNS NULL ON NULL INPUT
LANGUAGE SQL
AS $$
  SELECT "richtext"."_complete_layout_block"(
    "block",
    "paragraph",
    "line",
    "content" ||
      "richtext"."_complete_paragraph_line_node"("last_content_node_stack")
  );
$$;

-- Processes the provided parsed text/richtext into layouting blocks, composed
-- of paragraphs, composed of lines. Any formatting commands that span multiple
-- lines will be re-organized to be present in all affected lines with the same
-- effect, so that formatting state does not have to be tracked across lines,
-- paragraphs or blocks.
-- The function also replaces the ISO-8859-X, US-ASCII and No-op commands with
-- their content.
CREATE FUNCTION "richtext"."_resolve_layout_blocks"(
  "richtext" "richtext"."node"[],

  -- Optional callback to execute whenever the processor encounters a custom
  -- command. The callback is specified as a string containing a quoted
  -- function identifier. The function will be invoked with the following
  -- arguments:
  --   - command_node richtext.node - the richtext node representing the
  --     custom command.
  --   - case_insensitive_commands boolean - the value of the
  --     case_insensitive_commands flag passed to this function.
  -- The function must return a richtext.custom_command_layout_interpretation
  -- enum constant.
  -- Use an empty string to treat all custom commands as inline content (just
  -- like the No-op command).
  "custom_command_hook" character varying DEFAULT '',

  "case_insensitive_commands" boolean DEFAULT true
)
RETURNS SETOF "richtext"."layout_block"
RETURNS NULL ON NULL INPUT
LANGUAGE plpgsql
AS $$
DECLARE
  "block_type_stack" "richtext"."layout_block_type"[]
    DEFAULT CAST(ARRAY['MAIN_CONTENT'] AS "richtext"."layout_block_type"[]);
  -- stack of nodes which we nested into while traversing the input
  "command_stack" "richtext"."node"[][]
    DEFAULT CAST(ARRAY[] AS "richtext"."node"[][]);
  -- stack of current formatting nodes, used for copying currently applied
  -- formatting to the next line
  "formatting_stack" "richtext"."node"[]
    DEFAULT CAST(ARRAY[] AS "richtext"."node"[]);
  -- sibling nodes at the current level of the input node forest traversing
  "current_level" "richtext"."node"[] DEFAULT "richtext";
  -- the currently processed node
  "node" "richtext"."node";
  -- layout interpretation of the currently processed node
  "command_layout_interpretation" "richtext"."_command_layout_interpretation";
  -- currently constructed layout block
  "block" "richtext"."layout_block" DEFAULT CAST(ROW(
    1,
    'MAIN_CONTENT',
    NULL,
    CAST(ARRAY[] AS "richtext"."layout_block_paragraph"[])
  ) AS "richtext"."layout_block");
  -- currently constructed layout block's paragraph
  "paragraph" "richtext"."layout_block_paragraph"
    DEFAULT CAST(ROW(
      1,
      NULL,
      false,
      CAST(ARRAY[] AS "richtext"."layout_block_paragraph_line"[])
    ) AS "richtext"."layout_block_paragraph");
  -- currently constructed layout block's paragraph's line
  "line" "richtext"."layout_block_paragraph_line" DEFAULT CAST(
    ROW(1, NULL, CAST(ARRAY[] AS "richtext"."node"[]))
    AS "richtext"."layout_block_paragraph_line"
  );
  -- currently constructed layout block's paragraph's line's content
  "line_content" "richtext"."node"[]
    DEFAULT CAST(ARRAY[] AS "richtext"."node"[]);
  -- node stack of currently constructed line content's node
  "line_content_command_stack" "richtext"."node"[]
    DEFAULT CAST(ARRAY[] AS "richtext"."node"[]);
BEGIN
  WHILE array_length("current_level", 1) IS NOT NULL LOOP
    -- Traversing through and down into the node tree
    "node" := "current_level"[1];
    CASE ("node")."type"
      WHEN 'COMMAND' THEN
        "command_layout_interpretation" :=
          "richtext"."_get_command_layout_interpretation"(
            "node",
            "custom_command_hook",
            "case_insensitive_commands"
          );
        CASE "command_layout_interpretation"
          WHEN 'HEADING_BLOCK', 'FOOTING_BLOCK', 'NEW_BLOCK' THEN
            "block" := "richtext"."_complete_layout_block"(
              "block",
              "paragraph",
              "line",
              "line_content",
              "line_content_command_stack"
            );
            IF array_length(("block")."paragraphs", 1) IS NOT NULL THEN
              RETURN NEXT "block";
            END IF;
            "block" := CAST(
              ROW(
                ("node")."index",
                CASE
                  WHEN "command_layout_interpretation" = 'HEADING_BLOCK' THEN
                    'HEADING'
                  WHEN "command_layout_interpretation" = 'FOOTING_BLOCK' THEN
                    'FOOTING'
                  WHEN "command_layout_interpretation" = 'NEW_BLOCK' THEN
                    'CUSTOM'
                END,
                "node",
                CAST(ARRAY[] AS "richtext"."layout_block_paragraph"[])
              ) AS "richtext"."layout_block"
            );
            "block_type_stack" := ("block")."type" || "block_type_stack";
            "paragraph" := CAST(ROW(
              ("node")."index",
              "node",
              false,
              CAST(ARRAY[] AS "richtext"."layout_block_paragraph_line")
            ) AS "richtext"."layout_block_paragraph");
            "line" := CAST(ROW(
              ("node")."index",
              "node",
              CAST(ARRAY[] AS "richtext"."node"[])
            ) AS "richtext"."layout_block_paragraph_line");
            "line_content" := CAST(ARRAY[] AS "richtext"."node"[]);
            "line_content_command_stack" := "formatting_stack";
            "current_level" := "current_level"[2:];
          WHEN 'NEW_PAGE' THEN
            -- The <np> command is allowed in any other command, including
            -- <Heading> and <Footing>. So, we'll do our best to do both.
            "block" := "richtext"."_complete_layout_block"(
              "block",
              "paragraph",
              "line",
              "line_content",
              "line_content_command_stack"
            );
            IF array_length(("block")."paragraphs", 1) IS NOT NULL THEN
              RETURN NEXT "block";
            END IF;
            RETURN NEXT CAST(ROW(
              ("node")."index",
              'PAGE_BREAK',
              "node",
              CAST(ARRAY[] AS "richtext"."layout_block_paragraph"[])
            ) AS "richtext"."layout_block");
            -- We only need to clear the paragraphs, as the next block is
            -- caused by the same previous block-causing command node.
            "block"."paragraphs" := CAST(
              ARRAY[] AS "richtext"."layout_block_paragraph"[]
            );
            "paragraph" := CAST(ROW(
              ("node")."index",
              "node",
              false,
              CAST(ARRAY[] AS "richtext"."layout_block_paragraph_line"[])
            ) AS "richtext"."layout_block_paragraph");
            "line" := CAST(ROW(
              ("node")."index",
              "node",
              CAST(ARRAY[] AS "richtext"."node"[])
            ) AS "richtext"."layout_block_paragraph_line");
            "line_content" := CAST(ARRAY[] AS "richtext"."node"[]);
            "line_content_command_stack" := "formatting_stack";
            "current_level" := "current_level"[2:];
          WHEN 'SAME_PAGE' THEN
            "block" := "richtext"."_complete_layout_block"(
              "block",
              "paragraph",
              "line",
              "line_content",
              "line_content_command_stack"
            );
            IF array_length(("block")."paragraphs", 1) IS NOT NULL THEN
              RETURN NEXT "block";
            END IF;
            IF array_length(("node")."children", 1) IS NOT NULL THEN
              -- There is no point in emmitting the SAME_PAGE_START block if
              -- there is no content in it.
              RETURN NEXT CAST(ROW(
                ("node")."index",
                'SAME_PAGE_START',
                "node",
                CAST(ARRAY[] AS "richtext"."layout_block_paragraph"[])
              ) AS "richtext"."layout_block");
              "command_stack" := "current_level" || "command_stack";
              "current_level" :=
                "richtext"."_deserialize_node_children"("node");
            ELSE
              "current_level" := "current_level"[2:];
            END IF;
            "block" := CAST(ROW(
              ("node")."index",
              ("block")."type",
              "node",
              CAST(ARRAY[] AS "richtext"."layout_block_paragraph"[])
            ) AS "richtext"."layout_block");
            "paragraph" := CAST(ROW(
              ("node")."index",
              "node",
              false,
              CAST(ARRAY[] AS "richtext"."layout_block_paragraph_line"[])
            ) AS "richtext"."layout_block_paragraph");
            "line" := CAST(ROW(
              ("node")."index",
              "node",
              CAST(ARRAY[] AS "richtext"."node"[])
            ) AS "richtext"."layout_block_paragraph_line");
            "line_content" := CAST(ARRAY[] AS "richtext"."node"[]);
            "line_content_command_stack" := "formatting_stack";
          WHEN 'NEW_IMPLICIT_PARAGRAPH', 'NEW_ISOLATED_IMPLICIT_PARAGRAPH' THEN
            "paragraph" := "richtext"."_complete_paragraph"(
              "paragraph",
              "line",
              "line_content" || "richtext"."_complete_paragraph_line_node"(
                "line_content_command_stack"
              )
            );
            "block"."paragraphs" := "block"."paragraphs" || "paragraph";
            "paragraph" := CAST(ROW(
              ("node")."index",
              "node",
              "command_layout_interpretation" =
                'NEW_ISOLATED_IMPLICIT_PARAGRAPH',
              CAST(ARRAY[] AS "richtext"."layout_block_paragraph_line"[])
            ) AS "richtext"."layout_block_paragraph");
            "line" := CAST(ROW(
              ("node")."index",
              "node",
              CAST(ARRAY[] AS "richtext"."node"[])
            ) AS "richtext"."layout_block_paragraph_line");
            "line_content" := CAST(ARRAY[] AS "richtext"."node"[]);
            "line_content_command_stack" := "formatting_stack";
            "command_stack" := "current_level" || "command_stack";
            "current_level" := "richtext"."_deserialize_node_children"("node");
          WHEN 'NEW_LINE', 'NEW_ISOLATED_LINE' THEN
            "paragraph" := "richtext"."_complete_paragraph"(
              "paragraph",
              "line",
              "line_content" || "richtext"."_complete_paragraph_line_node"(
                "line_content_command_stack"
              )
            );
            "line" := CAST(ROW(
              ("node")."index",
              "node",
              CAST(ARRAY[] AS "richtext"."node"[])
            ) AS "richtext"."layout_block_paragraph_line");
            "line_content" := CAST(ARRAY[] AS "richtext"."node"[]);
            "line_content_command_stack" := "formatting_stack";
            "command_stack" := "current_level" || "command_stack";
            "current_level" := "richtext"."_deserialize_node_children"("node");
          WHEN 'INLINE_CONTENT' THEN
            "line_content_command_stack" :=
              "node" || "line_content_command_stack";
            IF array_length(("node")."children", 1) IS NOT NULL THEN
              "command_stack" := "current_level" || "command_stack";
              -- The only command resolved as INLINE_CONTENT that should not be
              -- put on the formatting stack is the <lt> command, which never
              -- has any children, so there is no need for additional condition
              -- here.
              "formatting_stack" := CAST(ROW(
                ("node")."index",
                ("node")."type",
                ("node")."value",
                CAST(ARRAY[] AS jsonb[])
              ) AS "richtext"."node") || "formatting_stack";
              "current_level" :=
                "richtext"."_deserialize_node_children"("node");
            ELSE
              "current_level" := "current_level"[2:];
            END IF;
          WHEN 'COMMENT' THEN
            IF array_length("line_content_command_stack", 1) IS NOT NULL THEN
              "line_content_command_stack"[1]."children" :=
                "line_content_command_stack"[1]."children" || to_jsonb("node");
            ELSE
              "line_content" := "line_content" || "node";
            END IF;
            "current_level" := "current_level"[2:];
          WHEN 'NO_OP' THEN
            "current_level" :=
              "richtext"."_deserialize_node_children"("node") ||
              "current_level"[2:];
          ELSE
            RAISE EXCEPTION
              'Encountered an unknown, unsupported or unexpected command layout interpretation % (command % parsed at index %)',
              "command_layout_interpretation",
              ("node")."type",
              ("node")."index";
        END CASE;
      WHEN 'TEXT', 'WHITESPACE' THEN
        IF array_length("line_content_command_stack", 1) IS NOT NULL THEN
          "line_content_command_stack"[1]."children" =
            "line_content_command_stack"[1]."children" || to_jsonb("node");
        ELSE
          "line_content" := "line_content" || "node";
        END IF;
        "current_level" := "current_level"[2:];
      ELSE
        RAISE EXCEPTION
          'Encountered an unknown or unsupported node type % (parsed at index %)',
          ("node")."type",
          ("node")."index";
    END CASE;

    -- Traversing to the next sibling node or the parent in the node tree.
    WHILE
      array_length("current_level", 1) IS NULL AND
      array_length("command_stack", 1) IS NOT NULL
    LOOP
      "current_level" := "command_stack"[1];
      "command_stack" := "command_stack"[2:];
      "node" := "current_level"[1];
      "current_level" := "current_level"[2:];
      "command_layout_interpretation" :=
        "richtext"."_get_command_layout_interpretation"(
          "node",
          "custom_command_hook",
          "case_insensitive_commands"
        );
      CASE "command_layout_interpretation"
        WHEN 'HEADING_BLOCK', 'FOOTING_BLOCK', 'NEW_BLOCK' THEN
          "block" := "richtext"."_complete_layout_block"(
            "block",
            "paragraph",
            "line",
            "line_content",
            "line_content_command_stack"
          );
          IF array_length(("block")."paragraphs", 1) IS NOT NULL THEN
            RETURN NEXT "block";
          END IF;
          "block_type_stack" := "block_type_stack"[2:];
          "block" := CAST(ROW(
            ("node")."index",
            "block_type_stack"[1],
            "node",
            CAST(ARRAY[] AS "richtext"."layout_block_paragraph"[])
          ) AS "richtext"."layout_block");
          "paragraph" := CAST(ROW(
            ("node")."index",
            "node",
            false,
            CAST(ARRAY[] AS "richtext"."layout_block_paragraph_line"[])
          ) AS "richtext"."layout_block_paragraph");
          "line" := CAST(ROW(
            ("node")."index",
            "node",
            CAST(ARRAY[] AS "richtext"."node"[])
          ) AS "richtext"."layout_block_paragraph_line");
          "line_content" := CAST(ARRAY[] AS "richtext"."node"[]);
          "line_content_command_stack" := "formatting_stack";
        WHEN 'SAME_PAGE' THEN
          "block" := "richtext"."_complete_layout_block"(
            "block",
            "paragraph",
            "line",
            "line_content",
            "line_content_command_stack"
          );
          IF array_length(("block")."paragraphs", 1) IS NOT NULL THEN
            RETURN NEXT "block";
          END IF;
          RETURN NEXT CAST(ROW(
            ("node")."index",
            'SAME_PAGE_END',
            "node",
            CAST(ARRAY[] AS "richtext"."layout_block_paragraph"[])
          ) AS "richtext"."layout_block");
          "block" := CAST(ROW(
            ("node")."index",
            "block_type_stack"[1],
            "node",
            CAST(ARRAY[] AS "richtext"."layout_block_paragraph"[])
          ) AS "richtext"."layout_block");
          "paragraph" := CAST(ROW(
            ("node")."index",
            "node",
            false,
            CAST(ARRAY[] AS "richtext"."layout_block_paragraph_line"[])
          ) AS "richtext"."layout_block_paragraph");
          "line" := CAST(ROW(
            ("node")."index",
            "node",
            CAST(ARRAY[] AS "richtext"."node"[])
          ) AS "richtext"."layout_block_paragraph_line");
          "line_content" := CAST(ARRAY[] AS "richtext"."node"[]);
          "line_content_command_stack" := "formatting_stack";
        WHEN 'NEW_IMPLICIT_PARAGRAPH' THEN
          NULL; -- Nothing to do.
        WHEN 'NEW_ISOLATED_IMPLICIT_PARAGRAPH' THEN
          "paragraph" := "richtext"."_complete_paragraph"(
            "paragraph",
            "line",
            "line_content" || "richtext"."_complete_paragraph_line_node"(
              "line_content_command_stack"
            )
          );
          "block"."paragraphs" := ("block")."paragraphs" || "paragraph";
          "paragraph" := CAST(ROW(
            ("node")."index",
            "node",
            EXISTS( -- Are we inside a <Paragraph>?
              SELECT 1
              FROM unnest("command_stack"[:][1])
              WHERE "value" = 'Paragraph' OR
                ("case_insensitive_commands" AND "value" ~* '^Paragraph$')
            ),
            CAST(ARRAY[] AS "richtext"."layout_block_paragraph_line"[])
          ) AS "richtext"."layout_block_paragraph");
          "line" := CAST(ROW(
            ("node")."index",
            "node",
            CAST(ARRAY[] AS "richtext"."node"[])
          ) AS "richtext"."layout_block_paragraph_line");
          "line_content" := CAST(ARRAY[] AS "richtext"."node"[]);
          "line_content_command_stack" := "formatting_stack";
        WHEN 'NEW_LINE' THEN
          NULL; -- Nothing to do.
        WHEN 'NEW_ISOLATED_LINE' THEN
          "line" := "richtext"."_complete_paragraph_line"(
            "line",
            "line_content" || "richtext"."_complete_paragraph_line_node"(
              "line_content_command_stack"
            )
          );
          "paragraph"."lines" := ("paragraph")."lines" || "line";
          "line" := CAST(ROW((
            "node")."index",
            "node",
            CAST(ARRAY[] AS "richtext"."node"[])
          ) AS "richtext"."layout_block_paragraph_line");
          "line_content" := CAST(ARRAY[] AS "richtext"."node"[]);
          "line_content_command_stack" := "formatting_stack";
        WHEN 'INLINE_CONTENT' THEN
          IF
            ("node")."index" <> ("formatting_stack"[1])."index" OR
            ("node")."value" <> ("formatting_stack"[1])."value"
          THEN
            RAISE EXCEPTION
              'The formatting stack is desynchronized with the traversal of the command node tree, the formatting stack head is the % command parsed at %, current traversed node is the % command parsed at %',
              ("formatting_stack"[1])."value",
              ("formatting_stack"[1])."index",
              ("node")."value",
              ("node")."index";
          END IF;
          "formatting_stack" := "formatting_stack"[2:];
          IF array_length("line_content_command_stack", 1) = 1 THEN
            "line_content" :=
              "line_content" || "line_content_command_stack"[1];
            "line_content_command_stack" = "formatting_stack";
          END IF;
        WHEN
          'NEW_PAGE', 'COMMENT', 'NO_OP'
        THEN
          RAISE EXCEPTION
            'Encountered a % command with % layout interpretation on the stack while traversing up the command node tree, but such commands are not allowed on the stack',
            ("node")."type",
            "command_layout_interpretation";
        ELSE
          RAISE EXCEPTION
            'Encountered an unknown or unsupported command layout interpretation % (command % parsed at index %) while traversing the node tree upwards',
            "command_layout_interpretation",
            ("node")."value",
            ("node")."index";
      END CASE;
    END LOOP;
  END LOOP;

  "block" := "richtext"."_complete_layout_block"(
    "block",
    "paragraph",
    "line",
    "line_content",
    "line_content_command_stack"
  );
  IF array_length(("block")."paragraphs", 1) IS NOT NULL THEN
    RETURN NEXT "block";
  END IF;
END;
$$;
