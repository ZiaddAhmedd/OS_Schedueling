build:
	gcc process_generator.c -o process_generator.out -lm
	gcc clk.c -o clk.out -lm
	gcc scheduler.SJF.c -o scheduler.SJF.out -lm
	gcc scheduler.HPF.c -o scheduler.HPF.out -lm
	gcc process.c -o process.out -lm
	gcc test_generator.c -o test_generator.out -lm

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out processes.txt -sch 5 -q 2
