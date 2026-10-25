#
# A Makefile for osFree Janus Clock
# (c) osFree project
#

DESCRIPTION = Janus Clock
TARGET_VERSION= 310
TARGET_PACKAGE = DISK02
EXPORTS = CLOCK_WndProc


!include $(%ROOT)tools/mk/build.mk

