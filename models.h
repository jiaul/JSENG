
float standardIDF(int df, long long int numOfDoc)
{
  double idf;
  idf = (double) ((1.0 * numOfDoc + 0.5)/(df + 0.5));
  return MAX(0.0, (float) log(idf));
}

float bm25IDF(int df, long long int numOfDoc)
{
  double idf;
  idf = (double) ((1.0 * numOfDoc - df + 0.5)/(df + 0.5));
  return MAX(0.0, (float) log(idf));
}


float IFB2(modelVariables_t *v)
{
  float ntf, f1, f2, score;
  ntf = 1.0 * v->tf * log2(1.0 + v->param * (v->avgDocLen/v->docLen));
  f1 = (1.0 * v->ctf + 1.0) / (1.0 * v->df * (ntf + 1.0));
  f2 = ntf * (float) log((1.0 * v->numOfDoc + 1.0) / (1.0 * v->ctf + 0.5));
  score = f1 * f2;
  
  return MAX(score, 0.0) * v->qtf;
}

float PL2(modelVariables_t *v)
{
  float ntf, f1, score, atf;
  
  atf = (float)(1.0 * v->ctf)/v->numOfDoc;
  ntf = 1.0 * v->tf * log2(1.0 + v->param * (v->avgDocLen/v->docLen));
  f1 = ntf * log2(ntf / atf) + log2(exp(1.0)) * (1.0 * atf + 1.0/(12 * ntf) - ntf) + 0.5 * log2(44.0/7.0 * ntf);
  score = f1/(1.0 + ntf);
  
  return MAX(score, 0.0) * v->qtf;
}


float TFIDF(modelVariables_t *v)
{
  float score, idf, ntf;
  
  idf = v->standardIDF;
  ntf = (1.0 + log(1.0 + log(1.0 * v->tf)))/(1.0 - v->param + (v->param * v->docLen)/v->avgDocLen);
  score = ntf * idf * v->qtf;
  
  return score;
  
}


float LM_DIR(modelVariables_t *v)
{
   float score;
   long double smooth;
   smooth = (long double)(v->param * v->colProbOfTerm);
   score = (float) logl((v->tf + smooth)/(1.0 * v->docLen + v->param));
   return score * v->qtf;
}


float LM_JM(modelVariables_t *v)
{
   float score;
   long double colProb, docProb;
   colProb = v->colProbOfTerm;
   docProb = (long double) ((1.0 * v->tf)/v->docLen);
   score = (float) logl(v->param * docProb + (1.0 - v->param) * colProb);
   return score * v->qtf;
}

float BM25(modelVariables_t *v)
{
  float score, idf, k1, k3;
  k1 = 1.2; k3 = 7.0;
  idf = v->bm25IDF;
  score = (((k1+1.0) * v->tf)/(k1 * (1.0 - v->param) + (v->param * v->docLen)/v->avgDocLen + v->tf)) * (((k3 + 1.0) * v->qtf)/(k3 + v->qtf)) * idf;
  return score;
  
}

float transformTF(float tf)
{
   float etf;
   etf = tf/(1.0+tf);
   return  etf;
}

float MATF(modelVariables_t *v)
{
   float idf, lf = 0.5, mdtf, mctf, ntf, ritf, lrtf;
   
   lf = 2.0/(1.0 + log2(1.0 + v->qlen));
   idf = v->standardIDF; 
   mctf = (1.0 * v->ctf)/v->df;  
   mctf = mctf/(1.0 + mctf);
   
   mdtf = (1.0 * v->docLen)/v->numDocUniqTerm;
   ntf = log2(1.0 + v->tf)/log2(1.0 + mdtf); 
   ritf = transformTF(ntf);

   ntf = 1.0 * v->tf * log2(1.0 + (v->avgDocLen/v->docLen));
   lrtf = transformTF(ntf);
   
   return (lf * ritf * idf+ (1.0-lf) * lrtf * idf) * mctf * v->qtf;
}

