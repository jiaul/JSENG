#!/bin/bash
if [ $# -ne 5 ]
then
echo "give colection path"
echo "give index name"
echo "Stopword file"
echo "Apply stemming? (y/n)"
echo "Give collection type: (trec or warc)"
exit
fi
gcc -o parser.o $5-DocParser.c
gcc -o IndexMerge.o MergeTempInvertedIndex.c
find $1 -type f > spec
./parser.o spec $2 $3 $4
echo "Tokenization and direct index building finished"
./IndexMerge.o ListOfTempIndex indexes/$2/$2
echo "Inverted index building finished"
rm -rf parser.o IndexMerge.o spec

