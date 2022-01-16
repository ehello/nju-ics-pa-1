#include <sys/types.h>
#include <stdio.h>
#include <regex.h>

regex_t re;
char regex1[] = "[[:alnum:]]+";  /* match [0-9a-zA-Z]+ */
char regex2[] = "\\(";           /* match (            */
char regex2[] = "[[:digit:]]+";  /* match [0-9]+       */
char match[] = "9sdfsdf";
regmatch_t pmatch;

int main()
{
    int ret = regcomp(&re, regex1, REG_EXTENDED);
    char error_msg[128];
    if (ret != 0) {
        regerror(ret, &re, error_msg, 128);
        printf("regex compilation failed: %s\n%s", error_msg, regex1);
    }

    if (regexec(&re, match, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = match;
        int substr_len = pmatch.rm_eo;
        printf("match rule \"%s\" len %d: %s\n", regex1, substr_len, substr_start);
    }

    regfree(&re);

    return 0;
}