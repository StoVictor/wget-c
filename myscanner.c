#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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

int is_exists(char **part){
  struct stat info;
  if (stat(*part, &info) == -1){
      return 0;
  }
  return 1;
}

void create_directory_from_url_part(char **part){
    mkdir(*part, 0700);
}

void make_file_or_dir(char *relative_part){

    printf("%s\n", relative_part);
    char *tmp=relative_part;
    char *tmp_dir;
    int i=0;
    int first_dir = 0;
    int second_dir = 0;

    while (*tmp != '\0'){
       if(*tmp == '/'){
          if (first_dir == 0){
            first_dir = i;
          } else {
            tmp_dir=strncat(tmp_dir, relative_part+first_dir, i);
            if (is_exists(&tmp_dir) == 0){
                create_directory_from_url_part(&tmp_dir);
            }
          }
       }
       i++;
       tmp++;
    }
    if (is_exists(&relative_part) == 0){
        tmp_dir = tmp_dir+strspn(tmp_dir, relative_part);
        get_page(relative_part, tmp_dir);
    }
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
        printf("%s\n", names[ntoken]);
        if((ntoken == HREF_TAG) && (status == CLOSE_TAG)) {
            status = HREF_TAG;
        } else if ((ntoken == SRC_TAG) && (status == CLOSE_TAG)){
            status = SRC_TAG;
        } else if ((ntoken == HREF) && (status == HREF_TAG)) {
            status = HREF;
        } else if ((ntoken == SRC) && (status == SRC_TAG)) {
            status = SRC;
        } else if((ntoken == LINK) && (status == HREF)){
            status = LINK;
        } else if ((ntoken == RELATIVE_LINK) && ((status == LINK) || (status == HREF) || (status == SRC))){
            status = RELATIVE_LINK;
            if(is_link_already_in_list(yytext, ref, *last_ref) == 0){ 
                add_link_to_list(yytext, &temp, &last_temp_ref, &temp_size);
                add_link_to_list(yytext, ref, last_ref, size);
             }
        } else if ((ntoken == END_HREF_OR_SRC) && (status == LINK)) {
            //If find link without relative part, suppose this link is the main page
            if (is_link_already_in_list("/", ref, *last_ref) == 0){
                add_link_to_list("/", &temp, &last_temp_ref, &temp_size);   
                add_link_to_list("/", ref, last_ref, size); 
            }
        } else if ((ntoken == END_HREF_OR_SRC) && (status == RELATIVE_LINK)){
            status = END_HREF_OR_SRC;
        } else if ((ntoken == CLOSE_TAG)){
            status = CLOSE_TAG;
        }
        
        ntoken = yylex(); 
    }

    free(temp);
    fclose(yyin);
    return 0;
}



int main(int argc, char *argv[]){
    int size = 256;
    char **ref = (char**)malloc(size*sizeof(char*));
    int last_ref = 0;

    //char *filename = "probnichek";
    char *filename = argv[1];
    //get_page(argv[1], filename);
    get_all_links_on_page(filename, &ref, &last_ref, &size);
    print_link_list(ref, last_ref);
    //for (int i=0; i < last_ref; i++){
    //    make_file_or_dir(ref[i]);
    //}

    free(ref);

    return 0;
}
