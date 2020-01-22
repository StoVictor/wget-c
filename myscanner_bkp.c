#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "myscanner.h"

extern int yylex();
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

int get_page(char *url, char *filename){
    CURL *curl;
    FILE *fp;
    int result;

    fp = fopen(filename, "wb");
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    result = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);
    return result;
}

int is_link_already_in_list(char *link, char **list[], int last_ref){
    int cmp_res;
    for(int i=0; i < last_ref; i++){
       cmp_res = strcmp((*list)[i], link); 
       if (cmp_res == 0) {
           return 1;
       }
    }
    return 0;
}

void add_link_to_list(char* link, char ***list, int *last_ref, int *size){
    if ((*last_ref) == (*size)){
        (*size)*=8;
        *list = (char**)realloc(*list, sizeof(**list)*(*size));
    }
    (*list)[*last_ref]= (char *)malloc(strlen(link)+1);
    strcpy((*list)[*last_ref], link);
    (*last_ref)++;
}

void print_link_list(char **list, int last_ref){
    if (list[0]){
        printf("open link list>>>>>>>\n");
        for(int i=0; i < last_ref; i++){
            printf("%s\n", list[i]);
        } 
        printf("close link list>>>>>>>\n");
    }
}

void add_url_part_to_relative_link(char *url, char **relative_part){
    char *tmp;
    strcat(tmp, url);
    strcat(tmp, *relative_part);
    strcpy(*relative_part, tmp);
}

int get_all_links_on_page(char *filename, char ***ref, int *last_ref, int *size){

    int ntoken;
    int temp_size = 256;
    int last_temp_ref = 0;
    int status = CLOSE_TAG;

    char **temp = (char**)malloc(temp_size*sizeof(char*));

    yyin = fopen(filename, "r");
    ntoken = yylex();

    char *names[9] = {NULL, "open href tag", "open src tag", "/> or >", "href", "src", "end href or src", "link", "relative_link"};

    while (ntoken) {
        if((ntoken == HREF_TAG) && (status == CLOSE_TAG)) {
            status = HREF_TAG;
        } else if ((ntoken == SRC_TAG)){
            status = SRC_TAG;
        } else if ((ntoken == HREF) && (status == HREF_TAG)) {
            status = HREF;
        } else if ((ntoken == SRC) && (status == SRC_TAG)) {
            status = SRC;
        } else if((ntoken == LINK) && (status == HREF)){
            status = LINK;
            if(is_link_already_in_list(yytext, ref, *last_ref) == 0){ 
                add_link_to_list(yytext, &temp, &last_temp_ref, &temp_size);
                add_link_to_list(yytext, ref, last_ref, size);
             }
        } else if ((ntoken == RELATIVE_LINK) && (status == HREF)){
            status = LINK;
            if(is_link_already_in_list(yytext, ref, *last_ref) == 0){ 
                add_link_to_list(yytext, &temp, &last_temp_ref, &temp_size);
                add_link_to_list(yytext, ref, last_ref, size);
             }
        } else if ((ntoken == RELATIVE_LINK) && (status == SRC)){
            status = LINK;
            if(is_link_already_in_list(yytext, ref, *last_ref) == 0){ 
                add_link_to_list(yytext, &temp, &last_temp_ref, &temp_size);
                add_link_to_list(yytext, ref, last_ref, size);
             }
        } else if ((ntoken == END_HREF_OR_SRC) && (status == LINK)){
            status = END_HREF_OR_SRC;
        } else if ((ntoken == CLOSE_TAG) && (status == END_HREF_OR_SRC)){
            status = CLOSE_TAG;
        }
        
        ntoken = yylex(); 
    }

    free(temp);
    fclose(yyin);
    return 0;
}
   
void get_page_from_url(char *url, char ***ref){
    ;
}

int main(int argc, char *argv[]){
    int size = 256;
    char **ref = (char**)malloc(size*sizeof(char*));
    int last_ref = 0;
    get_all_links_on_page(argv[1], &ref, &last_ref, &size);
    print_link_list(ref, last_ref);
    return 0;
}
