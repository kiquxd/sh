run:
	gcc src/main.c -o shell && ./shell

test:
	mkdir -p build && \
	cd build && \
	cmake .. && \
	cmake --build . && \
	ctest --output-on-failure

PHONY: run test