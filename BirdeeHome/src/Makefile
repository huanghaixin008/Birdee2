

CUR_DIR=$(shell pwd)
PARENT_DIR=$(shell dirname $(CUR_DIR))
BLIB_DIR=$(PARENT_DIR)/blib
BIN=$(PARENT_DIR)/bin
LIB=$(PARENT_DIR)/lib

all: ${BLIB_DIR}/birdee.o libs

print-%  : ; @echo $* = $($*)

${LIB}:
	rm -f $(PARENT_DIR)/lib
	ln -s $(PARENT_DIR)/../lib ${LIB}

${BIN}:
	rm -f $(PARENT_DIR)/bin
	ln -s $(PARENT_DIR)/../bin ${BIN}

${BLIB_DIR}:
	mkdir -p ${BLIB_DIR}

${BLIB_DIR}/birdee.o: birdee.txt ${BLIB_DIR} ${BIN}
	$(BIN)/birdeec --corelib -i birdee.txt -o $(BLIB_DIR)/birdee.o

libs: ${BLIB_DIR}/birdee.o
	python3 $(PARENT_DIR)/pylib/bbuild.py -i . -o ${BLIB_DIR} \
		variant list hash tuple fmt vector queue stack unsafe concurrent.threading rtti system.io.stdio typedptr reflection concurrent.sync system.io.net \
		functional.lazy concurrent.threadpool system.time extensions.string system.io.stream serialization.json.serializer serialization.json.deserializer

clean:
	rm -rf ${BLIB_DIR}
