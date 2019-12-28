PREFIX=/home/gongsf/dataSet
INPUT=${PREFIX}/$1/graph.txt
OUTPUT=${PREFIX}/$1/graph_weight.txt
src/SSSP --fun set_weight --in $INPUT --out $OUTPUT
