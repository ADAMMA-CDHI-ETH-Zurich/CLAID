
.PHONY: pip_package
pip_package:
	rm -fr pip_package
	mkdir -p pip_package
	bazel build :py_claid
	cp -L -r ../../../bazel-bin/dispatch/python/claid/py_claid.runfiles/claid-core/ pip_package/claid
	mv pip_package/claid/dispatch/python/claid/** pip_package/claid/
	rm -fr pip_package/claid/dispatch/python
	rm -fr pip_package/claid/external
	mv pip_package/claid/setup.py pip_package/setup.py
	mkdir -p pip_package/claid/dispatch/core/Darwin/x86_64
	mkdir -p pip_package/claid/dispatch/core/Darwin/arm64
	mkdir -p pip_package/claid/dispatch/core/Linux/x86_64
	mkdir -p pip_package/claid/dispatch/core/Linux/x86
	mkdir -p pip_package/claid/dispatch/core/Linux/arm64

	cp MANIFEST.in pip_package/MANIFEST.in
	cp README.md pip_package/README.md
	cp LICENSE.txt pip_package/LICENSE.txt
	cp NOTICES pip_package/NOTICES
	cp -r graphics pip_package/graphics
