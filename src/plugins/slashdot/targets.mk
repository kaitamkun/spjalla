build/plugins/slashdot/%.o: src/plugins/slashdot/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(strip $(ALLFLAGS) $(INCLUDE_SP) $(INCLUDE_LIBS) $(CFLAGS_TINYXML2)) -c $< -o $@

build/plugins/slashdot.dylib: $(patsubst %.cpp,%.o,$(addprefix build/plugins/slashdot/,$(filter %.cpp,$(shell ls "src/plugins/slashdot")))) $(MAINLIB)
	$(CC) $(SHARED_FLAG) $+ -o $@ $(LDFLAGS) $(LD_TINYXML2)