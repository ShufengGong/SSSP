PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/Google/Hash
OUTPUT=${PREFIX}/roadNet-CA/result/dtst_dis_result.txt
#./SSSP --in $INPUT --out $OUTPUT --s $3 --vnum $4 --fun $2 -delta $5

mpirun -np 4 src/deltaab --in $INPUT --out $OUTPUT --model 2 --source 0 --num_vertex 1000000 --req_capacity 300000 --delta 1 --delta_length 2000
