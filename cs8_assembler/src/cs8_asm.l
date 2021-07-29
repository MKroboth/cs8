%{
#include "parser.h"
#include <forward_list>
#include <filesystem>
std::forward_list<std::filesystem::path> cwds;
std::string buffer;
%}

%x incl
%x ch
%x str

%%
"'"  BEGIN(ch);
<ch>"\\n" { sslval.ival = 10;
            return TOK_NUMBER;
          }

<ch>[\\]. { sslval.ival = yytext[1];
            return TOK_NUMBER;
          }

<ch>[^']  { sslval.ival = yytext[0];
            return TOK_NUMBER;
          }

<ch>"'" BEGIN(INITIAL);

"\"" { buffer.clear();
       BEGIN(str);
     }

<str>"\\n"  { buffer += '\n'; }
<str>[\\].  { buffer += yytext; }
<str>[^\"]  { buffer += yytext; }
<str>"\""   { BEGIN(INITIAL);
              sslval.sval = (char*)buffer.c_str();
              return TOK_STRING;
            }

".include" BEGIN(incl);
<incl>[ \t]*      { /* eat the whitespace */ }
<incl>[^ \t\n]+   { /* got the include file name */
    yyin = fopen( yytext, "r" );
    if ( ! yyin )
        throw std::runtime_error(std::string("Cannot include file: ") + yytext);

        cwds.push_front(std::filesystem::current_path());
        auto path = std::filesystem::path(yytext);
        if(path.has_parent_path()) {
          std::filesystem::current_path(path.parent_path());
        }
        yypush_buffer_state(yy_create_buffer( yyin, YY_BUF_SIZE ));
        BEGIN(INITIAL);
}

<<EOF>> { if(!cwds.empty()) {
            std::filesystem::current_path(cwds.front());
            cwds.pop_front();
          }

          yypop_buffer_state();
          if ( !YY_CURRENT_BUFFER ) {
            yyterminate();
          }
        }

[\;][^\n]*[\n]* { /* eat comments */ }

".define" { return TOK_DEFINE; }
".macro"  { return TOK_START_MACRO; }
".endm"   { return TOK_END_MACRO; }

[a-zA-Z]+[a-zA-Z0-9]* { sslval.sval = yytext;
                        return TOK_IDENTIFIER;
                      }

0x[0-9A-Fa-f]+        { sslval.ival = strtol(yytext, NULL, 16);
                        return TOK_NUMBER;
                      }

[0-9]+ { sslval.ival = atoi(yytext);
         return TOK_NUMBER;
       }


"%"    { return TOK_PERCENT; }
"+"    { return TOK_PLUS; }
"-"    { return TOK_MINUS; }
"*"    { return TOK_TIMES; }
"/"    { return TOK_DIVIDE; }
","    { return TOK_COMMA; }
"."    { return TOK_DOT; }
":"    { return TOK_COLON; }
"("    { return TOK_OPEN_PAR; }
")"    { return TOK_CLOSE_PAR; }
"["    { return TOK_OPEN_BRA; }
"]"    { return TOK_CLOSE_BRA; }
"\\"   { return TOK_BACKSLASH; }
\n+    { return TOK_NEWLINE; }
[ \t]+ { /* eat whitespace */  }


.      { sslval.sval = yytext;
         return TOK_UNKNOWN;
       }
%%