#for doc_limit in 1000000 2000000 4000000 8000000
#do
 #   for t in 25 50 100
  #  do 
   #     ./buildCompatWindows -doc_limit $doc_limit -t $t -k 32 >> exp2.txt
    #done
#done

#for tokenNum in 16000 32000 64000 128000
#do
 #   ./buildCompatWindows -doc_limit 8000000 -t 50 -k 32 -tokenNum $tokenNum >> exp3.txt
#done
#do
	./buildCompatWindows -t 25 -k 4 >>exp1.txt
#done
