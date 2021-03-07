mkdir -p build
cd build
CC=/usr/local/bin/gcc CXX=/usr/local/bin/g++ cmake ..
make
cd ../test
mkdir -p build
cd build
CC=/usr/local/bin/gcc CXX=/usr/local/bin/g++ cmake ..
make
cd ../../
