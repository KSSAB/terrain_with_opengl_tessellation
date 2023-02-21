CPPFLAGS := -g -std=c++17 -Wall -Wextra -fsanitize=address
DOTOS := main.o gl.o quad.o
SHADERS := terrain_tess_control.hpp terrain_tess_eval.hpp terrain_vertex.hpp terrain_frag.hpp

all: ${DOTOS}
	g++ ${DOTOS} ${CPPFLAGS} -static-libasan -lGL -lglfw -lGLEW -lSOIL

clean:
	rm -f a.out
	rm -f ${DOTOS}

optimal_add_san: new_flags := -O3 -std=c++17 -Wall -Wextra -fsanitize=address
optimal_add_san:
	$(MAKE) clean
	$(MAKE) all CPPFLAGS='$(new_flags)'

optimal_no_san: new_flags := -O3 -std=c++17 -Wall -Wextra
optimal_no_san:
	$(MAKE) clean
	$(MAKE) all CPPFLAGS='$(new_flags)'

main.o: main.cpp ${SHADERS}
	g++ -c main.cpp ${CPPFLAGS}

gl.o: gl.hpp gl.cpp
	g++ -c gl.cpp ${CPPFLAGS}

quad.o: quad.hpp quad.cpp
	g++ -c quad.cpp ${CPPFLAGS}
