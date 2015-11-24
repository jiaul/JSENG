int getUTFChar(FILE *fin) 
{
    static unsigned short mask[] = {192, 224, 240}; 
    unsigned short length, i, j; 
    int wc;
    char buf[8];
    off_t of;
    
    while(1) {
    	/* read first byte into buffer */
    	of = ftello(fin);
   	buf[0] = fgetc(fin);
   	if(feof(fin))
   	   return WEOF;
   	/* check how many more bytes need to be read for this utf-8 character */
   	length = 1;
    	if ((buf[0] & mask[0]) == mask[0]) length++;
   	if ((buf[0] & mask[1]) == mask[1]) length++;
    	if ((buf[0] & mask[2]) == mask[2]) length++;

    	/* read subsequent  bytes of the character */
    	j = 1;
    	while (j < length)
             buf[j++] = fgetc(fin);
    	
    	if(mbstowcs(&wc, buf, 1) != (size_t) -1)
    	   break;
    	else 
    	   fseeko(fin, of+1, 0);
    	
    } 
    
    return wc;
}


int get_utf8_token(FILE *fin, wchar_t *temp, int max_length)
{
    int i;
    wchar_t c;

    while (WEOF != (c = getUTFChar(fin)) && c != '<' && !iswalnum(c));

    if (c == WEOF) return 0;

    i = 0;
    temp[0] = towlower(c);
    if (c == L'<') { /* read till end of tag */
        while (WEOF != (c = getUTFChar(fin)) && c != L'>' && i < max_length - 3)
           if(!iswspace(c) && iswprint(c))
              temp[++i] = towlower(c);
        if (c == L'>') temp[++i] = c;
    }
    else { /* normal word */
        while (WEOF != (c = getUTFChar(fin)) && iswalnum(c) && i < max_length - 2)
            temp[++i] = towlower(c);
        if (WEOF != c) ungetwc(c, fin);
    }
    temp[++i] = L'\0';
    return i;
}


int get_token(FILE *fin, char *temp, int max_length)
{
    int c, i;

    while (EOF != (c = fgetc(fin)) && c != '<' && !isalnum(c));

    if (c == EOF) return 0;

    i = 0;
    temp[0] = tolower(c);
    if (c == '<') { /* read till end of tag */
        while (EOF != (c = fgetc(fin)) && c != '>' && i < max_length - 3)
           if(!isspace(c) && isprint(c))
              temp[++i] = tolower(c);
        if (c == '>') temp[++i] = c;
    }
    else { /* normal word */
        while (EOF != (c = fgetc(fin)) && isalnum(c) && i < max_length - 2)
            temp[++i] = tolower(c);
        if (EOF != c) ungetc(c, fin);
    }
    temp[++i] = '\0';
    return i;
}

int get_warc_token(FILE *fin, char *temp, int max_length, int *clc, int cl)
{
    int c, i = 0;

    while (EOF != (c = fgetc(fin)) && ++(*clc) <= cl && c != '<' && !isalnum(c));
    
    if(*clc > cl) return 0;
    if (c == EOF) return 0;

    temp[0] = tolower(c);
    if (c == '<') { /* read till end of tag */
        while (EOF != (c = fgetc(fin)) && ++(*clc) <= cl && c != '>' && i < max_length - 3) {
           if(!isspace(c) && isprint(c))
              temp[++i] = tolower(c);
        }    
        if (c == '>') temp[++i] = c;
    }
    else { /* normal word */
        while (EOF != (c = fgetc(fin)) && isalnum(c) && i < max_length - 2) {
            (*clc)++;
            temp[++i] = tolower(c);
        }    
        if (EOF != c) ungetc(c, fin);
    }
    temp[++i] = '\0';
    return i;
}
