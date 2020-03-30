#gcc mapreduce.c mapreduce.h wordcount.c -o wordcount -pthread
#gcc -o mapreduce wordcount.c mapreduce.c mapreduce.h -Wall -Werror -pthread 
gcc -o mapreduce wordcount.c mapreduce.c mapreduce.h -pthread 
./mapreduce tmh.txt lxy.txt