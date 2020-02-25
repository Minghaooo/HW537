# cs537 p2a
## Minghao  (mtang64@wisc.edu)

1. when entering a command , the program will first checke if there is any ";" in the command, if have , then pass the command before ";". the program will also make sure all the kid_process are finished therefore the next command will be executed. 
   
2. second the "&" is checked , if have, then send them to execute. the main process will fork several kid process at the same time ,and then wait for them to finish.
   
3. check if have build_in commands, if not then pass the command to the parsing function.
   
4.  while parsing, the function will make sure that if we have ">", if have then the last argument will be send independent and will be replaced with NULL. 
   