#gcc mapreduce.c mapreduce.h wordcount.c -o wordcount -pthread
gcc -o mapreduce wordcount.c mapreduce.c mapreduce.h -Wall -Werror -pthread 