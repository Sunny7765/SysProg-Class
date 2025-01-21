#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len) {
    int user_string_len = 0;
    char *p = user_str;
    int buff_index = 0;
    int space_count = 0;

    while (*p != '\0') {
        user_string_len++;
        p++;
    }

    p = user_str;

    while (*p == ' ' || *p == '\t') {
        p++;
    }

    while (*p != '\0' && buff_index < len) {
        char c = *p;

        if (c == ' ' || c == '\t' || c == '\n') {
            if (!space_count) {
                buff[buff_index++] = ' ';
                space_count = 1;
            }
        } else {
            buff[buff_index++] = c;
            space_count = 0;
        }
        p++;
    }

    if (buff_index > 0 && buff[buff_index - 1] == ' ') {
        buff_index--;
    }

    for (int i = buff_index; i < len; i++) {
        buff[i] = '.';
    }

    return user_string_len;
}

void reverse_buff(char *buff, int len, int str_len){
    char *start = buff;
    char *end = buff + str_len - 1;

    while (start < end) {
        char temp;
        temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}

void word_print(char *buff, int len) {
    int letter_count = 0;
    int word_count = 1;

    for (int i = 0; i < len; i++) {
        if (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\n') {
            if (letter_count > 0) {
                printf("(%d)\n", letter_count);
                letter_count = 0;
                word_count++;
            }
        } else {
            if (letter_count == 0) {
                printf("%d. ", word_count);
            }
            putchar(buff[i]);
            letter_count++;
        }
    }

    if (letter_count > 0) {
        printf("(%d)\n", letter_count);
    }
    printf("\nNumber of words returned: %d\n", word_count);
}

void print_buff(char *buff, int len){
    printf("Buffer:  [");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    printf("]\n");
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

int count_words(char *buff, int len, int str_len) {
    int word_count = 0;
    int in_word = 0;
    
    for (int i = 0; i < str_len; i++) {
        if (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\n') {
            in_word = 0;
        } else {
            if (in_word == 0) {
                word_count++;
            }
            in_word = 1;
        }
    }
    return word_count;
}

int search_replace(char *buff, int len, int str_len, char *search, char *replace) {
    int search_len = 0;
    int replace_len = 0;
    char *p = search;

    while (*p != '\0') {
        search_len++;
        p++;
    }

    p = replace;
    while (*p != '\0') {
        replace_len++;
        p++;
    }

    for (int i = 0; i <= str_len - search_len; i++) {
        if (strncmp(&buff[i], search, search_len) == 0) {
            if (str_len - search_len + replace_len > len) {
                return -1; 
            }

            if (replace_len != search_len) {
                memmove(&buff[i + replace_len], &buff[i + search_len], str_len - (i + search_len));
            }

            memcpy(&buff[i], replace, replace_len);

            str_len += (replace_len - search_len);

            if (str_len < len) {
                buff[str_len] = '\0';
            }

            return 0;
        }
    }
    return 0;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
int main(int argc, char *argv[]){
    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    // It is safe because argc < 2 checks for if there are less than two arguments. Having less than two arguments would mean that no extra arguments would be given after the program name. Then it checks argv[1] which is the argument after the program name, and makes sure it doesn't start with a -. This would check for invalid inputs. 
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }
    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = malloc(BUFFER_SZ * sizeof(char));
    if (buff == NULL) {
        printf("Error: Failed to allocate memory for buffer.\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d\n", user_str_len);
        free(buff);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }

            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            reverse_buff(buff, BUFFER_SZ,user_str_len);
            break;

        case 'w':
            printf("Word Print\n");
            printf("----------\n");
            word_print(buff, user_str_len);
            break;

        case 'x':
            if (argc < 5) {
                usage(argv[0]);
                free(buff);
                exit(1);
            }
            rc = search_replace(buff, BUFFER_SZ, user_str_len, argv[3], argv[4]);
            if (rc == -1) {
                printf("Error setting up buffer, error = -1\n");
                free(buff);
                exit(2);
            }
            break;

        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE
