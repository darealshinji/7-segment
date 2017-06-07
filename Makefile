CLEANFILES = simple_stopwatch simple_timer simple_clock main stage2
DISTCLEANFILES = fltk


all:
	./build.sh

clean:
	-rm -rf $(CLEANFILES)
	if [ -d fltk-src ] && [ ! -f fltk-src/.clean_stamp ]; then \
	  $(MAKE) -C fltk-src distclean ; \
	  touch fltk-src/.clean_stamp ; \
	fi

distclean: clean
	-rm -rf $(DISTCLEANFILES)

maintainer-clean:
	-rm -rf $(CLEANFILES) $(DISTCLEANFILES)
	-rm -rf fltk-src ressources.h

