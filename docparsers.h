
int get_trec_doc(FILE *fin, document_t *doc)
{
  int i;
  char TEMP_BUF[BUF_SIZE];
  int insideDoc, acceptText;
  stop_t st;
  stem_t sm, *sptr;
  doc->len = 0; insideDoc = 0; acceptText = 0;
  
  while(get_token(fin, TEMP_BUF, BUF_SIZE) > 0) {
  
     if(!strcmp(TEMP_BUF, "<doc>")) {
        insideDoc = 1;
        doc->len = 0;
        continue;
     }
     
     if(!strcmp(TEMP_BUF, "</doc>")) {
        insideDoc = 0;
        return 1;
     }
     
     if(insideDoc && !strcmp(TEMP_BUF, "<docno>")) {
        i = 1;
        while (EOF != (doc->no[0] = fgetc(fin)) && isspace(doc->no[0]));
        while (EOF != (doc->no[i] = fgetc(fin)) && doc->no[i] != '<' && !isspace(doc->no[i])) i++;
        ungetc(doc->no[i], fin);
        doc->no[i] = '\0';
        acceptText = 1;
        continue;
     }
     
     if(TEMP_BUF[0] == '<' || TEMP_BUF[strlen(TEMP_BUF) - 1] == '>')   // ignore other tags
        continue;
        
     if(acceptText) {
           TEMP_BUF[MAX_TERM_LEN - 1] = '\0';
           st.word = TEMP_BUF;
           if(bsearch(&st, stopword, stop_size, sizeof(stop_t), comp_stop) == NULL) {
             /* sm.word = TEMP_BUF;
              if((sptr = bsearch(&sm, stemmer, stem_size, sizeof(stem_t), comp_stem)) == NULL)
                strcpy(doc.TERM[doc->len].term, TEMP_BUF);
              else
                strcpy(doc.TERM[doc->len].term, sptr->stem); */
              if(STEM_FLAG)
                 porter_stem(TEMP_BUF);                                        // Insert here stemmer
              if(strlen(TEMP_BUF)> 0)
                strcpy(doc->terms[doc->len].term, TEMP_BUF);
              doc.TERM[doc->len].pos = doc->len;
              if(doc->len < MAX_DOC_LEN-2)
              doc->len++;
           }  
     }   
  }  
  return doc->len;
}


int get_utf8_trec_doc(FILE *fin, utf8Document_t *doc)
{
  int insideDoc, acceptText, i;
  wchar_t TEMP_BUF[BUF_SIZE];
  utf8Stop_t st;
  utf8Stem_t sm, *sptr;
  doc->len = 0; insideDoc = 0; acceptText = 0;
  
  while(get_utf8_token(fin, TEMP_BUF, BUF_SIZE) > 0) {
  
     if(!wcscmp(TEMP_BUF, L"<doc>")) {       // enters into a document
        insideDoc = 1;
        doc->len = 0;
        acceptText = 0;
        continue;
     }
     
     if(!wcscmp(TEMP_BUF, L"</doc>")) {    //  comes out of a document
        insideDoc = 0;
        return 1;
     }
     
     if(insideDoc && !wcscmp(TEMP_BUF, L"<docno>")) {     // read document number for the current document
        i = 1;
        while (EOF != (doc->no[0] = getUTFChar(fin)) && iswspace(doc->no[0]));
        while (EOF != (doc->no[i] = getUTFChar(fin)) && doc->no[i] != L'<' && !iswspace(doc->no[i])) i++;
        ungetwc(doc->no[i], fin);
        doc->no[i] = L'\0';
        acceptText = 1;
        continue;
     }
     
     if(TEMP_BUF[0] == L'<' || TEMP_BUF[wcslen(TEMP_BUF) - 1] == L'>')    // ignore other tags
        continue;
        
     if(acceptText) {                                                     //  store document text
        TEMP_BUF[MAX_TERM_LEN - 1] = L'\0';
        st.word = TEMP_BUF;
        if(bsearch(&st, stopword, stop_size, sizeof(utf8Stop_t), comp_utf8_stop) != NULL)
           continue;
        /* sm.word = TEMP_BUF;
        if((sptr = bsearch(&sm, stemmer, stem_size, sizeof(utf8Stem_t), comp_utf8_stem)) == NULL)
           strcpy(doc.TERM[doc->len].term, TEMP_BUF);
        else
           strcpy(doc.TERM[doc->len].term, sptr->stem); */
        // if(STEM_FLAG)
             //porter_stem(TEMP_BUF);                                        // Insert here stemmer
        if(wcslen(TEMP_BUF) <= 0)
           continue;
        wcscpy(doc->terms[doc->len].term, TEMP_BUF);
        doc->terms[doc->len].pos = doc->len;
        if(doc->len < MAX_DOC_LEN -2)
           doc->len++;
     }   
  }  
  return doc->len;
}

int get_warc_doc(FILE *fin, document_t *doc)
{
  int i, c, cl, ConLen;
  int InsideDoc, ConLenCount;
  stop_t st;
  stem_t sm, *sptr;
  char TEMP_BUF[BUF_SIZE];
  doc->len = 0;               // set document length to zero
  doc->no[0] = '\0';          // set document number to null
  InsideDoc = 0; ConLenCount = 0;
  cl = 0;                    // content length counter to know start of document
  
  while(1) {                                        // look for TREC docid
    if(fgets(TEMP_BUF, BUF_SIZE, fin) == NULL)
       return 0;
    if(!strncmp(TEMP_BUF, "WARC-TREC-ID", 12)) {
       sscanf(TEMP_BUF, "WARC-TREC-ID:%s", doc->no);
       break;
    }
  } 
  
  while(cl != 2) {                                // look for content length
     if(fgets(TEMP_BUF, BUF_SIZE, fin) == NULL)
       return 0;
     if(sscanf(TEMP_BUF, "Content-Length:%d", &ConLen) == 1)
       cl++;
  }
  
  if(cl == 2 && strlen(doc->no) > 0)                // entered inside a document
     InsideDoc = 1;
     
  while(get_warc_token(fin, TEMP_BUF, BUF_SIZE, &ConLenCount, ConLen) > 0) {
     
     if(TEMP_BUF[0] == '<' || TEMP_BUF[strlen(TEMP_BUF) - 1] == '>') {          // ignore content within tag
        if(ConLenCount >= ConLen)
          return InsideDoc;
        else        
          continue;
     }
       
     if(InsideDoc) {                                      // store document text content
           TEMP_BUF[MAX_TERM_LEN - 1] = '\0';
           st.word = TEMP_BUF;
           if(bsearch(&st, stopword, stop_size, sizeof(stop_t), comp_stop) == NULL) {
              if(STEM_FLAG)
                 porter_stem(TEMP_BUF);                                        // Insert here stemmer
              if(strlen(TEMP_BUF) > 0)
                strcpy(doc.TERM[doc->len].term, TEMP_BUF);
              doc->terms[doc->len].pos = doc->len;
              if(doc->len < MAX_DOC_LEN -2)
                doc->len++;
           }   
     }
     
     if(ConLenCount >= ConLen)                                               // end of a document reached
        return InsideDoc;   
  }  
  return InsideDoc;
}


