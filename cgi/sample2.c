#include <stdio.h>

int main()
{
        int     chr;
        int     flag;

        printf("Content-type: text/html\n\n");
        printf("<HTML><BODY>\n");
        printf("name.cgi<BR>\n");

        flag = 0;
        while( flag == 0 )
        {
                /* 渡されたデータを標準入力より取得する */
                chr = getchar();
                /* 渡されたデータをそのまま表示する */
                putchar(chr);
                /* データ終了のチェック */
                if( (chr == EOF) || (chr == 0x00) || (chr == '\n') )
                {
                        flag = 1;
                }
        }
        printf("</BODY></HTML>\n");
}
