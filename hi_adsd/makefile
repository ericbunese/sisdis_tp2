all: tempo

tempo: tempo.o smpl.o rand.o cisj.o
	$(LINK.c) -o $@ -Bstatic tempo.o smpl.o rand.o cisj.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

tempo.o: tempo.c smpl.h
	$(COMPILE.c) -g  tempo.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

cisj.o: cisj.c
	$(COMPILE.c) -g cisj.c

clean:
	$(RM) *.o tempo relat saida
