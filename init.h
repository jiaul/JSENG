void initializeRetrievalEnvironment(char *indexname, char *stopfilename)
{
  char temp[128], lexfile[128], docfile[128], statfile[128], invfile[128], word[128], indexPath[256];
  FILE *tempfp;
  off_t sizeOfInvFile;
  long long int temp_sum;
  int i;
  
  sprintf(indexPath, "indexes/%s/%s", indexname, indexname);
  sprintf(statfile, "%s.stat", indexPath);
  sprintf(lexfile, "%s.lex", indexPath);
  sprintf(docfile, "%s.docid", indexPath);
  sprintf(invfile, "%s.index", indexPath);
  
  //reading collection statistics
  i = 0;
  tempfp = FOPEN(statfile, "r");
  while(fgets(temp, 1024, tempfp) > 0) {
    i++;
    if(!strncmp(temp, "lexSize", 7))
      sscanf(temp, "%*s%Ld", &sizeOfLexicon); 
    else if(!strncmp(temp, "numDoc", 6))
      sscanf(temp, "%*s%Ld", &numberOfDoc_coll);   
    else if(!strncmp(temp, "colLen", 6))
      sscanf(temp, "%*s%Ld", &numberOfToken_coll);
    else if(!strncmp(temp, "avgDocLen", 8))
      sscanf(temp, "%*s%f", &avgDocumentLen); 
    else {
      printf("Unrecognized property in line %d of %s file\n", i, statfile);    
      return; 
    }   
  } 
  fclose(tempfp);
  
  /* Reading lexicon statistics */
  tempfp = FOPEN(lexfile, "r");
  if((lexiconContainer = (lex_t *) malloc(sizeof(lex_t) * (sizeOfLexicon + 1))) == NULL)
     printf("Could not allocate memory for utf8LexiconContainer\n");    
  for(i = 0; fscanf(tempfp, "%s%d,%d,%Ld", temp, &lexiconContainer[i].df, &lexiconContainer[i].ctf, (long long int *)&lexiconContainer[i].of) > 0; i++) {
     lexiconContainer[i].term = strdup(temp);
     lexiconContainer[i].term[strlen(temp) - 1] = '\0'; // remove trailing comma
  } 
  fclose(tempfp);  
  
  // determine the size of the inverted index file: the last entry of lexiconContainer
  tempfp = FOPEN(invfile, "r");
  fseeko(tempfp, 0L, SEEK_END);
  sizeOfInvFile = ftello(tempfp);
  lexiconContainer[sizeOfLexicon].of = sizeOfInvFile - 1;
  fclose(tempfp);   
  
  /* Reading document statistics */
  tempfp = FOPEN(docfile, "r");
  if((documentContainer = (doc_t *) malloc(sizeof(doc_t) * (numberOfDoc_coll + 1))) == NULL)
     printf("Could not allocate memory for documentContainer information\n"); 
  temp_sum = 0;
  for(i = 0; fscanf(tempfp, "%s%Ld%d%d%Ld", temp, &documentContainer[i].id, &documentContainer[i].ut, &documentContainer[i].len, (long long int *)&documentContainer[i].directOfset) > 0; i++) {
     documentContainer[i].docno = strdup(temp);
     temp_sum += documentContainer[i].ut;
  }
  fclose(tempfp); 
  avgUniqDocLen = (1.0 * temp_sum)/numberOfDoc_coll;
     
  /* reading stopwords */
  tempfp = FOPEN(stopfilename, "r");
  for(numberOfStopword = 0; fscanf(tempfp, "%s", word) > 0; numberOfStopword++) ;
  rewind(tempfp); 
  if((stopwordContainer = (stop_t *) malloc(numberOfStopword * sizeof(stop_t))) == NULL)
     printf("Could not allocate memory for stopwords\n");
  for(numberOfStopword = 0; fscanf(tempfp, "%s", word) > 0; numberOfStopword++)
     stopwordContainer[numberOfStopword].word = strdup(word);
  qsort(stopwordContainer, numberOfStopword, sizeof(stop_t), comp_stop);
  fclose(tempfp);
  
}

void initializeUtf8RetrievalEnvironment(char *indexname, char *stopfilename, char *stem2colfilename)
{
  char temp[128], lexfile[128], docfile[128], statfile[128], invfile[128];
  wchar_t word[128], stem[128], term_buf[128];
  FILE *tempfp;
  off_t sizeOfInvFile;
  int i;
  
  sprintf(statfile, "%s.stat", indexname);
  sprintf(lexfile, "%s.lex", indexname);
  sprintf(docfile, "%s.docid", indexname);
  sprintf(invfile, "%s.index", indexname);
  
  //reading collection statistics
  i = 0;
  tempfp = FOPEN(statfile, "r");
  while(fgets(temp, 1024, tempfp) > 0) {
    i++;
    if(!strncmp(temp, "lexSize", 7))
      sscanf(temp, "%*s%Ld", &sizeOfLexicon); 
    else if(!strncmp(temp, "numDoc", 6))
      sscanf(temp, "%*s%Ld", &numberOfDoc_coll);   
    else if(!strncmp(temp, "colLen", 6))
      sscanf(temp, "%*s%Ld", &numberOfToken_coll);
    else if(!strncmp(temp, "avgDocLen", 8))
      sscanf(temp, "%*s%f", &avgDocumentLen); 
    else {
      printf("Unrecognized property in line %d of %s file\n", i, statfile);    
      return; 
    }   
  } 
  fclose(tempfp);
  
  /* Reading lexicon statistics */
  tempfp = FOPEN(lexfile, "r");
  if((utf8LexiconContainer = (utf8Lex_t *) malloc(sizeof(utf8Lex_t) * (sizeOfLexicon + 1))) == NULL)
     printf("Could not allocate memory for utf8LexiconContainer\n");    
  for(i = 0; fscanf(tempfp, "%ls%d,%d,%Ld", term_buf, &utf8LexiconContainer[i].df, &utf8LexiconContainer[i].ctf, (long long int *)&utf8LexiconContainer[i].of) > 0; i++) {
     utf8LexiconContainer[i].term = wcsdup(term_buf);
     utf8LexiconContainer[i].term[wcslen(term_buf) - 1] = L'\0'; // remove trailing comma
  } 
  fclose(tempfp);  
  
  // determine the size of the inverted index file: the last entry of utf8LexiconContainer
  tempfp = FOPEN(invfile, "r");
  fseeko(tempfp, 0L, SEEK_END);
  sizeOfInvFile = ftello(tempfp);
  utf8LexiconContainer[sizeOfLexicon].of = sizeOfInvFile - 1;
  fclose(tempfp);   
  
  /* Reading document statistics */
  tempfp = FOPEN(docfile, "r");
  if((documentContainer = (doc_t *) malloc(sizeof(doc_t) * (numberOfDoc_coll + 1))) == NULL)
     printf("Could not allocate memory for documentContainer information\n"); 
  for(i = 0; fscanf(tempfp, "%s%Ld%d%d%Ld", temp, &documentContainer[i].id, &documentContainer[i].ut, &documentContainer[i].len, (long long int *)&documentContainer[i].directOfset) > 0; i++)
     documentContainer[i].docno = strdup(temp);
  fclose(tempfp);    
     
  /* reading stopwords */
  tempfp = FOPEN(stopfilename, "r");
  for(numberOfStopword = 0; fscanf(tempfp, "%ls", word) > 0; numberOfStopword++) ;
  rewind(tempfp); 
  if((utf8StopwordContainer = (utf8Stop_t *) malloc(numberOfStopword * sizeof(utf8Stop_t))) == NULL)
     printf("Could not allocate memory for stopwords\n");
  for(numberOfStopword = 0; fscanf(tempfp, "%ls", word) > 0; numberOfStopword++)
     utf8StopwordContainer[numberOfStopword].word = wcsdup(word);
  qsort(utf8StopwordContainer, numberOfStopword, sizeof(utf8Stop_t), comp_utf8_stop);
  fclose(tempfp);
  
  /* reading stem information */
  tempfp = FOPEN(stem2colfilename, "r");
  for(numberOfStem = 0; fscanf(tempfp, "%ls%ls", word, stem) > 0; numberOfStem++) ;
  rewind(tempfp);
  if((utf8StemContainer = (utf8Stem_t *) malloc(numberOfStem * sizeof(utf8Stem_t))) == NULL)
     printf("Could not allocate memory for stemming\n");
  for(numberOfStem = 0; fscanf(tempfp, "%ls%ls", word, stem) > 0; numberOfStem++) {
     word[MAX_TERM_LEN - 1] = L'\0';
     stem[MAX_TERM_LEN - 1] = L'\0';
     utf8StemContainer[numberOfStem].word = wcsdup(word);
     utf8StemContainer[numberOfStem].stem = wcsdup(stem);
  }
  qsort(utf8StemContainer, numberOfStem, sizeof(utf8Stem_t), comp_utf8_stem); 
  fclose(tempfp);
  
}

