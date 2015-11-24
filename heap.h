void heapify(void *ptr, int root, size_t numElem, size_t elemSize, int (*comp)(const void *, const void *))
{
   int lChild, rChild, best;
   lChild = 2*root+1; rChild = 2*root+2;
   if(lChild < numElem && (*comp)(ptr+lChild*elemSize, ptr+root*elemSize) < 0)
      best = lChild;
   else
      best = root;
   if(rChild < numElem && (*comp)(ptr+rChild*elemSize, ptr+best*elemSize) < 0)
      best = rChild;
   if(best != root) {
      SWAP(ptr+root*elemSize, ptr+best*elemSize, elemSize);
      heapify(ptr, best, numElem, elemSize, comp);
   }
}

void buildHeap(void *ptr, size_t numElem, size_t elemSize, int (*comp)(const void *, const void *))
{
   int root;
   for(root = numElem/2 - 1; root >= 0; root--)
     heapify(ptr, root, numElem, elemSize, comp);
}

size_t deleteRootElement(void *ptr, size_t numElem, size_t elemSize, int (*comp)(const void *, const void *))
{
	SWAP(ptr, ptr+(numElem-1)*elemSize, elemSize);
	numElem = numElem-1;
	heapify(ptr, 0, numElem, elemSize, comp);
	return numElem;
}
