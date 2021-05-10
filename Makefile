clock:
	g++ -w -Wall -o clock clock.cpp forces.cpp particle.cpp vector.cpp -lGL -lGLU -lglut
	
clean:
	-rm -f clock

run:
	make clean
	make clock
	./clock