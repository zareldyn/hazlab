hazlab:
	gcc -O2 -Wall -o hazlab -lGLU -lm -lglut -lGL *.c

clean:
	rm hazlab &>/dev/null
