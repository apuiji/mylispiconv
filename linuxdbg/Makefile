include ../Makefile

PROD := mylisp

${PROD}: ${OBJS} libmylisp.so
	clang++ -o $@ $^ -L . $(addprefix -l, ${LIBS}) -g -stdlib=libc++

%.o: ../%.cc $(addprefix ../, ${HHS})
	clang++ -o $@ $< -I .. -I ../.. -std=c++2b -c -g -stdlib=libc++

libmylisp.so:
	make -C ../../mylisp/linuxdbg
	cp ../../mylisp/linuxdbg/$@ .

clean:
	touch ${PROD}
	touch libmylisp.so
	rm ${PROD} *.o libmylisp.so

.PHONY: clean exportLibsPath
