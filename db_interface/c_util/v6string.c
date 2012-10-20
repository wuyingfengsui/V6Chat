#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "xmalloc.c"

/**
 * 连接字符串
 *
 * @param len the number of arguments to follow this parameter
 * @param ... string to escape and added (may be NULL)
 *
 * @return pointer to memory holding the concatenated 
 * strings - needs to be free(d) by the caller. or NULL
 * in case we ran out of memory.
 */
char *v6strcat(int len, ...) {
    va_list va;
    int i;
    char *rv = (char*) xmalloc(sizeof(char));
    *rv='\0';
    va_start(va, len);
    for(i=0;i<len;i++) {
        char *enc = va_arg(va, char *);
        int len;
		if(!enc) continue;
        len = strlen(enc) + 1;
        if(rv) len+=strlen(rv);
        rv=(char*) xrealloc(rv,len*sizeof(char));

        strncat(rv, enc, len);
  }
  va_end(va);
  return(rv);
}

/*
 * C字符串替换函数
 * 调用成功返回指向被替换后新字符串的指针失败返回NULL
 * 调用之后用free()释放
 */
char* replacestr(const char* str, const char* sstr, const char* dstr)
{
	if(str != NULL && sstr != NULL && dstr != NULL)
	{
		char* newstr = (char*)malloc(sizeof(char));
		char* curstr = (char*)str;
		int replacedlen = 0;
		int mstrlen = strlen(str);
		int sstrlen = strlen(sstr);
		int dstrlen = strlen(dstr);
		int leftlen = 0;

		*newstr = '\0';
		while(*curstr != '\0')
		{
			char* wanted = strstr(curstr, sstr);
			int rebytelen = wanted - curstr;

			if(wanted == NULL)
				break;

			int t = wanted - str;
			newstr = (char*)realloc(newstr, (replacedlen + rebytelen + dstrlen)*sizeof(char));
			memcpy(newstr + replacedlen, curstr, rebytelen*sizeof(char));
			replacedlen += rebytelen;
			curstr += rebytelen;

			memcpy(newstr + replacedlen, dstr, dstrlen*sizeof(char));
			replacedlen += dstrlen;
			curstr += sstrlen;
		}
		leftlen = mstrlen - (curstr - str) + 1;
		newstr = (char*)realloc(newstr, ( replacedlen + leftlen )*sizeof(char));
		memcpy(newstr + replacedlen, curstr, leftlen);

		return newstr;
	}

	return NULL;
}