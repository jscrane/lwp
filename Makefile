CC=gcc
AR=ar
RANLIB=ranlib
CFLAGS=-Wall -g

OBJ=lwp.o sem.o queue.o clk.o sig.o $(ARCH).o

all:	arch.h timer producer

arch.h:
	ln -sf $(ARCH).h arch.h

producer:	producer.o liblwp.a
		gcc $(CFLAGS) -o producer producer.o -L. -llwp

timer:	timer.o liblwp.a
	gcc $(CFLAGS) -o timer timer.o -L. -llwp

liblwp.a:	$(OBJ) lwp.h
		$(AR) rc liblwp.a $(OBJ)
		$(RANLIB) liblwp.a

clean:
	rm -f liblwp.a $(OBJ) producer.o timer.o producer timer arch.h

bm.o: bm.c lwp.h $(ARCH).h 
clk.o: clk.c lwp.h $(ARCH).h
lwp.o: lwp.c lwp.h $(ARCH).h
producer.o: producer.c lwp.h $(ARCH).h
queue.o: queue.c lwp.h $(ARCH).h
sem.o: sem.c lwp.h $(ARCH).h
sig.o: sig.c lwp.h $(ARCH).h
timer.o: timer.c lwp.h $(ARCH).h
producer.o: producer.c lwp.h $(ARCH).h
