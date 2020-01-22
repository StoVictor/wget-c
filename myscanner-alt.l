%{
#include "myscanner.h"
%}

ws   [ \t\r\n]
src_tags "script"|"area"|"image"|"audio"|"video"|"bgsound"|"track"
href_tags "link"|"a"

%%
"<"(?i:{href_tags})          return HREF_TAG;
"<"(?i:{src_tags})           return SRC_TAG;
"/>"                         return CLOSE_TAG;
">"                          return CLOSE_TAG;
(?i:href)=\"                 return HREF;
(?i:src)=\"                  return SRC;
\"                           return END_HREF_OR_SRC;
https?:\/\/[^ \n<>"]*\/\/      return LINK;
[^ \n<>"?]*                  return RELATIVE_LINK;
.                            ;
%%

int yywrap(void)
{
    return 1;
}
