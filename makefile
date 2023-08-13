all:
	[ -d build ] || mkdir build
	@g++ main.cpp src/bucket.cpp -o build/bucket -I./src

run:all
	@./build/bucket

debug:
	[ -d build ] || mkdir build
	@g++ -g main.cpp src/bucket.cpp -o build/bucket  -I./src
	@g++ -g main.cpp src/bucket.cpp -DTEST_ENABLED -o build/bucket.tst -I./src

test:
	[ -d build ] || mkdir build
	@mkdir testfiles
	@g++ -g main.cpp src/bucket.cpp -DTEST_ENABLED -o build/bucket.tst -I./src
	@echo ----------------- Compilation Ends ------------------------------
	@./build/bucket.tst
	@rm -rf testfiles

clean:
	rm -rf build

