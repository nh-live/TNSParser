# Top Makefile for C program
# # Copyright (C) 2014 shallnew \at 163 \dot com

SRCS    := $(wildcard ./*.c)
SRCS    += $(wildcard ./libkdnetwork/*.c)
SRCS    += $(wildcard ./libprocess/*.c)
SRCS    += $(wildcard ./libcrypt/*.c)
OBJS    := $(SRCS:.c=.o) 
LDFLAGS := -lpthread -lm
DEFINES := 
INCLUDE := -I./
PRILIBS := 
CFLAGS  := -g -Wall $(DEFINES) $(INCLUDE) -fno-strict-aliasing -Werror
CC		:= gcc
TARGET  := tnsparser
modules := 


all : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(PRILIBS) $(LDFLAGS)

clean :
	rm -rf $(OBJS) $(TARGET)
	
.PHONY : all clean
