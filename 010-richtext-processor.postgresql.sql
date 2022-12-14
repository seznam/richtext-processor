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
