PROGS = bench_ins bench_rand_ins bench_query bench_rtree empirical_density

OBJDIR = obj

OBJS = $(addprefix $(OBJDIR)/, geode.o geohash.o timer.o bitmap_graphics.o rtree.o)

CFLAGS = -g -Wall -std=c11 -O3 -lm -lstdc++

obj/%.o : src/%.c
	@mkdir -p $(@D)
	gcc -c ${CFLAGS} -o $@ $<

all : libcount.a ${PROGS}

libcount.a:
	cd ./libcount && make
	cp libcount/libcount.a .

bench_ins : src/bench_ins.c ${OBJS}
	gcc ${CFLAGS} -o $@ $^ ./libcount.a

bench_rand_ins : src/bench_rand_ins.c ${OBJS}
	gcc ${CFLAGS} -o $@ $^ ./libcount.a

bench_query : src/bench_query.c ${OBJS}
	gcc ${CFLAGS} -o $@ $^ ./libcount.a

bench_rtree : src/bench_rtree.c ${OBJS}
	gcc ${CFLAGS} -o $@ $^ ./libcount.a


empirical_density : src/empirical_density.c ${OBJS}
	gcc ${CFLAGS} -o $@ $^ ./libcount.a

clean :
	rm -rf ${PROGS} ${OBJDIR}  libcount.a
	cd libcount && make clean
