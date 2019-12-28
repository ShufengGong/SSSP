PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/$1/graph_weight.txt
OUTPUT=${PREFIX}/$1/graph_weight_vertex.txt
src/SSSP --fun convert_graph --in $INPUT --out $OUTPUT --vnum $2
