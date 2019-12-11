
all: $(OUT_DIR) autoclose_test.test array_literal.test class_inherit_import.test container_test.test concurrent_test.test defer_test.test exception_test.test file_test.test \
    functype_test.test gc_test.test hardware_exception_test.test import_test.test logic_obj_cmp.test net_test.ntest operators.test \
	rtti2.test template_link_test.test threading_test.test threadpool_test.test typedptr_test.test vector_test.test reflection_test.test unary_op_test.test \
	ast_write_test.pytest scripttest.pytest template_vararg.pytest getset_test.pytest \
	py_module_test.module
SRC_DIR=.
OUT_DIR=bin


$(OUT_DIR):
	cmd /c "if not exist $@ mkdir $@"

.SUFFIXES : .test .txt .bdm .py .pytest .module

.txt.test: 
	python3 $(BIRDEE_HOME)\pylib\bbuild.py -i $(SRC_DIR) -o $(OUT_DIR) -le $(OUT_DIR)\$*.exe $*
	$(OUT_DIR)\$*.exe

.bdm.test: 
	python3 $(BIRDEE_HOME)\pylib\bbuild.py -i $(SRC_DIR) -o $(OUT_DIR) -le $(OUT_DIR)\$*.exe $*
	$(OUT_DIR)\$*.exe

.bdm.ntest: 
	python3 $(BIRDEE_HOME)\pylib\bbuild.py -i $(SRC_DIR) -o $(OUT_DIR) -le $(OUT_DIR)\$*.exe -lc "ws2_32.lib" $*
	$(OUT_DIR)\$*.exe

.py.pytest:
	$(BIRDEE_HOME)\bin\birdeec.exe -s -i $*.py -o bin\$*.obj

.txt.module: 
	python3 $(BIRDEE_HOME)\pylib\bbuild.py -i $(SRC_DIR) -o $(OUT_DIR) $*

.bdm.module: 
	python3 $(BIRDEE_HOME)\pylib\bbuild.py -i $(SRC_DIR) -o $(OUT_DIR) $*

clean:
	del /S $(OUT_DIR)\*.bmm
	del /S $(OUT_DIR)\*.obj
	del /S $(OUT_DIR)\*.exe
	del /S $(OUT_DIR)\*.log
	del /S $(OUT_DIR)\*.pdb
	del /S $(OUT_DIR)\*.manifest

remake: clean all