PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/roadNet-CA/Hash
OUTPUT=${PREFIX}/roadNet-CA/result/dtst_dis_result.txt
#./SSSP --in $INPUT --out $OUTPUT --s $3 --vnum $4 --fun $2 -delta $5

mpirun -np 4 src/beldel --in $INPUT --out $OUTPUT --source 0 --num_vertex 2000000 --req_capacity 2000000 --delta 1 --delta_length 2000
