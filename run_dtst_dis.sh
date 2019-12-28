PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/WiKi-talk/Hash
OUTPUT=${PREFIX}/WiKi-talk/result/dtst_dis_result.txt
#./SSSP --in $INPUT --out $OUTPUT --s $3 --vnum $4 --fun $2 -delta $5

mpirun -np 4 src/deltastep --model 2 --in $INPUT --out $OUTPUT --source 3 --num_vertex 4000000 --req_capacity 10000000 --delta 1 --delta_length 2000
