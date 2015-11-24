# JSENG
**How to build index**

Just hit the following command, it will tell you the arguments in needs.

```
bash buildindex.sh
```
The indexer create a directory named `indexes`, in your current working directory, where it stores all the indexes. 

**How to execute queries**

First compile the source to generate `<a.out>` as follows.

```
gcc RunTrecQuery.c -lm
```

Then just hit the following to know the required arguments. 

```
./a.out 
```
**NOTE**:  
The system assumes that the queries are in a file with TREC fomatting. 
