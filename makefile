#
# A Makefile for osFree Janus Clock
# (c) osFree project
#

DESCRIPTION = osFree Janus Clock
SOURCES = main winclock
TARGET_VERSION= 310
EXPORTS = CLOCK_WndProc

# defines additional options for C compiler
ADD_COPT = -sg


!include $(%ROOT)tools/mk/build.mk

