#include "common.h"
#define BUF_SIZE 100
#include "heap.h"
#include "utility.h"

long long int prevDocid;

typedef struct 
{
  char term[BUF_SIZE];
  long long int df;
  long long int ctf;
  int fileNo;
} indexKey_t;

int comp_indexKey(const void *a, const void *b)
{
  indexKey_t *e1, *e2;
  e1 = (indexKey_t *) a;
  e2 = (indexKey_t *) b;
  if(strcmp(e1->term, e2->term) == 0) {
     if(e1->fileNo < e2->fileNo)
       return -1;
     else
       return 1;
  }    
  else
     return strcmp(e1->term, e2->term); 
}

void ReadAndCompressAndWritePosting(FILE *readfp, FILE *writefp, long long int df)
{
  long long int i, docid;
  unsigned char code[BUF_SIZE];
  int tf, codelen;
  for(i = 0; i < df; i++) {
    if(2 != fscanf(readfp, "(%Ld,%d)", &docid, &tf)) {
       printf("Wrong posting entry in file for term\n");
       exit(0);
    } 
    codelen = enCode(docid-prevDocid, code); // encode docid difference
    codelen += enCode(tf, code + codelen); // encode tf
    if(fwrite(code, 1, codelen, writefp) != codelen) {
      printf("Failed to write index\n");
      exit(0);
    }
    prevDocid = docid;
  }  
}

void MultiWayMerge(char *fileOfFile, char *indexName)
{
  char temp[BUF_SIZE], prevTerm[BUF_SIZE], **tempIndexFiles;
  FILE *fof, **tempIndexFp, *indexFp, *lexFp;
  long long int df, ctf;
  int i, numFile = 0, nf;
  indexKey_t *indexKey;
  off_t address;
  
  if((fof = fopen(fileOfFile, "r")) == NULL) {
    printf("Could not open file %s\n", fileOfFile);
    exit(0);
  }
  
  sprintf(temp, "%s.lex", indexName);
  if((lexFp = fopen(temp, "w")) == NULL) {
    printf("Could not open file %s\n", temp);
    exit(0);
  }
  
  sprintf(temp, "%s.index", indexName);
  if((indexFp = fopen(temp, "wb")) == NULL) {
    printf("Could not open file %s\n", temp);
    exit(0);
  }
  
  while(fscanf(fof, "%s", temp) == 1)
     numFile++;
  nf = numFile;   
  tempIndexFp = (FILE **) malloc(numFile * sizeof(FILE *)); 
  rewind(fof);
  tempIndexFiles = (char **) malloc(numFile * sizeof(char *));
  // open all files for reading
  for(numFile = 0; fscanf(fof, "%s", temp) == 1; numFile++) {
     if((tempIndexFp[numFile] = fopen(temp, "r")) == NULL) {
        printf("Could not open file %s\n", temp);
        exit(0);
     }
     tempIndexFiles[numFile] = strdup(temp);
  }   
  fclose(fof);   
  
  indexKey = (indexKey_t *) malloc(numFile * sizeof(indexKey_t));
  //read the first entry from each file
  for(i = 0; i < numFile; i++)
    if(3 == fscanf(tempIndexFp[i], "(%s %Ld %Ld)", indexKey[i].term, &indexKey[i].df, &indexKey[i].ctf))
      indexKey[i].fileNo = i; 
    else  
      printf("Error reading term, df, ctf from temporary file %d\n", i);
   
   buildHeap(indexKey, numFile, sizeof(indexKey_t), comp_indexKey); // build heap with first element of each file
   // initialize
   prevTerm[0] = '\0'; df = 0; ctf = 0; 
   address = ftello(indexFp);  
   prevDocid = 0;
                
   do {
      if(strcmp(prevTerm, indexKey[0].term) == 0) {
         df += indexKey[0].df;
         ctf += indexKey[0].ctf;
         ReadAndCompressAndWritePosting(tempIndexFp[indexKey[0].fileNo], indexFp, indexKey[0].df);   
      }
      else {
         if(df > 0)                // last term is not blank
            fprintf(lexFp, "%s, %Ld, %Ld, %Ld\n", prevTerm, df, ctf, (long long int) address);
         strcpy(prevTerm, indexKey[0].term); df = indexKey[0].df; ctf = indexKey[0].ctf; 
         address = ftello(indexFp);
         prevDocid = 0;
         ReadAndCompressAndWritePosting(tempIndexFp[indexKey[0].fileNo], indexFp, indexKey[0].df);
      }
      
      if(fscanf(tempIndexFp[indexKey[0].fileNo], "\n(%s %Ld %Ld)", indexKey[0].term, &indexKey[0].df, &indexKey[0].ctf) == 3)
         heapify(indexKey, 0, numFile, sizeof(indexKey_t), comp_indexKey);
      else if(!feof(tempIndexFp[indexKey[0].fileNo])) 
         printf("Error reading temporary inverted index from file %s\n", tempIndexFiles[indexKey[0].fileNo]);   
      else
         numFile = deleteRootElement(indexKey, numFile, sizeof(indexKey_t), comp_indexKey);
      
   } while(numFile > 0);
   
   //write lex entry for the last term 
   fprintf(lexFp, "%s, %Ld, %Ld, %Ld\n", indexKey[0].term, df, ctf, (long long int) address);
   
   fclose(indexFp);
   fclose(lexFp);
   for(i = 0; i < numFile; i++)
     fclose(tempIndexFp[i]);
   // delete all temporary inverted files  
  for(i = 0; i < nf; i++)
     remove(tempIndexFiles[i]); 
  remove(fileOfFile); 
}

int main(int argc, char *argv[])
{
  FILE *lexFp, *docFp, *statFp;
  long long int lexSize, docLenSum, numDoc, t1, t2, t3, t4;
  char temp[1024];
  if(argc -1 != 2) {
     printf("Missing the following:\n1. File containing file names of temporary indexes\n2. Index name\n");
     return 0;
  } 
  
  // merge the temporary indexes
  MultiWayMerge(argv[1], argv[2]);
  
  //ready to write collection statistics 
  sprintf(temp, "%s.lex", argv[2]);
  if((lexFp = fopen(temp, "r")) == NULL) {
    printf("Could not open file %s\n", temp);
  }
  for(lexSize = 0; fscanf(lexFp, "%s%Ld,%Ld,%Ld", temp, &t1, &t2, &t3) > 0; lexSize++) ;
    
  
  
  sprintf(temp, "%s.docid", argv[2]);
  if((docFp = fopen(temp, "r")) == NULL) {
     printf("Could not open file %s\n", temp);
  }
  docLenSum = 0;
  for(numDoc = 0; fscanf(docFp, "%s%Ld%Ld%Ld%Ld", temp, &t1, &t2, &t3, &t4) > 0; numDoc++)
     docLenSum += t3; 
  
  sprintf(temp, "%s.stat", argv[2]);
  if((statFp = fopen(temp, "w")) == NULL) {
    printf("Could not open file %s\n", temp);
  }
  fprintf(statFp, "lexSize\t%Ld\n", lexSize);
  fprintf(statFp, "numDoc\t%Ld\n", numDoc);
  fprintf(statFp, "colLen\t%Ld\n", docLenSum);
  fprintf(statFp, "avgDocLen\t%.2f\n",(float) (1.0 * docLenSum)/numDoc);
  
  fclose(lexFp);
  fclose(docFp);
  fclose(statFp);
  remove(argv[1]);
  return 0;
}



