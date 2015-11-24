int isAcceptedField(char *buf, char *qf)
{
        char *p[1024], fields[1024];
        int i, n, l, l1;
        l = strlen(buf);
        strcpy(fields, qf);
        l1 = strlen(fields);
        for(i = 0, n = 1, p[0] = fields; i < l1; i++) {
                if(fields[i] == ',') {
                        p[n++] = fields+i+1;
                        fields[i] = '\0';
                }
         } 
         
        if(buf[0] == '<' && buf[l-1] == '>') {
                for(i = 0; i < n; i++) {
                        if(!strncmp(buf+1, p[i], l-2))
                                return 1;
                }                  
        }
        
        return 0;
}

int read_queries(FILE *fin, query_t *Q, char *queryFields, char *stemFlag)
{
  int i = -1, j, k, n, insideQuery = 0, acceptText = 0, max_term_length = 30, termBufSize = 0;
  char BUF[1024], term_buf[1000][30];
  char qno[128];
  float qtf[1000];
  stop_t st;
  
  while(get_token(fin, BUF, max_term_length) > 0) {
  
     if(strcmp(BUF, "<top>") == 0) {
        insideQuery = 1;
        n = 0;
        i++;
        continue;
     }   
       
     if((insideQuery) && !strcmp(BUF, "<num>")) {
        get_token(fin, BUF, max_term_length);
        Q[i].no = atoi(BUF);
        continue;
     }
     
     if((insideQuery) && isAcceptedField(BUF, queryFields)) { 
        acceptText = 1;
        continue;
     }
     
     else if(!strcmp(BUF, "</top>")) { /* reading one query done */
        insideQuery = 0;
        acceptText = 0;
        Q[i].no_terms = n;
        Q[i].terms = (char **) malloc(sizeof(char *) * n);
        Q[i].qtf = (float *) malloc(sizeof(float) * n);
        for(j = 0; j < n; j++) {
           Q[i].terms[j] = strdup(term_buf[j]);
           Q[i].qtf[j] = qtf[j];
        }
        n = 0;
        continue;
     }
     
     else if((acceptText) && BUF[0] == '<' && BUF[strlen(BUF) - 1] == '>') {
        insideQuery = 1;
        acceptText = 0;
        continue;
     }
     
     else ;
     
     if(acceptText) {
        st.word = BUF;
       if(bsearch(&st, stopwordContainer, numberOfStopword, sizeof(stop_t), comp_stop) != NULL)
         continue;
         
       if(strcmp(stemFlag, "y") == 0) {         /* perform stemming */
          porter_stem(BUF);  
       }                      
         
        for(j = 0; j < n; j++)
           if(strcmp(term_buf[j], BUF) == 0)
              break;
        if(j == n) {
           qtf[j] = 1;
           BUF[max_term_length-1] = '\0';
           strcpy(term_buf[j], BUF);
           n++;
        }  
        else
           qtf[j] += 1;         
     }
  }
  
  return i+1;
}


int read_utf8_queries(FILE *fin, utf8Query_t *Q, char *queryFields)
{
  int i = -1, j, k, n, insideQuery = 0, acceptText = 0, max_term_length = 30, termBufSize = 0;
  wchar_t BUF[1024], term_buf[1000][30];
  char qno[128], temp_buf[30];
  float qtf[1000];
  utf8Stop_t st;
    
  while(get_utf8_token(fin, BUF, max_term_length) > 0) {
  
     if(wcscmp(BUF, L"<top>") == 0) {
        insideQuery = 1;
        n = 0;
        i++;
        continue;
     }   
       
     if((insideQuery) && !wcscmp(BUF, L"<num>")) {
        get_utf8_token(fin, BUF, max_term_length);
        wcstombs(qno, BUF, 1024);
        Q[i].no = atoi(qno);
        continue;
     }
     
     wcstombs(temp_buf, BUF, 1000);
     if((insideQuery) && isAcceptedField(temp_buf, queryFields)) { 
        acceptText = 1;
        continue;
     }
     
     else if(!wcscmp(BUF, L"</top>")) { /* reading one query done */
        insideQuery = 0;
        acceptText = 0;
        Q[i].no_terms = n;
        Q[i].terms = (wchar_t **) malloc(sizeof(wchar_t *) * n);
        Q[i].qtf = (float *) malloc(sizeof(float) * n);
        for(j = 0; j < n; j++) {
           Q[i].terms[j] = wcsdup(term_buf[j]);
           Q[i].qtf[j] = qtf[j];
        }
        n = 0;
        continue;
     }
     
     else if((acceptText) && BUF[0] == L'<' && BUF[wcslen(BUF) - 1] == L'>') {
        insideQuery = 1;
        acceptText = 0;
        continue;
     }
     
     else ;
     
     if(acceptText) {
        st.word = BUF;
        //printf("%ls\n", BUF);
        //if(bsearch(&st, stopword, stop_size, sizeof(utf8Stop_t), comp_stop) != NULL)
          //continue;
       /*sm.word = BUF;
        if((sptr = bsearch(&sm, stemmer, stem_size, sizeof(stem_t), comp_stem)) != NULL) 
           strcpy(BUF, sptr->stem); */
       //porter_stem(BUF);                       /* perform stemming */
         
        for(j = 0; j < n; j++)
           if(wcscmp(term_buf[j], BUF) == 0)
              break;
        if(j == n) {
           qtf[j] = 1;
           BUF[max_term_length-1] = L'\0';
           wcscpy(term_buf[j], BUF);
           n++;
        }  
        else
           qtf[j] += 1;         
     }
  }
  
  return i+1;
}


int readSingleLineQuery(FILE *f, query_t *Q)
{
   int i, j, c, n;
   char buf[1024];
   float wt;
   off_t qof;
   i = 0;
   while(1) {
      if(fscanf(f, "%d", &Q[i].no) != 1) {
         printf("num : Wrong format query file\n");
         return 0;
      }  
      qof = ftello(f);
      n = 0;
      while(1) {
         if(fscanf(f, "%f,%s", &wt, buf) != 2) {
            printf("term : Wrong format query file\n");
            return 0;
         }
         n++;   
         while((c = fgetc(f)) != EOF) {
            if(!isblank(c)) {
               ungetc(c, f);
               break;
            }
         }
         if(c == '\n' || c == EOF) {
            fseeko(f, qof, 0);
            Q[i].no_terms = n;
            Q[i].terms = (char **) malloc(sizeof(char *) * n);
            Q[i].qtf = (float *) malloc(sizeof(float) * n);
            for(j = 0; j < n; j++) {
               fscanf(f, "%f,%s", &Q[i].qtf[j], buf);
               Q[i].terms[j] = strdup(buf);
            }   
            i++;   
            while((c = fgetc(f)) != EOF) 
              if(isalnum(c)) {
                 ungetc(c, f);
                 break;
              }
                 
            if(c == EOF)
               return i; 
            else     
               break;   
         }   
      } 
   }
}

