N = 50
OUTPUT = output
SEED = 0

.PHONY: run show clean

run: hamilton
	./$< ${N} ${OUTPUT} ${SEED}

${OUTPUT}: hamilton
	${MAKE} run

hamilton: hamilton.cpp
	g++ -O3 -DNDEBUG -std=c++17 -o $@ $<

show: ${OUTPUT}
	python show.py $<

clean:
	-rm -rf hamilton ${OUTPUT}
