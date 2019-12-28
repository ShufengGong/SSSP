PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/roadNet-CA/Hash
OUTPUT=../logs/thread

#./SSSP --in $INPUT --out $OUTPUT --s $3 --vnum $4 --fun $2 -delta $5
rm ../logs/thread*
./simple_sssp --in $INPUT --out $OUTPUT --source 0 --num_thread 4 --vertex_num 1971281
