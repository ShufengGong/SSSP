PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/roadNet-CA/Hash
#OUTPUT=./result/${1}_sssp_$2.txt
#./SSSP --in $INPUT --out $OUTPUT --s $3 --vnum $4 --fun $2 -delta $5

./absssp --model $1 --in $INPUT --num_thread 4 --vertex_num 1971281
