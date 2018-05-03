target:=snmpdemo

CC=gcc
BIN_PATH:=/usr/sbin
TOPDIR=$(shell pwd)
SOURCES=$(wildcard $(TOPDIR)/*.c)

LDLIBS:=$(shell net-snmp-config --netsnmp-libs)
LDLIBS+=$(shell net-snmp-config --netsnmp-agent-libs)

objs:=$(patsubst %.c,%.o,$(notdir $(SOURCES)))

.PHONY:all clean install uninstall

all:$(target)

$(target):$(objs)
	$(CC) -o $@ $(objs) $(LDLIBS) 

clean:
	rm -f $(objs) $(target)
install:
	cp -f $(target) $(BIN_PATH)/$(target)
uninstall:
	rm -f $(BIN_PATH)/$(target)
