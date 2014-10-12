make clean
make
g++ -o test basic.c librvm.a
g++ -o 1test test1.c librvm.a
g++ -o tabort abort.c librvm.a
g++ -o multabort multi-abort.c librvm.a
g++ -o 2test test2.c librvm.a
g++ -o 3test test3.c librvm.a
g++ -o 4test test4.c librvm.a
g++ -o 5test test5.c librvm.a
g++ -o 6test semantics_01.c librvm.a
g++ -o 7test semantics_02.c librvm.a
g++ -o 8test semantics_03.c librvm.a



sudo rm rvm_segments/*
sudo rm -rf log/*
sudo rmdir rvm_segments
sudo rm REDO_LOG
