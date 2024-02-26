include ../Makefile

DEPS1 := $(addprefix ../, $(addsuffix /windbg, ${DEPS}))
PROD = mylisp.exe

${PROD}: $(addsuffix .o, ${OBJS})
	$(addprefix make -C , $(addsuffix ;, ${DEPS1}))
	clang++ -o $@ $^ $(addprefix -L, ${DEPS1}) $(addprefix -l, ${LIBS}) -g -stdlib=libc++

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I ../.. -std=c++2b -c -g -stdlib=libc++

clean:
	echo > ${PROD}
	del ${PROD} *.o

.PHONY: clean
