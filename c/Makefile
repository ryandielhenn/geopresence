PROGS = bench_ins bench_rand_ins bench_query bench_rtree empirical_density

OBJDIR = obj
# pull rtree and roaring from deps, keep modified bitmap_graphics in src
OBJS = $(addprefix $(OBJDIR)/, geode.o geohash.o timer.o bitmap_graphics.o rtree.o roaring.o)

# Include paths for the submodules
INCLUDES = -I./deps/libcount/include -I./deps/rtree -I./deps/roaring/include -I./deps/roaring/include/roaring

CFLAGS = -g -Wall -std=c11 -O3 $(INCLUDES)

LD_FLAGS = -lm -lstdc++

# Library path for libcount submodule
LIBCOUNT_A = ./deps/libcount/libcount.a

.submodules:
	git submodule update --init --recursive
	touch .submodules

.roaring: .submodules
	cd deps/roaring && ./amalgamation.sh
	touch .roaring

all : $(LIBCOUNT_A) ${PROGS}

# Rule for project source files
$(OBJDIR)/%.o : src/%.c .submodules
	@mkdir -p $(@D)
	gcc -c ${CFLAGS} -o $@ $<

# Specific rules for dependencies located in deps/
$(OBJDIR)/rtree.o : deps/rtree/rtree.c .submodules
	@mkdir -p $(@D)
	gcc -c ${CFLAGS} -o $@ $<

$(OBJDIR)/roaring.o : .submodules .roaring
	@mkdir -p $(@D)
	gcc -c ${CFLAGS} -o $@ deps/roaring/roaring.c

# Build libcount inside its submodule directory
$(LIBCOUNT_A): .submodules
	cd ./deps/libcount && $(MAKE)

# Linking the programs
# Using $(LIBCOUNT_A) directly ensures the binaries depend on the actual lib file
bench_ins : src/bench_ins.c ${OBJS} $(LIBCOUNT_A)
	gcc ${CFLAGS} -o $@ $^ $(LD_FLAGS)

bench_rand_ins : src/bench_rand_ins.c ${OBJS} $(LIBCOUNT_A)
	gcc ${CFLAGS} -o $@ $^ $(LD_FLAGS)

bench_query : src/bench_query.c ${OBJS} $(LIBCOUNT_A)
	gcc ${CFLAGS} -o $@ $^ $(LD_FLAGS)

bench_rtree : src/bench_rtree.c ${OBJS} $(LIBCOUNT_A)
	gcc ${CFLAGS} -o $@ $^ $(LD_FLAGS)

empirical_density : src/empirical_density.c ${OBJS} $(LIBCOUNT_A)
	gcc ${CFLAGS} -o $@ $^ $(LD_FLAGS)

clean :
	rm -rf ${PROGS} ${OBJDIR}
	cd deps/libcount && $(MAKE) clean
