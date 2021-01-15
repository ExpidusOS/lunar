SUBDIRS =								\
	design								\
	papers								\
	reference

doc_DATA =								\
	README.gtkrc

man_MANS =								\
	Lunar.1

if MAINTAINER_MODE
%.1: %.xml
	xsltproc -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<
endif

EXTRA_DIST =								\
	$(doc_DATA)							\
	$(man_MANS)							\
	Lunar.xml

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
