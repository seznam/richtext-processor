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

