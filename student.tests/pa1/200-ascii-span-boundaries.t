// N3485 focus: [lex.phases], [lex.name], [lex.string].
ordinary_identifier_123   after_spaces
prefix\
suffix ascii\u03C0tail \u03C0after πending
joined??/
identifier /* ignored */ after_comment
/* comment containing a trigraph splice ??/
and a closing marker *\
/ after_block_comment
// continued comment \
"discarded unterminated literal
after_line_comment
R"abcdefghijklmnop(raw\
text??/\u03C0)abcdefghijklmnop"_suffix
"escaped\\u03C0" after_literal
last_identifier
