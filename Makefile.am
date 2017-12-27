lib_LTLIBRARIES = librouting_greedy.la librouting_filestatic.la librouting_geostatic.la librouting_aodv.la librouting_directed_diffusion.la librouting_olsrv2.la librouting_oracenet.la

librouting_greedy_la_CFLAGS = $(CFLAGS) $(GLIB_FLAGS) $(GSL_FLAGS) $(XML_FLAGS) -Wall
librouting_greedy_la_SOURCES = greedy.c
librouting_greedy_la_LDFLAGS = -module

librouting_filestatic_la_CFLAGS = $(CFLAGS) $(GLIB_FLAGS) $(GSL_FLAGS) $(XML_FLAGS) -Wall
librouting_filestatic_la_SOURCES = filestatic.c
librouting_filestatic_la_LDFLAGS = -module

librouting_geostatic_la_CFLAGS = $(CFLAGS) $(GLIB_FLAGS) $(GSL_FLAGS) $(XML_FLAGS) -Wall
librouting_geostatic_la_SOURCES = geostatic.c
librouting_geostatic_la_LDFLAGS = -module

librouting_aodv_la_CFLAGS = $(CFLAGS) $(GLIB_FLAGS) $(GSL_FLAGS) $(XML_FLAGS) -Wall
librouting_aodv_la_SOURCES = aodv.c routing_rreq_management.c routing_neighbors_management.c routing_routes_management.c
librouting_aodv_la_LDFLAGS = -module

librouting_olsrv2_la_CFLAGS = $(CFLAGS) $(GLIB_FLAGS) $(GSL_FLAGS) $(XML_FLAGS) -Wall
librouting_olsrv2_la_SOURCES = olsrv2.c routing_rreq_management.c routing_neighbors_management.c routing_routes_management.c
librouting_olsrv2_la_LDFLAGS = -module

librouting_directed_diffusion_la_CFLAGS = $(CFLAGS) $(GLIB_FLAGS) $(GSL_FLAGS) $(XML_FLAGS) -Wall
librouting_directed_diffusion_la_SOURCES = directed_diffusion.c routing_rreq_management.c routing_sink_interest_management.c routing_neighbors_management.c routing_routes_management.c
librouting_directed_diffusion_la_LDFLAGS = -module


librouting_oracenet_la_CFLAGS = $(CFLAGS) $(GLIB_FLAGS) $(GSL_FLAGS) $(XML_FLAGS) -Wall
librouting_oracenet_la_SOURCES = oracenet.c routing_rreq_management.c routing_sink_interest_management.c routing_neighbors_management.c routing_routes_management.c
librouting_oracenet_la_LDFLAGS = -module

