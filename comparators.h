int comp_lex(const void *a, const void *b)
{
   lex_t *p = (lex_t *) a;
   lex_t *q = (lex_t *) b;
   return strcmp(p->term, q->term);
}

int comp_stop(const void *a, const void *b)
{
   stop_t *p = (stop_t *) a;
   stop_t *q = (stop_t *) b;
   return strcmp(p->word, q->word);
}

int comp_stem(const void *a, const void *b)
{
   stem_t *p = (stem_t *) a;
   stem_t *q = (stem_t *) b;
   return strcmp(p->word, q->word);
}

int comp_docno(const void *a, const void *b)
{
   doc_t *p = (doc_t *) a;
   doc_t *q = (doc_t *) b;
   return strcmp(p->docno, q->docno);
}

int comp_utf8_stop(const void *a, const void *b)
{
   utf8Stop_t *p = (utf8Stop_t *) a;
   utf8Stop_t *q = (utf8Stop_t *) b;
   return wcscmp(p->word, q->word);
}

int comp_utf8_stem(const void *a, const void *b)
{
   utf8Stem_t *p = (utf8Stem_t *) a;
   utf8Stem_t *q = (utf8Stem_t *) b;
   return wcscmp(p->word, q->word);
}

int comp_utf8_lex(const void *a, const void *b)
{
   utf8Lex_t *p = (utf8Lex_t *) a;
   utf8Lex_t *q = (utf8Lex_t *) b;
   return wcscmp(p->term, q->term);
}

int comp_result(const void *a, const void *b)
{
   res_t *p = (res_t *) a;
   res_t *q = (res_t *) b;
   if(p->score > q->score)
      return -1;
   else
      return 1;   
}

int comp_invEntry_t(const void *p, const void *q)
{
  invEntry_t *p1, *q1;
  p1 = (invEntry_t *) p;
  q1 = (invEntry_t *) q;
  if(strcmp(p1->term, q1->term) < 0)
    return -1;
  else if(strcmp(p1->term, q1->term) == 0 && (p1->docid < q1->docid))  
    return -1;
  else
    return 1; 
}

int comp_termAndPos_t(const void *p, const void *q)
{
  termAndPos_t *p1, *q1;
  p1 = (termAndPos_t *) p;
  q1 = (termAndPos_t *) q;
  if(strcmp(p1->term, q1->term) < 0)
    return -1;
  else if(strcmp(p1->term, q1->term) == 0 && (p1->pos < q1->pos))  
    return -1;
  else
    return 1; 
}

int comp_utf8_inv_entry(const void *p, const void *q)
{
  utf8InvEntry_t *p1, *q1;
  p1 = (utf8InvEntry_t *) p;
  q1 = (utf8InvEntry_t *) q;
  if(wcscmp(p1->term, q1->term) < 0)
    return -1;
  else if(wcscmp(p1->term, q1->term) == 0 && (p1->docid < q1->docid))  
    return -1;
  else
    return 1; 
}


int comp_utf8_termAndPos(const void *p, const void *q)
{
  utf8TermAndPos_t *p1, *q1;
  p1 = (utf8TermAndPos_t *) p;
  q1 = (utf8TermAndPos_t *) q;
  if(wcscmp(p1->term, q1->term) < 0)
    return -1;
  else if(wcscmp(p1->term, q1->term) == 0 && (p1->pos < q1->pos))  
    return -1;
  else
    return 1; 
}

