%{
#include "myscanner.h"
%}

ws   [ \t\r\n]
src_tags "script"|"area"|"image"|"audio"|"video"|"bgsound"|"track"
href_tags "link"|"a"

%%
"<"(?i:{href_tags})     return HREF_TAG;
"<"(?i:{src_tags})      return SRC_TAG;
"/>"                    return CLOSE_TAG;
>                       return CLOSE_TAG;
href=\"                 return HREF;
src=\"                  return SRC;
\"                      return END_HREF_OR_SRC;
http:\/\/[^ \n<>"]*     return LINK;
https:\/\/[^ \n<>"]*    return LINK;
\/[^ \n<>"]*            return RELATIVE_LINK;
.                       ;
%%

//Выделить в отдельную функцию все проверки и добавления в список
//Сделать проверку, есть ли функция уже в списке
//Сдлеать дописывание относительных ссылок
//Слить эту программу с программой для скачивания страниц
//Добавить функционал создания директорий
//Сделать рекурсивное скачивание с настройкой глубины

int yywrap(void)
{
    return 1;
}
