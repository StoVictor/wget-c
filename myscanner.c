#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include "myscanner.h"

#define HTTP 80
#define HTTPS 443

extern int yylex();
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

int get_page(char *url, char *filename){
    CURL *curl = NULL;
    FILE *fp = NULL;
    int result = 0;

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

int get_page_s(char *url, char *filename){
    CURL *curl = NULL;
    FILE *fp = NULL;
    int result = 0;

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
    } (*list)[*last_ref]= (char *)malloc(strlen(link)+1);
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


// link processing

int is_link_has_web_protocol_part(char *link){
    char *tmp = "https";
    int len = strlen(tmp);
    for(int i=0; i < len; i++){
        if((tmp[i] != link[i])) {
            if(i != len-1){
                return 0;
            }
            return HTTP;
        }
    }
    return HTTPS; 
}

void remove_link_web_protocol_part(char *link, char **res_link, int protocol){
   if (protocol == HTTP){
       strcpy(*res_link, link+7);
   } else if(protocol == HTTPS){
       strcpy(*res_link, link+8);
   } else {
       strcpy(*res_link, link);
   }
}

void add_not_relative_part(char **link, char *url){
    char *res = (char*)malloc(strlen(*link)+strlen(url)+1);
    strcpy(res, *link);
    strcpy(*link, url);
    strcat(*link, res);
    free(res);
}

void add_not_relative_part_to_simple_file(char **link, char *url){
    char *res = (char*)malloc(strlen(*link)+strlen(url)+5);
    strcpy(res, *link);
    strcpy(*link, url);
    strcat(*link, "/");
    strcat(*link, res);
    free(res);
}

bool is_link_refer_to_this_site(char *link, char *url){
   long int min_size = 0;
   int k = 0;
   if (strstr(link, "www.") != NULL){
       k = 4;
   }
   if(strlen(url) < (strlen(link)-k)){
       min_size = strlen(url);
   } else {
       min_size = (strlen(link)-k);
   }
   for(int i=0; i < min_size; i++){
        //printf("%c|%c ", link[i+k], url[i]);
        if (link[i+k] != url[i]){
            return false;
        }
   }
   //printf("\n");
    
   return true;
}

bool is_link_has_not_realtive_part(char *link){
     int num = strcspn(link, "/");
     if (num != 0){
         return true;
     }
     return false;
}

void get_not_relative_part(char *link, char **res){
    if (strlen(link) == strcspn(link, "/")){
        strncpy(*res, link, strcspn(link, "/")+1);
    } else {
        strncpy(*res, link, strcspn(link, "/"));
    }
}

void get_relative_part(char *link, char **res){
    strcpy(*res, link+strcspn(link, "/"));
}

bool is_link_is_simple_file(char *link){
    int len = strlen(link)-1;
    if (len-5 < 1){
        return false;
    }
    char *htm=".html";
    if ((strchr(link, '/') == NULL)){
        for(int i=len, k=4; i > len-5 && k >= 0; i--, k--){
            if (link[i] != htm[k]){
                return false;
            }
        }
        return true;
    }
    return false;
}

// end link processing

int is_exists(char *part){
    DIR* dir = opendir(part);
    if (dir) {
        closedir(dir);
        return 1;
    } else if (ENOENT == errno) {

        return 0;
    }
    return 0;
}

void create_directory_from_url_part(char *part){
    mkdir(part, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

char *make_file_or_dir(char *link, char **tmp_dir){
    char *rel_part = (char*)malloc(strlen(link)+1);
    get_relative_part(link, &rel_part);
    char *not_rel_part = (char*)malloc(strlen(link)+1);
    get_not_relative_part(link, &not_rel_part);
    char *tmp = (char*)malloc(strlen(rel_part)+1);
    if (strlen(rel_part) == 1){
        return "";  
    }
    strcpy(tmp, rel_part+strcspn(rel_part, "/"));

    //char *tmp_dir = (char*)malloc(strlen(rel_part)+1);
    strcpy(*tmp_dir, "");
    int i=0;
    int first_dir = 0;

    while (i < strlen(tmp)){
       if(tmp[i] == '/'){
          if (first_dir == 0){
            first_dir = i+1;
          } else {
            strncat(*tmp_dir, tmp+first_dir, i-first_dir);
            first_dir = i;
            if (is_exists(*tmp_dir) == 0){
                create_directory_from_url_part(*tmp_dir);
            }
          }
       }
       i++;
    }
    if (is_exists(rel_part+strcspn(rel_part, "/")+1) == 0){
        strncat(*tmp_dir, tmp+first_dir, i);
        //printf("LINK: %s, TMP_DIR: %s \n", link, tmp_dir);
        get_page_s(link, *tmp_dir);
    }
    free(tmp);
    //free(tmp_dir);
    free(rel_part);
    free(not_rel_part);
    return "";
}

int get_all_links_on_page(char *filename, char ***ref, int *last_ref, int *size, char *url, int *depth){

    int ntoken;
    int temp_size = 256;
    int last_temp_ref = 0;
    int status = CLOSE_TAG;

    char **temp = (char**)malloc(temp_size*sizeof(char*));
    //printf("READ %s %s \n", filename, url);
    yyin = fopen(filename, "r");
    ntoken = yylex();

    char *names[9] = {NULL, "open href tag", "open src tag", "/> or >", "href", "src", "end href or src", "LINK", "relative_link"};
    while (ntoken) {
        if((ntoken == HREF_TAG) && (status == CLOSE_TAG)) {
            status = HREF_TAG;
        } else if ((ntoken == SRC_TAG) && (status == CLOSE_TAG)){
            status = SRC_TAG;
        } else if ((ntoken == HREF) && (status == HREF_TAG)) {
            status = HREF;
        } else if ((ntoken == SRC) && (status == SRC_TAG)) {
            status = SRC;
        } else if ((ntoken == RELATIVE_LINK) && ((status == HREF) || (status == SRC))){
            char *tmp_link = (char*)malloc(strlen(yytext)+strlen(url)+120);
            status = RELATIVE_LINK;
            strcpy(tmp_link, yytext);
            if(is_link_has_web_protocol_part(yytext)){
                remove_link_web_protocol_part(yytext, &tmp_link, is_link_has_web_protocol_part(yytext));
            }
            if(is_link_has_not_realtive_part(tmp_link)){
                if(is_link_is_simple_file(tmp_link)){
                    add_not_relative_part_to_simple_file(&tmp_link, url);
                    if(is_link_already_in_list(tmp_link, ref, *last_ref) == 0){ 
                        add_link_to_list(tmp_link, &temp, &last_temp_ref, &temp_size);
                        add_link_to_list(tmp_link, ref, last_ref, size);
                    }
                }
                if(is_link_refer_to_this_site(tmp_link, url)){
                    /*if(is_link_already_in_list(tmp_link, ref, *last_ref) == 0){ 
                        add_link_to_list(tmp_link, &temp, &last_temp_ref, &temp_size);
                        add_link_to_list(tmp_link, ref, last_ref, size);
                    }*/
                }
            } else { 
                add_not_relative_part(&tmp_link, url);
                if(is_link_already_in_list(tmp_link, ref, *last_ref) == 0){ 
                    add_link_to_list(tmp_link, &temp, &last_temp_ref, &temp_size);
                    add_link_to_list(tmp_link, ref, last_ref, size);
                }
            }
            free(tmp_link);
        } else if ((ntoken == END_HREF_OR_SRC) && (status == RELATIVE_LINK)){
            status = END_HREF_OR_SRC;
        } else if ((ntoken == CLOSE_TAG)){
            status = CLOSE_TAG;
        }
        ntoken = yylex(); 
    }
    //printf("LAST_REF: %d \n", *last_ref);
    //printf("LAST_TMP_REF: %d \n", last_temp_ref);
    print_link_list(*ref, *last_ref);
    for (int i=0; i < last_temp_ref; i++){
        char *tmp_dir = (char*)malloc(strlen(temp[i])+1);
        make_file_or_dir(temp[i], &tmp_dir);
        if (*depth < 10){
            (*depth) += 1;
            get_all_links_on_page(tmp_dir, ref, last_ref, size, url, depth);
        }
        free(tmp_dir);
    }

    free(temp);
    fclose(yyin);
    return 0;
}


int main(int argc, char *argv[]){
    int size = 256;
    char **ref = (char**)malloc(size*sizeof(char*));
    char *url = (char*)malloc(strlen(argv[1])+1);
    get_not_relative_part(argv[1], &url);

    int last_ref = 0;
    int depth = 0;
    char *filename = argv[2];
    get_page(argv[1], filename); 
    get_all_links_on_page(filename, &ref, &last_ref, &size, url, &depth);
    //make_file_or_dir(ref[0]);
    // grab links that doesnt belongs to site
    // bad cat url with relative part
    free(url);
    free(ref);

    return 0;
}
