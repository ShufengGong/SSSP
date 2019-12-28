PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/roadNet-CA/graph_weight_vertex.txt
OUTPUT=${PREFIX}/roadNet-CA/result/single_sssp_dijkstra_result.txt
#./SSSP --in $INPUT --out $OUTPUT --s $3 --vnum $4 --fun $2 -delta $5

./SSSP --fun dijkstra --in $INPUT --out $OUTPUT --source 0 --vertex_num 1971281
#./SSSP --fun delta_priority --in $INPUT --out ${PREFIX}/roadNet-CA/result/delta_priority.txt --source 0 --vertex_num 1971281
