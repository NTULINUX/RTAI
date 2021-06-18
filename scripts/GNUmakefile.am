install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) rtai-load $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) rtai-config $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) $(srcdir)/rtai-info $(DESTDIR)$(bindir)

EXTRA_DIST = rtai-info bootstrap
