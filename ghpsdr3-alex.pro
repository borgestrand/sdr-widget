# This is a project file so you can use QT Creator as your IDE.

TEMPLATE = subdirs

QtRadio.subdir = trunk/src/QtRadio
softrock.subdir = trunk/src/softrock
softrock.makefile = Makefile

SUBDIRS += QtRadio softrock
