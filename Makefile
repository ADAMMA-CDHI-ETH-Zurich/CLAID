.PHONY: all_packages
all_packages:
	rm -fr build_packages
	mkdir build_packages
	cd dispatch/python && make pip_package && cd  ../../ && mv dispatch/python/pip_package build_packages/pip_package