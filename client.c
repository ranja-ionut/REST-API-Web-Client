#include "helpers.h"
#include "requests.h"
#include "buffer.h"

/**
 * Citeste o comanda de la tastatura si va returna un cod specific.
 */
int get_command() {
    char command[LINELEN];

    memset(command, 0, LINELEN);
    scanf(" %[^\n]", command);

    if (strcmp(command, EXIT_CMD) == 0) return EXIT;

    if (strcmp(command, REGISTER_CMD) == 0) return REGISTER;
            
    if (strcmp(command, LOGIN_CMD) == 0) return LOGIN;

    if (strcmp(command, LIBRARY_ACCESS_CMD) == 0) return LIBRARY_ACCESS;
            
    if (strcmp(command, LOGOUT_CMD) == 0) return LOGOUT;

    if (strcmp(command, VIEW_BOOKS_CMD) == 0) return VIEW_BOOKS;

    if (strcmp(command, VIEW_BOOK_CMD) == 0) return VIEW_BOOK;

    if (strcmp(command, ADD_BOOK_CMD) == 0) return ADD_BOOK;

    if (strcmp(command, DELETE_BOOK_CMD) == 0) return DELETE_BOOK;

    if (strcmp(command, LOGOUT_CMD) == 0) return LOGOUT;

    return BAD_CMD;
}

/**
 * Creeaza un payload de tip JSON pentru register/login folosind
 * datele primite ca argumente.
 */
char *create_register_login_json(char *username, char *password) {
    char *payload = calloc(LINELEN, sizeof(char));
    
    memcpy(payload, JSON_START, sizeof(JSON_START));
    strcat(payload, USERNAME);
    strcat(payload, JSON_STRING);
    strcat(payload, username);
    strcat(payload, JSON_STRING);
    strcat(payload, LINKER);
    strcat(payload, PASSWORD);
    strcat(payload, JSON_STRING);
    strcat(payload, password);
    strcat(payload, JSON_STRING);
    strcat(payload, JSON_END);

    return payload;
}

/**
 * Creeaza un payload de tip JSON pentru adaugarea unei carti
 * in biblioteca folosind datele primite ca argumente.
 */
char *create_add_book_json(char *title, char *author, 
                            char *publisher, char *genre, int page_count) {
    char *payload = calloc(LINELEN, sizeof(char));
    
    memcpy(payload, JSON_START, sizeof(JSON_START));
    strcat(payload, TITLE);
    strcat(payload, JSON_STRING);
    strcat(payload, title);
    strcat(payload, JSON_STRING);
    strcat(payload, LINKER);
    strcat(payload, AUTHOR);
    strcat(payload, JSON_STRING);
    strcat(payload, author);
    strcat(payload, JSON_STRING);
    strcat(payload, LINKER);
    strcat(payload, PUBLISHER);
    strcat(payload, JSON_STRING);
    strcat(payload, publisher);
    strcat(payload, JSON_STRING);
    strcat(payload, LINKER);
    strcat(payload, GENRE);
    strcat(payload, JSON_STRING);
    strcat(payload, genre);
    strcat(payload, JSON_STRING);
    strcat(payload, LINKER);
    strcat(payload, PAGE_COUNT);
    sprintf(&payload[strlen(payload)], "%i", page_count);
    strcat(payload, JSON_END);

    return payload; 
}

/**
 * Parcurge raspunsul de la server si cauta aparitia cuvantului-cheie
 * "Set-Cookie:", dupa care va citi din buffer acest cookie si va
 * pune pe ultima pozitia a acestuia terminatorul de sir pentru
 * a sterge `;` din cookie.
 * 
 * Intoarce NULL in cazul in care raspunsul de la server nu este cel
 * asteptat, asadar neexistand un cookie in raspuns.
 */
char *extract_cookie_from_response(char *response) {
    buffer helper = buffer_init();
    buffer_add(&helper, response, strlen(response));

    char *cookie = calloc(LINELEN, sizeof(char));

    int found = buffer_find(&helper, SET_COOKIE, SET_COOKIE_SIZE - 1);
    if (found == -1) { 
        buffer_destroy(&helper);
        return NULL;
    }

    found += SET_COOKIE_SIZE - 1;

    sscanf(&response[found], "%s", cookie);

    cookie[strlen(cookie) - 1] = '\0'; // remove ; from end of cookie

    buffer_destroy(&helper);

    return cookie;
}

/**
 * La fel ca la extragerea pentru cookie, va parcurge raspunsul de la
 * server pentru a gasi inceputul unui JSON -> `{"` si va imparti
 * JSON-ul in tokeni, pentru a putea extrage token-ul JWT.
 * 
 * Intoarce NULL in cazul in care raspunsul de la server nu este cel
 * asteptat, astfel raspunsul necontinand un JWT token.
 */
char *extract_jwt_token_from_response(char *response) {
    buffer helper = buffer_init();
    buffer_add(&helper, response, strlen(response));

    char *jwt_token = calloc(LINELEN, sizeof(char));

    int found = buffer_find(&helper, JSON_SEARCH, sizeof(JSON_SEARCH) - 1);
    if (found == -1) {
        buffer_destroy(&helper);
        return 0;
    }

    char copy[LINELEN];
    memset(copy, 0, LINELEN);
    memcpy(copy, &response[found], strlen(&response[found]));

    char *token;
    const char *delim = "{\" \":\" \"}";

    token = strtok(copy, delim);

    if (token == NULL) {
        buffer_destroy(&helper);
        return NULL;
    }

    token = strtok(NULL, delim);

    if (token == NULL) {
        buffer_destroy(&helper);
        return NULL;
    }

    memcpy(jwt_token, token, strlen(token));

    buffer_destroy(&helper);

    return jwt_token;
}

/**
 * Detecteaza daca raspunsul de la server primit in urma unei comenzi
 * care actioneaza asupra bibliotecii este 500 Internal Server Error,
 * mesaj care se va primi cand token-ul JWT a expirat sau este invalid.
 * 
 * Intoarce 1 daca s-a primit 500 Internal Server Error, in caz contrar
 * va intoarce 0.
 */
int detect_expiration(char *response) {
    buffer helper = buffer_init();
    buffer_add(&helper, response, strlen(response));

    int found = buffer_find(&helper, JWT_EXPIRATION, sizeof(JWT_EXPIRATION) - 1);
    if (found == -1) {
        buffer_destroy(&helper);
        return 0;
    }

    buffer_destroy(&helper);

    return 1;
}



int main() {
    char *message;
    char *response;

    int sockfd;
    int choice;

    /**
     * Afla adresa ip a HOST-ului folosind functii specializate.
     */
    struct hostent *host = gethostbyname(HOST);
    DIE(host == NULL, "couldn't resolve host name");

    char *ip_str = inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));
    DIE(ip_str == NULL, "inet_ntoa");


    /**
     * Variabile folosite pentru logica de gestionare a comenzilor primite
     * de la utilizator.
     */
    int page_count, expired = 0;

    char username[LINELEN], password[LINELEN], book[LINELEN];
    char title[LINELEN], author[LINELEN], genre[LINELEN], publisher[LINELEN];
    char *payload = NULL, *login_cookie = NULL, *token = NULL;

    forever {
        choice = get_command();
        
        // Comanda `exit` duce la distrugerea buclei.
        if (choice == EXIT) {
            break;
        }

        /** 
         * Daca token-ul JWT a expirat sau cumva a devenit invalid
         * atunci scapa de cel vechi si marcheaza faptul ca userul
         * nu dispune de un token JWT.
         */
        if (expired == 1) {
            if (token != NULL) free(token);

            token = NULL;
        }

        switch (choice) {
            /**
             * Citeste de la tastatura `username` si `password` si trimite
             * serverului un mesaj de tip POST pentru a se inregistra
             * folosind variabilele citite anterior.
             */
            case REGISTER:
                memset(username, 0, LINELEN);
                memset(password, 0, LINELEN);

                printf("username=");
                scanf(" %[^\n]", username);

                printf("password=");
                scanf(" %[^\n]", password);

                payload = create_register_login_json(username, password);

                message = compute_post_request(HOST, REGISTER_URL, 
                CONTENT_APP_JSON, payload, NULL, NULL, 0);

                sockfd = open_connection(ip_str, PORT, AF_INET,
                                         SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                
                response = receive_from_server(sockfd);
                close_connection(sockfd);

                printf("\n%s\n\n", response);

                free(message);
                free(response);
                free(payload);
            break;

            /**
             * Citeste de la tastatura `username` si `password` si trimite
             * serverului un mesaj de tip POST pentru a se autentifica
             * folosind variabilele citite anterior.
             */
            case LOGIN:
                if (login_cookie != NULL) {
                    printf("\nYou already are logged in. "
                            "Try logging out first.\n\n");
                    break;
                }

                memset(username, 0, LINELEN);
                memset(password, 0, LINELEN);

                fflush(stdin);
                fflush(stdout);
                printf("username=");
                scanf(" %[^\n]", username);

                printf("password=");
                scanf(" %[^\n]", password);

                payload = create_register_login_json(username, password);

                message = compute_post_request(HOST, LOGIN_URL, 
                CONTENT_APP_JSON, payload, NULL, NULL, 0);

                sockfd = open_connection(ip_str, PORT, AF_INET,
                                         SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                
                response = receive_from_server(sockfd);
                close_connection(sockfd);

                login_cookie = extract_cookie_from_response(response);
                
                printf("\n%s\n\n", response);

                free(message);
                free(response);
                free(payload);
            break;

            /**
             * Trimite un mesaj de tip GET serverului pentru a putea
             * extrage un token JWT.
             */
            case LIBRARY_ACCESS:
                if (login_cookie == NULL) {
                    message = compute_get_request(HOST, LIBRARY_ACCESS_URL, 
                    NULL, NULL, NULL, 0);
                } else {
                    message = compute_get_request(HOST, LIBRARY_ACCESS_URL, 
                    NULL, NULL, &login_cookie, 1);
                }

                sockfd = open_connection(ip_str, PORT, AF_INET,
                                         SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                
                response = receive_from_server(sockfd);
                close_connection(sockfd);

                token = extract_jwt_token_from_response(response);
                
                printf("\n%s\n\n", response);

                free(message);
                free(response);
            break;

            /**
             * Trimite un mesaj de tip GET serverului pentru a primi
             * lista cu carti.
             */
            case VIEW_BOOKS:
                if (login_cookie == NULL) {
                    message = compute_get_request(HOST, VIEW_BOOKS_URL,
                    NULL, token, NULL, 0);
                } else {
                    message = compute_get_request(HOST, VIEW_BOOKS_URL,
                    NULL, token, &login_cookie, 1);
                }

                sockfd = open_connection(ip_str, PORT, AF_INET,
                                         SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                
                response = receive_from_server(sockfd);
                close_connection(sockfd);

                expired = detect_expiration(response);
                
                printf("\n%s\n\n", response);

                free(message);
                free(response);
            break;

            /**
             * Trimite un mesaj de tip GET serverului pentru a primi
             * toate detaliile despre o carte a carui ID este citit
             * de la tastatura.
             */
            case VIEW_BOOK:
                memset(book, 0, LINELEN);
                memcpy(book, VIEW_BOOK_URL, sizeof(VIEW_BOOK_URL) - 1);

                /**
                 * Daca nu s-a citit o valoare de tip int atunci
                 * continua sa citesti de la tastatura.
                 */
                char read_buf[LINELEN], checker[LINELEN];
                int stop = -1, book_id;
                while (stop < 0) {
                    printf("id=");

                    memset(read_buf, 0, LINELEN);
                    memset(checker, 0, LINELEN);
                    scanf(" %[^\n]", read_buf);
                    
                    printf("\n");

                    book_id = atoi(read_buf);

                    sprintf(checker, "%i", book_id);

                    if (book_id < 0
                        || (book_id == 0 && (strcmp(read_buf, "0") != 0))
                        || strcmp(checker, read_buf) != 0) {
                        printf("Bad input. Try again!\n");
                    } else stop = 1;
                }

                sprintf(&book[strlen(book)], "%i", book_id);

                if (login_cookie == NULL) {
                    message = compute_get_request(HOST, book,
                    NULL, token, NULL, 0);
                } else {
                    message = compute_get_request(HOST, book,
                    NULL, token, &login_cookie, 1);
                }

                sockfd = open_connection(ip_str, PORT, AF_INET,
                                         SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                
                response = receive_from_server(sockfd);
                close_connection(sockfd);

                expired = detect_expiration(response);
                
                printf("\n%s\n\n", response);

                free(message);
                free(response);
            break;
            
            /**
             * Trimite un mesaj de tip POST serverului pentru a adauga
             * o carte a caror campuri sunt citite de la tastatura.
             */
            case ADD_BOOK:
                memset(title, 0, LINELEN);
                memset(author, 0, LINELEN);
                memset(genre, 0, LINELEN);
                memset(publisher, 0, LINELEN);

                fflush(stdin);
                fflush(stdout);
                printf("title=");
                scanf(" %[^\n]", title);

                printf("author=");
                scanf(" %[^\n]", author);

                printf("genre=");
                scanf(" %[^\n]", genre);

                printf("publisher=");
                scanf(" %[^\n]", publisher);

                /**
                 * Daca nu s-a citit o valoare de tip int atunci
                 * continua sa citesti de la tastatura.
                 */
                stop = -1;
                while (stop < 0) {
                    printf("page_count=");

                    memset(read_buf, 0, LINELEN);
                    memset(checker, 0, LINELEN);
                    scanf(" %[^\n]", read_buf);
                    
                    printf("\n");

                    page_count = atoi(read_buf);
                    sprintf(checker, "%i", page_count);

                    if (page_count < 0
                        || (page_count == 0 && (strcmp(read_buf, "0") != 0))
                        || strcmp(checker, read_buf) != 0) {
                        printf("Bad input. Try again!\n");
                    } else stop = 1;
                }

                payload = create_add_book_json(title, author, 
                            publisher, genre, page_count);

                if (login_cookie == NULL) {
                    message = compute_post_request(HOST, ADD_BOOK_URL, 
                    CONTENT_APP_JSON, payload, token, NULL, 0);
                } else {
                    message = compute_post_request(HOST, ADD_BOOK_URL, 
                    CONTENT_APP_JSON, payload, token, &login_cookie, 1);
                }

                sockfd = open_connection(ip_str, PORT, AF_INET,
                                         SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                
                response = receive_from_server(sockfd);
                close_connection(sockfd);

                expired = detect_expiration(response);
                
                printf("\n%s\n\n", response);

                free(message);
                free(response);
                free(payload);
            break;

            /**
             * Trimite un mesaj de tip DELETE catre server
             * pentru a sterge o carte a carui ID este citit
             * de la tastatura.
             */
            case DELETE_BOOK:
                memset(book, 0, LINELEN);
                memcpy(book, REMOVE_BOOK_URL, sizeof(REMOVE_BOOK_URL) - 1);

                /**
                 * Daca nu s-a citit o valoare de tip int atunci
                 * continua sa citesti de la tastatura.
                 */
                stop = -1;
                while (stop < 0) {
                    printf("id=");

                    memset(read_buf, 0, LINELEN);
                    memset(checker, 0, LINELEN);
                    scanf(" %[^\n]", read_buf);
                    
                    printf("\n");

                    book_id = atoi(read_buf);
                    sprintf(checker, "%i", book_id);
                    
                    if (book_id < 0
                        || (book_id == 0 && (strcmp(read_buf, "0") != 0))
                        || strcmp(checker, read_buf) != 0) {
                        printf("Bad input. Try again!\n");
                    } else stop = 1;
                }

                sprintf(&book[strlen(book)], "%i", book_id);

                if (login_cookie == NULL) {
                    message = compute_delete_request(HOST, book, token,
                                NULL, 0);
                } else {
                    message = compute_delete_request(HOST, book, token,
                                &login_cookie, 1);
                }
                
                sockfd = open_connection(ip_str, PORT, AF_INET,
                                         SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                
                response = receive_from_server(sockfd);
                close_connection(sockfd);

                expired = detect_expiration(response);
                
                printf("\n%s\n\n", response);

                free(message);
                free(response);
            break;

            /**
             * Trimite un mesaj de tip GET catre server pentru a cere
             * delogarea user-ului curent.
             */
            case LOGOUT:
                if (login_cookie == NULL) {
                    message = compute_get_request(HOST, LOGOUT_URL, NULL,
                    token, NULL, 1);
                } else {
                    message = compute_get_request(HOST, LOGOUT_URL, NULL,
                    token, &login_cookie, 1);
                }

                sockfd = open_connection(ip_str, PORT, AF_INET,
                                         SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                
                response = receive_from_server(sockfd);
                close_connection(sockfd);
                
                printf("\n%s\n\n", response);
                
                if (login_cookie != NULL) free(login_cookie);

                if (token != NULL) free(token);

                login_cookie = NULL;
                token = NULL;

                free(message);
                free(response);
            break;

            /**
             * In cazul in care s-a citit o comanda care nu este
             * recunoscuta de client atunci afiseaza mesajul de 
             * mai jos.
             */
            case BAD_CMD:
                printf("Bad input. Try again!\n");
            break;

            default: break;
        }
    }
    
    return 0;
}
