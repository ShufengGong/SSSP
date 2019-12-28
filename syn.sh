scp CMakeLists.txt hadoop0:/home/gongsf/SSSP/
scp CMakeLists.txt hadoop1:/home/gongsf/SSSP/
scp CMakeLists.txt hadoop2:/home/gongsf/SSSP/
scp -r src hadoop0:/home/gongsf/SSSP/
scp -r src hadoop1:/home/gongsf/SSSP/
scp -r src hadoop2:/home/gongsf/SSSP/
#ssh hadoop1 'make -C /home/gongsf/SSSP/'
#ssh hadoop2 'make -C /home/gongsf/SSSP/'
#scp -r ../SSSP aliyun:/home/gongsf/
