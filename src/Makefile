.PHONY: all ant gauss winograd clean run_ant run_gauss run_winograd clang_format

all: clean ant gauss winograd

ant:
	$(MAKE) -C ./ant

gauss:
	$(MAKE) -C ./gauss

winograd:
	$(MAKE) -C ./winograd

run_ant:
	./ant.out

run_gauss:
	./gauss.out

run_winograd:
	./winograd.out

clean:
	@rm -f *.o *.out .clang-format

clang_format:
	cp ../materials/linters/.clang-format .clang-format
	clang-format -n */*/*.cpp */*/*.h
