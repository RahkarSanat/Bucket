SOURCE_DIR := src
SOURCE_FILES := $(wildcard $(SOURCE_DIR)/*.cpp)

all:$(SOURCE_FILES)
	[ -d build ] || mkdir build
	@g++ main.cpp src/bucket.cpp src/queue.cpp src/circular_queue.cpp -o build/bucket -I./src

run:all
	@./build/bucket

debug:$(SOURCE_FILES)
	[ -d build ] || mkdir build
	@g++ -g main.cpp src/bucket.cpp src/queue.cpp -o build/bucket  -I./src
	@g++ -g main.cpp src/bucket.cpp src/queue.cpp -DTEST_ENABLED -o build/bucket.tst -I./src

test:$(SOURCE_FILES)
	[ -d build ] || mkdir build
	@mkdir testfiles
	@g++ -g main.cpp src/bucket.cpp src/queue.cpp -DTEST_ENABLED -o build/bucket.tst -I./src
	@echo ----------------- Compilation Ends ------------------------------
	@./build/bucket.tst
	@rm -rf testfiles

clean:
	rm -rf build

