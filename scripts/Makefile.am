AUTOMAKE_OPTIONS = foreign
bin_SCRIPTS = mpirun-opt.sh mpirun-prof.sh mpirun-ubmt.sh machine.py mk-app-data.py read.py ric.py var-freq-est.py var-rapl-est.py mk-ubmt-table.py

mpirun-opt.sh: mpirun-opt.base.sh
	$(SED) "s|TOPDIR|@prefix@|" $< >$@

mpirun-prof.sh: mpirun-prof.base.sh
	$(SED) "s|TOPDIR|@prefix@|" $< >$@

mpirun-ubmt.sh: mpirun-ubmt.base.sh
	$(SED) "s|TOPDIR|@prefix@|" $< >$@

machine.py: machine.base.py
	$(SED) "s|TOPDIR|@prefix@|" $< >$@

CLEANFILES = mpirun-opt.sh mpirun-prof.sh mpirun-ubmt.sh machine.py

EXTRA_DIST = mpirun-opt.base.sh mpirun-prof.base.sh mpirun-ubmt.base.sh machine.base.py mk-app-data.py read.py ric.py var-freq-est.py var-rapl-est.py mk-ubmt-table.py
