lex_t *lexiconContainer;   // strores lexicon info (term, df, ctf, posting list offset)
doc_t *documentContainer;  // stores document info (docno, len, unique term, direct file offset)
stop_t *stopwordContainer; // stores stopwords
stem_t *stemContainer;     // stores word and stem from two column file
utf8Lex_t *utf8LexiconContainer;   // stores lexicon info (term, df, ctf, posting list offset)
utf8Stop_t *utf8StopwordContainer; // stores stopwords
utf8Stem_t *utf8StemContainer;     // stores word and stem from two column file
int numberOfStopword;     
int numberOfStem;
long long int numberOfDoc_coll;
long long int sizeOfLexicon;
long long int numberOfToken_coll;
float avgDocumentLen;
float avgUniqDocLen;
