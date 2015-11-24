#define _FILE_OFFSET_BITS 64
#define BUF_SIZE 1024
#define QUERY_SIZE 20000

#include "constants.h"
#include "common.h"
#include "structures.h"
#include "globalvars.h"
#include "comparators.h"
#include "stemmer.h"
#include "utility.h"
#include "tokenizer.h"
#include "heap.h"
#include "models.h"
#include "init.h"
#include "queryParsers.h"


int main(int argc, char *argv[])
{
  FILE *fpinv, *fpquery, *fpout;
  lex_t *ptr, key;
  int j, l, numOfQuery, qlen, smoothReq;
  long long int i, k, docid, pe, PostingBufSize = 0, postingPos, PostingLength;
  long long int l1, l2, m, curResSetSize, queryTermCtf[1024];
  char invfile[1024], temp[1024], output[1024], BUF[1024], runid[1024];
  float termScore, param;
  unsigned char *postingBuffer = NULL;
  float (* retrievalModel) (modelVariables_t *); // model function pointer
  modelVariables_t V, smoothV;
  res_t *result, *tempResult, *temptr;
  query_t query[QUERY_SIZE];
  
  if(argc - 1 == 6) {               // setting model paramater to default value
    if(!strcmp(argv[4], "TFIDF"))
      param = 0.05;
    if(!strcmp(argv[4], "BM25"))
      param = 0.3;
    if(!strcmp(argv[4], "LM_DIR"))
      param = 1000.0;
    if(!strcmp(argv[4], "MATF"))
      param = 1.0;
    if(!strcmp(argv[4], "PL2"))
      param = 4.0;  
    if(!strcmp(argv[4], "IFB2"))
      param = 7.0; 
    if(!strcmp(argv[4], "LM_JM"))
      param = 0.7;   
  }
  else if(argc - 1 == 7)
     param = atof(argv[7]);
  else {
     printf("1. Give index name\n2. TREC format Query file\n3. Apply stemming? (y/n)\n");
     printf("4. Model name (LM_DIR, LM_JM, PL2, IFB2, MATF, BM25, TFIDF)\n");
     printf("5. stopword file\n6. Query fields\n7. Model Parameter value (optional)\n");
     return 0;
  }   
  
  if(!strcmp(argv[4], "LM_DIR")) retrievalModel = LM_DIR;
  else if(!strcmp(argv[4], "LM_JM")) retrievalModel = LM_JM;
  else if(!strcmp(argv[4], "PL2")) retrievalModel = PL2;
  else if(!strcmp(argv[4], "IFB2")) retrievalModel = IFB2;
  else if(!strcmp(argv[4], "BM25")) retrievalModel = BM25;
  else if(!strcmp(argv[4], "TFIDF")) retrievalModel = TFIDF;
  else if(!strcmp(argv[4], "MATF")) retrievalModel = MATF;  
  else {
    printf("Unrecognized model: %s\n", argv[4]);
    return 0;
  }  
  if(!strncmp(argv[4], "LM", 2))
  	smoothReq = 1;
  else
  	smoothReq = 0;
  smoothV.tf = 0;
  smoothV.param = param;
  
  sprintf(runid, "%s-%.2f", argv[4], param);
  //read all information from index for retrieval
  initializeRetrievalEnvironment(argv[1], argv[5]);
  
  sprintf(invfile, "indexes/%s/%s.index", argv[1], argv[1]);
  
  if((fpinv = fopen(invfile, "r")) == NULL) {
  	printf("Could not open inverted index <%s>\n", invfile);
  	return 0;
  }
  
  /* reading query */
  fpquery = FOPEN(argv[2], "r");
  numOfQuery = read_queries(fpquery, query, argv[6], argv[3]); 
  fclose(fpquery);
  
  if((result = (res_t *) malloc(sizeof(res_t) * numberOfDoc_coll)) == NULL) {
     printf("Could not allocate memory for result buffer\n");
     return 0;
  }
  if ((tempResult = (res_t *) malloc(sizeof(res_t) * numberOfDoc_coll)) == NULL) {
     printf("Could not allocate memory for temp result buffer\n");
     return 0;	
  }
  
   /* Matching begins */
  for(i = 0; i < numOfQuery; i++) {
     qlen = query[i].no_terms;
     curResSetSize = 0;                         
     for(j = 0; j < query[i].no_terms; j++) {
     
          key.term = query[i].terms[j];
          if((ptr = bsearch(&key, lexiconContainer, sizeOfLexicon, sizeof(lex_t), comp_lex)) == NULL) {
          	printf("not found %s \n", query[i].terms[j]);
          	queryTermCtf[j] = 0;
                continue;
          }
          queryTermCtf[j] = ptr->ctf;
          PostingLength = (ptr+1)->of - ptr->of;
          if(PostingBufSize < PostingLength) {
          	if((postingBuffer = (unsigned char *) realloc(postingBuffer, PostingLength)) == NULL) {
                	printf("Could not re-allocate memory for Posting list\n");
                	return 0;
                }
               PostingBufSize = PostingLength;
          }
             
          fseeko(fpinv, ptr->of, 0);
          if(fread(postingBuffer, 1, PostingLength, fpinv) != PostingLength)
               printf("Warning: Posting list reading error for term %s\n", query[i].terms[j]);
          postingPos = 0; docid = 0;
          //set term and collection specific retrieval model variables
          V.numOfDoc = numberOfDoc_coll; V.numOfColTerm = numberOfToken_coll; V.df = ptr->df; V.ctf = ptr->ctf; V.qtf = query[i].qtf[j];
          V.colProbOfTerm = (1.0 * ptr->ctf)/numberOfToken_coll; V.standardIDF = standardIDF(ptr->df, numberOfDoc_coll); 
          V.bm25IDF = bm25IDF(ptr->df, numberOfDoc_coll);
          V.avgDocLen = avgDocumentLen; V.qlen = qlen; V.param = param;
          l1 = 0; l2 = 0; m = 0;
          while(l2 < ptr->df) {
                long long int temp;                                       // temporary variable for storing docid difference and freq.
                postingPos += deCode(postingBuffer + postingPos, &temp);  // decoding docid gap
                docid = docid + temp;
                postingPos += deCode(postingBuffer + postingPos, &temp);  // decoding tf 
                // set document specific retrieval model variables 
                V.tf = (int) temp; V.docLen = documentContainer[docid].len; V.numDocUniqTerm = documentContainer[docid].ut;
                //compute weight of a term in a document
                termScore = retrievalModel(&V);
                if (l1 == curResSetSize) {
                	tempResult[m].docid = docid;
                	tempResult[m].score = termScore;
                	// required only for language model smoothing
                	if (smoothReq) {
                		for(l = 0; l < j; l++) {
                			if(queryTermCtf[l] == 0) 
                				continue;
                			smoothV.colProbOfTerm = (1.0 * queryTermCtf[l])/numberOfToken_coll;
                			smoothV.qtf = query[i].qtf[l]; smoothV.docLen = documentContainer[docid].len;
                			tempResult[m].score += retrievalModel(&smoothV);
                		}
                	}
                	m++; l2++;
                	continue;
                }
                
                while ( (result[l1].docid < docid) && (l1 < curResSetSize) ) {
                	tempResult[m].docid = result[l1].docid;
                	tempResult[m].score = result[l1].score;
                	// required only for language model smoothing
                	if (smoothReq) {
                		smoothV.colProbOfTerm = (1.0 * queryTermCtf[j])/numberOfToken_coll;
                		smoothV.qtf = query[i].qtf[j]; smoothV.docLen = documentContainer[tempResult[m].docid].len;
                		tempResult[m].score += retrievalModel(&smoothV);
                	}
                	m++; l1++;
                }
                
                if (result[l1].docid == docid) {
                	tempResult[m].docid = docid;
                	tempResult[m].score = result[l1].score + termScore;
                	m++; l1++; l2++;
                }
                
                else {
                	tempResult[m].docid = docid;
                	tempResult[m].score = termScore;
                	// required only for language model smoothing
                	if (smoothReq) {
                		for(l = 0; l < j; l++) {
                			if(queryTermCtf[l] == 0) 
                				continue;
                			smoothV.colProbOfTerm = (1.0 * queryTermCtf[l])/numberOfToken_coll;
                			smoothV.qtf = query[i].qtf[l]; smoothV.docLen = documentContainer[docid].len;
                			tempResult[m].score += retrievalModel(&smoothV);
                		}
                	}
                	m++; l2++;
                }
                // list 2 exhausted, copy the remaining part of list 1
                if(l2 == ptr->df) {
                        while(l1 < curResSetSize) {
                             tempResult[m].docid = result[l1].docid;
                	     tempResult[m].score = result[l1].score;
                	     // required only for language model smoothing
                	     if (smoothReq) {
                		smoothV.colProbOfTerm = (1.0 * queryTermCtf[j])/numberOfToken_coll;
                		smoothV.qtf = query[i].qtf[j]; smoothV.docLen = documentContainer[tempResult[m].docid].len;
                		tempResult[m].score += retrievalModel(&smoothV);
                	     }
                	     m++; l1++;   
                        }
                }
          } 
          
          curResSetSize = m;
          temptr = result; result = tempResult; tempResult = temptr;  // swap result lists
          //printf("found %s with df %d\n", query[i].terms[j], ptr->df);  
     }
     
     buildHeap(result, curResSetSize, sizeof(res_t), comp_result);
     // printing results
     for(k = 0; k < MIN(1000, curResSetSize); k++) {              
        printf("%d Q0 %s %Ld %f %s\n", query[i].no, documentContainer[result[0].docid].docno, k, result[0].score, runid);
        if(deleteRootElement(result, curResSetSize-k, sizeof(res_t), comp_result) == 0)
           break;
     }  
  }   
  
  /* Clean up */
  for(i = 0; i < sizeOfLexicon; i++)
    free(lexiconContainer[i].term);
  free(lexiconContainer); 
  
  for(i = 0; i < numberOfDoc_coll; i++)
    free(documentContainer[i].docno); 
  free(documentContainer);
  
  free(result); 
  free(tempResult);
  free(postingBuffer); 
  
  fclose(fpinv);  
  return 0;
}
