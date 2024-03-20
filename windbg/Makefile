include ../Makefile

PROD = mylisp.exe

${PROD}: ${OBJS} libmylisp.so
	clang++ -o $@ $^ -L . $(addprefix -l, ${LIBS}) -g -stdlib=libc++

%.o: ../%.cc $(addprefix ../, ${HHS})
	clang++ -o $@ $< -I .. -I ../.. -std=c++2b -c -g -stdlib=libc++

libmylisp.so:
	make -C ../../mylisp/windbg
	copy ..\..\mylisp\windbg\$@ $@

clean:
	echo > ${PROD}
	echo > libmylisp.so
	del ${PROD} *.o libmylisp.so

.PHONY: clean
