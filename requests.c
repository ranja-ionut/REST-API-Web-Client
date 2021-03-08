#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char *token,
                            char **cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Add the host
    memset(line, 0, LINELEN);
    memcpy(line, HOST_HDR, HOST_HDR_SIZE);
    strcat(line, host);
    compute_message(message, line);

    // Add headers and/or cookies, according to the protocol format
    if(token != NULL){
        memset(line, 0, LINELEN);
        memcpy(line, AUTHORIZATION_HDR, AUTHORIZATION_HDR_SIZE);
        strcat(line, token);
        compute_message(message, line);
    }

    if (cookies != NULL) {
       for(int i = 0; i < cookies_count; i++){
            memset(line, 0, LINELEN);
            memcpy(line, COOKIE, COOKIE_SIZE);
            strcat(line, cookies[i]);
            compute_message(message, line);
       }
    }
    // Add final new line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
                            char *token,
                           char **cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Add the host
    memset(line, 0, LINELEN);
    memcpy(line, HOST_HDR, HOST_HDR_SIZE);
    strcat(line, host);
    compute_message(message, line);

    /** 
     * Add necessary headers (Content-Type and Content-Length are mandatory).
     */
    if(token != NULL){
        memset(line, 0, LINELEN);
        memcpy(line, AUTHORIZATION_HDR, AUTHORIZATION_HDR_SIZE);
        strcat(line, token);
        compute_message(message, line);
    }

    memset(line, 0, LINELEN);
    memcpy(line, CONTENT_LENGTH_HDR, CONTENT_LENGTH_HDR_SIZE);
    sprintf(&line[CONTENT_LENGTH_HDR_SIZE], "%li", strlen(body_data));
    compute_message(message, line);

    memset(line, 0, LINELEN);
    memcpy(line, CONTENT_TYPE_HDR, CONTENT_TYPE_HDR_SIZE);
    strcat(line, content_type);
    compute_message(message, line);

    // Add cookies
     if (cookies != NULL) {
       for(int i = 0; i < cookies_count; i++){
            memset(line, 0, LINELEN);
            memcpy(line, COOKIE, COOKIE_SIZE);
            strcat(line, cookies[i]);
            compute_message(message, line);
       }
    }
    // Add new line at end of header
    compute_message(message, "");

    // Add the actual payload data
    memset(line, 0, LINELEN);
    memcpy(line, body_data, strlen(body_data));
    compute_message(message, line);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *token,
						   char** cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Write the method name, URL, request params (if any) and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    // Add the host
    memset(line, 0, LINELEN);
    memcpy(line, HOST_HDR, HOST_HDR_SIZE);
    strcat(line, host);
    compute_message(message, line);

    // Add headers and/or cookies, according to the protocol format
    if(token != NULL){
        memset(line, 0, LINELEN);
        memcpy(line, AUTHORIZATION_HDR, AUTHORIZATION_HDR_SIZE);
        strcat(line, token);
        compute_message(message, line);
    }

    if (cookies != NULL) {
       for(int i = 0; i < cookies_count; i++){
            memset(line, 0, LINELEN);
            memcpy(line, COOKIE, COOKIE_SIZE);
            strcat(line, cookies[i]);
            compute_message(message, line);
       }
    }
    // Add final new line
    compute_message(message, "");

    free(line);
    return message;
}