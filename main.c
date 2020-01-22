#include <stdio.h>
#include <curl/curl.h>

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

int main(int argc, char* argv[]){
    int result;
    result = get_page(argv[1], argv[2]);
    return 0;
}
