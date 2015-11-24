typedef struct
{
  char term[MAX_TERM_LEN];
  long long int docid;
  int tf;
} invEntry_t;

typedef struct 
{
  char term[MAX_TERM_LEN];
  int pos;
} termAndPos_t;

typedef struct
{
  char no[128];
  termAndPos_t terms[MAX_DOC_LEN];
  int len;
} document_t;

typedef struct
{
  int no;
  char **terms;
  float *qtf;
  int no_terms;
} query_t;

typedef struct
{
  char *term;
  int df; 
  int ctf;
  off_t of;
} lex_t;


typedef struct
{
  char *docno;
  long long int id;
  int ut;
  int len;
  off_t directOfset;
} doc_t;

typedef struct
{
  long long int docid;
  float score;
} res_t;

typedef struct
{
  char *word;
  char *stem;
} stem_t;

typedef struct
{
  char *word;
} stop_t;

typedef struct
{
  wchar_t *term;
  int df; 
  int ctf;
  off_t of;
} utf8Lex_t;

typedef struct
{
  int no;
  wchar_t **terms;
  float *qtf;
  int no_terms;
} utf8Query_t;

typedef struct
{
  wchar_t *word;
} utf8Stop_t;

typedef struct
{
  wchar_t *word;
  wchar_t *stem;
} utf8Stem_t;

typedef struct
{
  wchar_t term[MAX_TERM_LEN];
  long long int docid;
  int tf;
} utf8InvEntry_t;

typedef struct
{
  wchar_t term[MAX_TERM_LEN];
  int pos;
} utf8TermAndPos_t;

typedef struct
{
  wchar_t no[128];
  utf8TermAndPos_t terms[MAX_DOC_LEN];
  int len;
} utf8Document_t;

typedef struct
{
  long long int numOfDoc;
  long long int numOfColTerm;
  long long int df;
  long long int ctf;
  long double colProbOfTerm;
  float qtf;
  float avgDocLen;
  float param;
  float pdmShape;
  float standardIDF;
  float bm25IDF;
  int docLen;
  int numDocUniqTerm;
  int tf;
  int qlen;
} modelVariables_t;
