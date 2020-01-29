%{
#include "myscanner.h"
%}

ws   [ \t\r\n]
src_tags "image"|"audio"|"video"|"bgsound"|"track"
href_tags "link"|"a"
h   "http"|"https"

%%
"<"(?i:{href_tags})          return HREF_TAG;
"<"(?i:{src_tags})           return SRC_TAG;
"/>"                         return CLOSE_TAG;
">"                          return CLOSE_TAG;
href=\"                      return HREF;
src=\"                       return SRC;
HREF=\"                      return HREF;
SRC=\"                       return SRC;
[^(href=|src=)]\"            return END_HREF_OR_SRC;
[^ \n<>"?]*                  return RELATIVE_LINK;
.                            ;
%%

int yywrap(void)
{
    return 1;
}
