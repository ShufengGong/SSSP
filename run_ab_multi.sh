PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/roadNet-CA/Hash
OUTPUT=${PREFIX}/roadNet-CA/result/ab_multi_result

#./SSSP --in $INPUT --out $OUTPUT --s $3 --vnum $4 --fun $2 -delta $5
./absssp --model 1 --in $INPUT --out $OUTPUT --source 0 --num_thread 4 --num_vertex 1971281
