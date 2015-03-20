
TEMPLATE = subdirs
SUBDIRS = picojpeg src tests

src.depends = picojpeg
tests.depends = src

