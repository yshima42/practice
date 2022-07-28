#include <stdio.h>
#include <stdlib.h>

int main(void){

    printf("Content-type: text/plain\n\n");
    printf("%sから、%sでようこそ！",getenv("REMOTE_HOST"),getenv("HTTP_USER_AGENT"));

    return 1;

}

