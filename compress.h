int enCode(long long int d, unsigned char *code)
{
  int i;
  i = 0;
  while(d >= 128) {
    code[i++] = 128 + (d & 127);
    d = d >> 7;
  }
  code[i++] = d;
 return i; 
}

int deCode(unsigned char *code, int startPos)
{
  int i, w = 0, shift = 0;
  i = startPos;
  while(code[i] >= 128) {
    w = w + ((code[i] & 127) << shift);
    shift += 7;
    i++;
  }
  w = w + (code[i] << shift);
  return w;
}

/*int main(int argc, char *argv[])
{
  unsigned char code[1000];
  int i, n; 
  n = enCode(atoi(argv[1]), code);
  for(i = 0; i < n; i++)
    printf("%d\n", code[i]); 
  printf("Decoded number %d\n", deCode(code));  
  return 0;
} */
