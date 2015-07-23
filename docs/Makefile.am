SUBDIRS = images

if ENABLE_DOCUMENTATION

doxyfile_deps = $(top_srcdir)/gtkmm-plplot/canvas.h \
								$(top_srcdir)/gtkmm-plplot/exception.h \
								$(top_srcdir)/gtkmm-plplot/plot2d.h \
								$(top_srcdir)/gtkmm-plplot/plot2ddata.h \
								$(top_srcdir)/gtkmm-plplot/enums.h \
								$(top_srcdir)/gtkmm-plplot/utils.h


doxyfile.stamp: $(doxyfile_deps) Doxyfile extra_pages.dox doxygen_hack.css DoxygenLayout.xml
	$(DOXYGEN) Doxyfile
	echo Timestamp > doxyfile.stamp

gtkmm_plplot_docdir = $(datadir)/doc/gtkmm-plplot/html


install-data-local:
	$(MKDIR_P) $(DESTDIR)$(gtkmm_plplot_docdir)
	$(MKDIR_P) $(DESTDIR)$(gtkmm_plplot_docdir)/search
	$(INSTALL_DATA) $(builddir)/html/*.css $(DESTDIR)$(gtkmm_plplot_docdir)
	$(INSTALL_DATA) $(builddir)/html/*.html $(DESTDIR)$(gtkmm_plplot_docdir)
	$(INSTALL_DATA) $(builddir)/html/*.js $(DESTDIR)$(gtkmm_plplot_docdir)
	$(INSTALL_DATA) $(builddir)/html/*.map $(DESTDIR)$(gtkmm_plplot_docdir)
	$(INSTALL_DATA) $(builddir)/html/*.md5 $(DESTDIR)$(gtkmm_plplot_docdir)
	$(INSTALL_DATA) $(builddir)/html/*.png $(DESTDIR)$(gtkmm_plplot_docdir)
	$(INSTALL_DATA) $(builddir)/html/search/*.css $(DESTDIR)$(gtkmm_plplot_docdir)/search
	$(INSTALL_DATA) $(builddir)/html/search/*.html $(DESTDIR)$(gtkmm_plplot_docdir)/search
	$(INSTALL_DATA) $(builddir)/html/search/*.js $(DESTDIR)$(gtkmm_plplot_docdir)/search
	$(INSTALL_DATA) $(builddir)/html/search/*.png $(DESTDIR)$(gtkmm_plplot_docdir)/search

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
clean-local:
	rm -rf html
uninstall-local:
	rm -rf $(DESTDIR)$(gtkmm_plplot_docdir)
endif
EXTRA_DIST = extra_pages.dox doxygen_hack.css DoxygenLayout.xml