int enCode(long long int d, unsigned char *code)
{
  int codeLength = 0;
  while(d >= 128) {
    code[codeLength++] = 128 + (d & 127);
    d = d >> 7;
  }
  code[codeLength++] = d;
 return codeLength; 
}

int deCode(unsigned char *code, long long int *num)     // decodes and stores the next integer: returns number of bytes
{
  int codeLength = 0, shift = 0;
  long long int w = 0;
  while(code[codeLength] >= 128) {
    w = w + ((code[codeLength] & 127) << shift);
    shift += 7;
    codeLength++;
  }
  w = w + (code[codeLength] << shift);
  codeLength++;
  *num = w;
  return codeLength;
}

int partition(void *base, void *pivot, size_t numElem, size_t size, int (*comp)(const void *, const void *))
{
   int i, j, p;
   p = 0; i = p-1;
   for(j = p; j < numElem; j++)
     if( (*comp)(base + j * size, pivot) < 0 ) {
        i = i+1;
        SWAP(base + (i * size), base + (j * size), size);
     }   
   return i;     
}
