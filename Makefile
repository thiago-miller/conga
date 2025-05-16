CC             = gcc
CFLAGS         = -Wall -O2
LDFLAGS_TEST   = -Wl,--wrap=malloc -Wl,--wrap=calloc
LDLIBS_TEST    = -lcheck
SRC_DIR        = src
TEST_DIR       = tests
BUILD_DIR      = builddir

BUILD_SRC_DIR  = $(BUILD_DIR)/$(SRC_DIR)
SRCS           = $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c))
OBJS           = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_SRC_DIR)/%.o)
TARGET         = conga
TARGET_LIB     = libconga.a

BUILD_TEST_DIR = $(BUILD_DIR)/$(TEST_DIR)
TEST_SRCS      = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS      = $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_TEST_DIR)/%.o)
TEST_TARGET    = check_conga

VALGRIND       = valgrind --leak-check=full --show-leak-kinds=all

.PHONY: all clean test test-valgrind

all: $(BUILD_SRC_DIR)/$(TARGET)

test: $(BUILD_TEST_DIR)/$(TEST_TARGET)
	./$^ 2> /dev/null

test-valgrind: $(BUILD_TEST_DIR)/$(TEST_TARGET)
	$(VALGRIND) ./$^

$(BUILD_SRC_DIR)/$(TARGET): $(SRC_DIR)/main.c $(BUILD_SRC_DIR)/$(TARGET_LIB)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_SRC_DIR)/$(TARGET_LIB): $(OBJS)
	ar rcs $@ $^

$(BUILD_SRC_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_SRC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_TEST_DIR)/$(TEST_TARGET): $(TEST_OBJS) $(BUILD_SRC_DIR)/$(TARGET_LIB)
	$(CC) $(CFLAGS) $^ $(LDLIBS_TEST) $(LDFLAGS_TEST) -o $@

$(BUILD_TEST_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_TEST_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_SRC_DIR) $(BUILD_TEST_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
