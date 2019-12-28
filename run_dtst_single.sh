PREFIX=/home/gongsf/data
INPUT=${PREFIX}/graph.txt
OUTPUT=${PREFIX}/result/single_dtst_sssp_result.txt
#./SSSP --in $INPUT --out $OUTPUT --s $3 --vnum $4 --fun $2 -delta $5

./deltastep --model 0 --in $INPUT --out $OUTPUT --source 0 --num_vertex 1971281 --delta 1 --delta_length 2000
