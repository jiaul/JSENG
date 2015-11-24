int read_utf8_queries(FILE *fin, utf8Query_t *Q)
{
  int i = -1, j, n, *qtf, maxQueryLen = 10000, insideQuery = 0, acceptText = 0, max_term_length = 30, termBufSize = 0;
  wchar_t BUF[1024], **term_buf;
  char qno[128];
  utf8Stop_t st;
  
  term_buf = (wchar_t **) malloc(maxQueryLen * sizeof(wchar_t *));
  qtf = (int *) malloc(maxQueryLen * sizeof(int *));
  
  if((qtf == NULL) || (term_buf == NULL)) {
    printf("Could not allocate memory for query term buffer\n");
    exit(0);
  }    
  
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
     
     if((insideQuery) && !wcscmp(BUF, L"<title>")) {  /* insert here query field tags */
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
     
     else if((acceptText) && BUF[0] == '<' && BUF[wcslen(BUF) - 1] == '>') {
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
           term_buf[j] = wcsdup(BUF);
           n++;
        }  
        else
           qtf[j] += 1;         
     }
  }
  
  for(j = 0; j < maxQueryLen; j++);
    //free(term_buf[j]);
  free(term_buf);
  free(qtf);  
  
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

