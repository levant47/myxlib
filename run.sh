set -ex
COMPILER_FLAGS='-I /home/levant/doc/plain/code/c++ -Werror -nostdinc -nostdlib -mno-sse'
g++ -O2 $COMPILER_FLAGS main.cpp -o main.bin
g++ -g $COMPILER_FLAGS main.cpp -o debug.bin
./main.bin
