hazlab:
	gcc -O2 -Wall *.c -o hazlab -lGLU -lm -lglut -lGL

clean:
	rm hazlab &>/dev/null
