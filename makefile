###########################################
#
# Toxplash
# Copyright (C) 2018 Florian Evaldsson
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
###########################################

NAME := toxplash

VERSION := 0.1

OUTPUT := $(NAME)

CC := gcc
MEMTEST := valgrind
DEBUG := gdb

SRCS := $(wildcard ./src/*.c)

BUILD_DIR := .build
INC_DIR := inc

PKG_CONF_LIBS := libsodium libtoxcore glib-2.0

CFLAGS := -g
CFLAGS += $(shell pkg-config --cflags $(PKG_CONF_LIBS))
CFLAGS += -I./inc -I./inc/gen
CFLAGS += -std=gnu11 -D_GNU_SOURCE

LDFLAGS := $(shell pkg-config --libs $(PKG_CONF_LIBS))

INCLUDE_TEST := ./test
INCLUDE_TEST_BIN := $(INCLUDE_TEST)/gmrtest

OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(SRCS:.c=.o)))
HEADERS := $(addprefix $(INC_DIR)/gen/,$(notdir $(SRCS:.c=.h)))

DEPS := $(OBJS:.o=.d)

########### TARGETS ETC ###########

all: $(OUTPUT)
	
$(OUTPUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MT $@ -MMD -MF $(BUILD_DIR)/$*.d -c $< -o $@

inc/gen:
	mkdir -p $@

.PRECIOUS: inc/gen/%.h
inc/gen/%.h: src/%.c | inc/gen
	@$(shell OUT=$$(makeheaders -h $<) $(if $(wildcard $@),; if ! [ "$$OUT" == "$$(cat $@)" ]; then echo "$$OUT" > $@;echo "echo 'generating $@...'"; fi,> $@;echo "echo 'generating $@...'";))

%:
	@:

clean:
	rm -f $(OBJS) $(OUTPUT) $(HEADERS)

run: $(OUTPUT)
	./$< test
	
memtest: all
	$(MEMTEST) --track-origins=yes --tool=memcheck --leak-check=full --show-leak-kinds=all ./$(NAME) test
	
$(DEBUG):
	$(DEBUG) --args ./$(NAME) test

-include $(DEPS)

