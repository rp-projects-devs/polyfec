# polyfec - code polynomial correcteur d'erreurs

TARGET    := polyfec
SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build

WARNINGS  := -Wall -Wextra -Werror -pedantic -std=c17
CFLAGS    ?= -O2
CPPFLAGS  += -I$(INC_DIR)

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all test unit-test cli-test sanitize valgrind format check-format clean help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(WARNINGS) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(WARNINGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

## Tests

test: unit-test cli-test

unit-test: $(TARGET)
	./$(TARGET) test

cli-test: $(TARGET)
	bash tests/cli_tests.sh ./$(TARGET)

## Outils de verification

sanitize: clean
	$(MAKE) CFLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" \
	        LDFLAGS="-fsanitize=address,undefined" test

valgrind: clean
	$(MAKE) CFLAGS="-O0 -g" $(TARGET)
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET) test
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET) tests/data/message.txt 0.001 > /dev/null

format:
	clang-format -i $(SRC_DIR)/*.c $(INC_DIR)/*.h

check-format:
	clang-format --dry-run --Werror $(SRC_DIR)/*.c $(INC_DIR)/*.h

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

help:
	@echo "make               compile ./$(TARGET)"
	@echo "make test          tests unitaires et tests de la ligne de commande"
	@echo "make sanitize      tests avec AddressSanitizer et UBSan"
	@echo "make valgrind      tests sous valgrind (fuites memoire)"
	@echo "make format        formate le code avec clang-format"
	@echo "make check-format  verifie le formatage sans modifier les fichiers"
	@echo "make clean         supprime les fichiers generes"

-include $(DEPS)