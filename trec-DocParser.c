#define _FILE_OFFSET_BITS 64
#define MAX_DOC_LEN 12400
#define MAX_TERM_LEN 30
#define BUF_SIZE 1024
#include "common.h"
#include "structures.h"
#include "stemmer.h"
#include "tokenizer.h"
#include "comparators.h"


invEntry_t *inv_buf;
stop_t *stopword;
stem_t *stemmer;
int stop_size, stem_size;
int STEM_FLAG;

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
              if(STEM_FLAG)
                 porter_stem(TEMP_BUF);                                        // Insert here stemmer
              if(strlen(TEMP_BUF)> 0)
                strcpy(doc->terms[doc->len].term, TEMP_BUF);
              doc->terms[doc->len].pos = doc->len;
              if(doc->len < MAX_DOC_LEN-2)
              doc->len++;
           }  
     }   
  }  
  return doc->len;
}


int main(int argc, char *argv[])
{
  FILE *fin, *fall, *fdirect, *foffset, *ftemp, *fpstop, *fptmpindx;
  int tf, temp_pos[BUF_SIZE], num_uniq_term;
  int df, ctf, inv_file_count, POS_INDEX;
  long long int doc_id, voc_size, l, i, j, k, m1, m2;
  char file_name[256], inv_file[256], inv_file_no[256], dir_file[256], dir_off_file[256], indexPath[1024];
  char TEMP_BUF[BUF_SIZE], WORD[BUF_SIZE], command[256];
  size_t invBufSize;
  document_t doc;
  struct stat directory = {0};
  struct sysinfo info;
  
  POS_INDEX = 0;
  
  if(argc - 1 < 4) {
     printf("Give file containing doc file names\nGive collection name\nGive stopword file\n");
     printf("Apply stemming? (y/n)\n");
     printf("If you want positional direct index, give p in command line\n");
     return 0;
  }
  else 
    if(argc - 1 == 5) {
      if(strcmp(argv[5], "p") == 0)
         POS_INDEX = 1;   
    }    
  else ;
  
  if(strcmp(argv[4], "y") == 0)
    STEM_FLAG = 1;
  
  if(stat("./indexes", &directory) == -1) {
    mkdir("./indexes", 0777);
  }  
  
  sprintf(indexPath, "./indexes/%s", argv[2]);
  if(stat(indexPath, &directory) == -1) {
     mkdir(indexPath, 0777);
  }
  else {
    printf("Index <%s> already exists\n", argv[2]);
    return 0;
  }  

  sprintf(dir_file, "%s/%s.direct", indexPath, argv[2]);
  sprintf(dir_off_file, "%s/%s.docid", indexPath, argv[2]);
  
  if((fall = fopen(argv[1], "r")) == NULL)
     printf("Could not open file %s\n", argv[1]);
  
  if((fdirect = fopen(dir_file, "w")) == NULL)
     printf("Could not open file direct-output\n");
     
  if((foffset = fopen(dir_off_file, "w")) == NULL)
     printf("Could not open file offset-output\n"); 
  
  if((fpstop = fopen(argv[3], "r")) == NULL)
     printf("Could not open stem file %s\n", argv[3]);   
     
  if((fptmpindx = fopen("ListOfTempIndex", "w")) == NULL)
     printf("Could not open file for list of inverted index\n");
    
  sysinfo(&info);                   // for knowing the ram size
  invBufSize = info.totalram/(2 * sizeof(invEntry_t));
  if((inv_buf = malloc(invBufSize * sizeof(invEntry_t))) == NULL) {
     printf("Could not allocate memory for inverted buffer\n");
     return 0;
  }
  else 
     printf("Allocated buffer size for inverted index: %lu bytes\n", invBufSize);
  
  doc_id = -1;
  voc_size = 0;
  inv_file_count = 0;
  
  /* reading stopword */
  
  for(stop_size = 0; fscanf(fpstop, "%s", TEMP_BUF) > 0; stop_size++) ;
  rewind(fpstop);
  if((stopword = malloc(stop_size * sizeof(stop_t))) == NULL)
    printf("Could not allocate memory for stopword\n");
  for(stop_size = 0; fscanf(fpstop, "%s", TEMP_BUF) > 0; stop_size++) {
     TEMP_BUF[MAX_TERM_LEN - 1] = '\0';
     stopword[stop_size].word = strdup(TEMP_BUF);
  }
  qsort(stopword, stop_size, sizeof(stop_t), comp_stop);
  
  while(fscanf(fall, "%s", file_name) > 0) {
  
     if(strcmp(file_name+strlen(file_name)-3, ".gz") != 0) {          // ordinary file: open for reading
        if((fin = fopen(file_name, "r")) == NULL) {
          printf("Could not open file %s\n", file_name);
          continue;
        }  
     }
     else {                                                          // .gz file: uncompress it and then open for reading
        sprintf(command, "gzip -d -c %s > decom-doc-file", file_name);
        system(command);
        if((fin = fopen("decom-doc-file", "r")) == NULL) {
           printf("Could not open decompressed file of %s\n", file_name);
           continue;
        } 
     } 
      
     while((l = get_trec_doc(fin, &doc)) > 0) {
     
        if(doc.len == 0) {
          printf("Empty or wrongly tagged document ignored\n");
          continue;
        }
        
        if(doc.len > 0) {
           doc_id ++;
           qsort(doc.terms, doc.len, sizeof(termAndPos_t), comp_termAndPos_t);
           for(i = 0, num_uniq_term = 1; i < doc.len-1; i++)
              if(strcmp(doc.terms[i].term, doc.terms[i+1].term) != 0)
                 num_uniq_term++;
           fprintf(foffset, "%s\t%Ld\t\t%d\t%d\t%Ld\n", doc.no, doc_id, num_uniq_term, doc.len, (long long int)ftello(fdirect));   
           fprintf(fdirect, "(%s %d %d):", doc.no, doc.len, num_uniq_term);
              
           for(i = 0; i < doc.len; ) {
              for(j = i, tf = 0; j < doc.len && strcmp(doc.terms[i].term, doc.terms[j].term) == 0; tf++, j++)
                if(tf < BUF_SIZE - 2)
                  temp_pos[tf] = doc.terms[j].pos;
                else
                  tf = BUF_SIZE -2;  
              
              strcpy(inv_buf[voc_size].term, doc.terms[i].term);
              inv_buf[voc_size].tf = tf;
              inv_buf[voc_size].docid = doc_id;
              
              if(voc_size < invBufSize -1)
                 voc_size++;
              else {
                 qsort(inv_buf, voc_size, sizeof(invEntry_t), comp_invEntry_t);
                 sprintf(inv_file, "%s/temp-inv-file-%d", indexPath, inv_file_count);
                 if((ftemp = fopen(inv_file, "w")) == NULL)
                     printf("Could not open file temp-inv-output\n"); 
                 fprintf(fptmpindx, "%s\n", inv_file); 
                 /* inverting the temporary list */
                 for(m1 = 0; m1 < voc_size; ) {
                    df = 0; ctf = 0;
                    for(m2 = m1; m2 < voc_size && strcmp(inv_buf[m1].term, inv_buf[m2].term) == 0; m2++) {
                       df++;
                       ctf += inv_buf[m2].tf;     
                    }
                    fprintf(ftemp,"(%s\t%d\t%d)", inv_buf[m1].term, df, ctf);
                    for( ; m1 < m2; m1++)
                       fprintf(ftemp, "(%Ld,%d)", inv_buf[m1].docid, inv_buf[m1].tf);
                    fprintf(ftemp, "\n");   
                 }   
                 fclose(ftemp);
                 inv_file_count += 1;     
                 voc_size = 0;   
              }   
              
              fprintf(fdirect, "(%s %d", doc.terms[i].term, tf);
              if(POS_INDEX)
                for(k = 0; k < tf; k++)
                   fprintf(fdirect, " %d", temp_pos[k]);
              fprintf(fdirect, ")"); 
              i = j;  
           }  
           fprintf(fdirect, "\n");
        }      
     }  
     fclose(fin);
     printf("Processing %s finished\n", file_name);
  }
  
  /* inverting the tail end */
  if(voc_size > 0) {
     qsort(inv_buf, voc_size, sizeof(invEntry_t), comp_invEntry_t);
     sprintf(inv_file, "%s/temp-inv-file-%d", indexPath, inv_file_count);
     if((ftemp = fopen(inv_file, "w")) == NULL)
       printf("Could not open file temp-inv-output\n"); 
     fprintf(fptmpindx, "%s\n", inv_file);
     for(m1 = 0; m1 < voc_size; ) {
         df = 0; ctf = 0;
         for(m2 = m1; m2 < voc_size && strcmp(inv_buf[m1].term, inv_buf[m2].term) == 0; m2++) {
            df++;
            ctf += inv_buf[m2].tf;   
         }
         
         fprintf(ftemp,"(%s\t%d\t%d)", inv_buf[m1].term, df, ctf);
         for( ; m1 < m2; m1++)
            fprintf(ftemp, "(%Ld,%d)", inv_buf[m1].docid, inv_buf[m1].tf);
         fprintf(ftemp, "\n");   
     } 
     fclose(ftemp);  
  }
  
  remove("decom-doc-file");
  remove(argv[1]);
  fclose(fall);
  fclose(fdirect);
  fclose(foffset);
  fclose(fpstop);
  fclose(fptmpindx);
  return 0;
}
