//
// vsprintf.cpp
// 
// Since the playstation doesn't support this extremely useful function
// I've decided to write my own, more fool me.
// It's not gonna be the best, of the most feature packed, but it will 
// provide a basic framework.
//			

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

char *vspf_getnum(char *s,int *len)
{
	int minus=0;
	*len=0;

	if (*s=='-')
		minus=1;
	while(isdigit(*s)) 
		*len=10**len+(*s++-'0');
	if (minus) 
		*len=-*len;
	return s;
}

int vsprintf(char *str,const char *fmt,va_list arg)
{
	char *s=fmt;
	char *p=str;
	int len;

	while(*s)
	{
		if (*s=='%')
		{
			len=0;
			s=vspf_getnum(s+1,&len);
			switch(*s++)
			{
			case 'd':
				p+=sprintf(p,"%*d",len,va_arg(arg,int));
				break;
			case 's':
				p+=sprintf(p,"%*s",len,va_arg(arg,char*));
				break;
			case 'c':
				*p++=va_arg(arg,char);
				break;
			case '%':
				*p++='%';
			}
		} else
			*p++=*s++;
	}
	*p=0;
	return ((int)p)-((int)str);
}