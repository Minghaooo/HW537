#gcc mapreduce.c mapreduce.h wordcount.c -o wordcount -pthread
gcc -o mapreduce wordcount.c mapreduce.c mapreduce.h -Wall -Werror -pthread 
#gcc -fsanitize=address -o mapreduce wordcount.c mapreduce.c mapreduce.h -pthread
#gcc -fsanitize=thread -o mapreduce wordcount.c mapreduce.c mapreduce.h -pthread 
#gcc  -o mapreduce wordcount.c mapreduce.c mapreduce.h -pthread 
./mapreduce tmh lxy asd bbb aaa sss