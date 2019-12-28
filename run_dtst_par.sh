PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/roadNet-CA/Hash
OUTPUT=${PREFIX}/roadNet-CA/result/dtst_multi_result.txt
#./SSSP --in $INPUT --out $OUTPUT --s $3 --vnum $4 --fun $2 -delta $5

./deltastep --model 1 --in $INPUT --out $OUTPUT --source 0 --num_vertex 1971281 --num_thread 2 --delta 0.5 --delta_length 2000
