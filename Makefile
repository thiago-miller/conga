CC             = gcc
CFLAGS         = -Wall -O2 -DHAVE_VERSION_H -I$(BUILD_SRC_DIR)
LDFLAGS_TEST   = -Wl,--wrap=malloc -Wl,--wrap=calloc
LDLIBS_TEST    = -lcheck
SRC_DIR        = src
TEST_DIR       = tests
BUILD_DIR      = builddir
VERSION_FILE   = version.h

BUILD_SRC_DIR  = $(BUILD_DIR)/$(SRC_DIR)
SRCS           = $(filter-out %/main.c,$(wildcard $(SRC_DIR)/*.c))
OBJS           = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_SRC_DIR)/%.o)
TARGET         = conga
TARGET_LIB     = libconga.a

BUILD_TEST_DIR = $(BUILD_DIR)/$(TEST_DIR)
TEST_SRCS      = $(filter-out %/check_conga_main.c,$(wildcard $(TEST_DIR)/*.c))
TEST_OBJS      = $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_TEST_DIR)/%.o)
TEST_TARGET    = check_conga

VALGRIND       = valgrind --leak-check=full --show-leak-kinds=all
VCS_TAG        = git describe --tags --dirty=+

.PHONY: all clean test test-valgrind vcs-tag

all: $(BUILD_SRC_DIR)/$(TARGET)

$(BUILD_SRC_DIR)/$(TARGET): $(SRC_DIR)/main.c $(BUILD_SRC_DIR)/$(TARGET_LIB)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_SRC_DIR)/$(TARGET_LIB): $(OBJS)
	ar rcs $@ $^

$(BUILD_SRC_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_SRC_DIR)/$(VERSION_FILE) | $(BUILD_SRC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

vcs-tag $(BUILD_SRC_DIR)/$(VERSION_FILE): | $(BUILD_SRC_DIR)
	printf '#define VERSION "%s"\n' $$($(VCS_TAG)) \
		> $(BUILD_SRC_DIR)/$(VERSION_FILE)

test: $(BUILD_TEST_DIR)/$(TEST_TARGET)
	./$^ 2> /dev/null

test-valgrind: $(BUILD_TEST_DIR)/$(TEST_TARGET)
	$(VALGRIND) ./$^

$(BUILD_TEST_DIR)/$(TEST_TARGET): $(TEST_DIR)/check_conga_main.c $(TEST_OBJS) $(BUILD_SRC_DIR)/$(TARGET_LIB)
	$(CC) $(CFLAGS) $^ $(LDLIBS_TEST) $(LDFLAGS_TEST) -o $@

$(BUILD_TEST_DIR)/check_conga_%.o: $(TEST_DIR)/check_conga_%.c $(SRC_DIR)/%.c | $(BUILD_TEST_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_SRC_DIR) $(BUILD_TEST_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
