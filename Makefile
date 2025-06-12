CC             = gcc
SHELL          = bash -euo pipefail
CFLAGS         = -Wall -O2 $$(pkg-config --cflags ncurses) -DHAVE_VERSION_H -DHAVE_PATTERN_DEFS_H -I$(BUILD_SRC_DIR)
LDLIBS         = $$(pkg-config --libs ncurses) -lm
LDFLAGS_TEST   = -Wl,--wrap=malloc -Wl,--wrap=calloc
LDLIBS_TEST    = -lcheck
SRC_DIR        = src
SCRIPTS_DIR    = scripts
TEST_DIR       = tests
DATA_DIR       = data
BUILD_DIR      = builddir

BUILD_SRC_DIR  = $(BUILD_DIR)/$(SRC_DIR)
SRCS           = $(filter-out %/main.c,$(wildcard $(SRC_DIR)/*.c))
OBJS           = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_SRC_DIR)/%.o)
TARGET         = conga
TARGET_LIB     = libconga.a

BUILD_TEST_DIR = $(BUILD_DIR)/$(TEST_DIR)
TEST_SRCS      = $(filter-out %/check_conga_main.c,$(wildcard $(TEST_DIR)/*.c))
TEST_OBJS      = $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_TEST_DIR)/%.o)
TEST_TARGET    = check_conga

BUILD_LOG_DIR  = $(BUILD_DIR)/log
BUILD_LOG      = build.log
TEST_LOG       = test.log
VALGRIND_LOG   = test_valgrind.log

VALGRIND       = valgrind --leak-check=full --show-leak-kinds=all
VCS_TAG        = git describe --tags --dirty=+

.PHONY: all clean test test-valgrind vcs-tag

all: | $(BUILD_LOG_DIR)
	@if $(MAKE) --question $(BUILD_SRC_DIR)/$(TARGET); then \
		echo "Target up to date. Nothing to do"; \
	else \
		bash $(SCRIPTS_DIR)/logrotate.sh $(BUILD_LOG_DIR)/$(BUILD_LOG); \
		$(MAKE) --no-print-directory $(BUILD_SRC_DIR)/$(TARGET) 2>&1 \
		| tee $(BUILD_LOG_DIR)/$(BUILD_LOG) \
			&& echo "Build successful" \
			|| (echo "Build failed. See $(BUILD_LOG_DIR)/$(BUILD_LOG)"; exit 1); \
	fi

$(BUILD_SRC_DIR)/$(TARGET): $(SRC_DIR)/main.c $(BUILD_SRC_DIR)/$(TARGET_LIB)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_SRC_DIR)/$(TARGET_LIB): $(OBJS)
	ar rcs $@ $^

$(BUILD_SRC_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h | $(BUILD_SRC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_SRC_DIR)/config.o: $(BUILD_SRC_DIR)/version.h
$(BUILD_SRC_DIR)/pattern.o: $(BUILD_SRC_DIR)/pattern_defs.h

vcs-tag $(BUILD_SRC_DIR)/version.h: | $(BUILD_SRC_DIR)
	printf '#define VERSION "%s"\n' $$($(VCS_TAG)) \
		> $(BUILD_SRC_DIR)/version.h

$(BUILD_SRC_DIR)/pattern_defs.h: $(DATA_DIR)/pattern_defs.json | $(BUILD_SRC_DIR)
	perl $(SCRIPTS_DIR)/convert_patterns.pl $< > $@

test: $(BUILD_TEST_DIR)/$(TEST_TARGET) | $(BUILD_LOG_DIR)
	@echo "Running tests..."
	@bash $(SCRIPTS_DIR)/logrotate.sh $(BUILD_LOG_DIR)/$(TEST_LOG)
	@./$^ > >(tee -a $(BUILD_LOG_DIR)/$(TEST_LOG)) \
		2> >(tee -a $(BUILD_LOG_DIR)/$(TEST_LOG) > /dev/null) \
		&& echo "Tests OK" \
		|| (echo "See $(BUILD_LOG_DIR)/$(TEST_LOG)"; exit 1;)

test-valgrind: $(BUILD_TEST_DIR)/$(TEST_TARGET) | $(BUILD_LOG_DIR)
	@echo "Running valgrind tests..."
	@bash $(SCRIPTS_DIR)/logrotate.sh $(BUILD_LOG_DIR)/$(VALGRIND_LOG)
	@$(VALGRIND) ./$^ > $(BUILD_LOG_DIR)/$(VALGRIND_LOG) 2>&1
	@NUM_LEAKS=$$(awk '/are definitely lost/ {c++} END {print c}' $(BUILD_LOG_DIR)/$(VALGRIND_LOG)); \
	if [[ "$$NUM_LEAKS" -gt 0 ]]; then \
		echo "ERROR: $$NUM_LEAKS memory leaks were detected by valgrind"; \
		echo "See $(BUILD_LOG_DIR)/$(VALGRIND_LOG)"; \
		exit 1; \
	else \
		echo "No memory leaks found"; \
	fi

$(BUILD_TEST_DIR)/$(TEST_TARGET): $(TEST_DIR)/check_conga_main.c $(TEST_OBJS) $(BUILD_SRC_DIR)/$(TARGET_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS_TEST) $^ $(LDLIBS_TEST) -o $@

$(BUILD_TEST_DIR)/check_conga_%.o: $(TEST_DIR)/check_conga_%.c $(SRC_DIR)/%.c | $(BUILD_TEST_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS_TEST) -c $< -o $@

$(BUILD_SRC_DIR) $(BUILD_TEST_DIR) $(BUILD_LOG_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
