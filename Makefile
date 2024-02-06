PROD = mylisp

${PROD}: main.o
	make -C ../mylisp/linuxdbg
	clang++ -o $@ $^ -L ../mylisp/linuxdbg -lmylisp

%.o: %.cc
	clang++ -o $@ $< -I .. -std=c++2b -c -g
