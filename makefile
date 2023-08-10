all:
	[ -d build ] || mkdir build
	g++ main.cpp bucket.cpp -o build/bucket 

run:all
	./build/bucket

test:
	@g++ main.cpp bucket.cpp -DTEST_ENABLED -o build/bucket.tst
	@echo ----------------- Compilation Ends ------------------------------
	@./build/bucket.tst
	@rm test.txt

clean:
	rm -rf build

