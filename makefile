all:
	[ -d build ] || mkdir build
	@g++ main.cpp bucket.cpp -o build/bucket 

run:all
	@./build/bucket

debug:
	[ -d build ] || mkdir build
	g++ -g main.cpp bucket.cpp -o build/bucket 

test:
	[ -d build ] || mkdir build
	@mkdir testfiles
	@g++ main.cpp bucket.cpp -DTEST_ENABLED -o build/bucket.tst
	@echo ----------------- Compilation Ends ------------------------------
	@./build/bucket.tst
	@rm -rf testfiles

clean:
	rm -rf build

