.PHONY: pip_package
pip_package:
	rm -fr pip_package
	mkdir -p pip_package
	bazel build :py_claid
	cp -r ../../bazel-bin/dispatch/python/py_claid.runfiles/claid-core/ pip_package/claid
	mv pip_package/claid/dispatch/python/** pip_package/claid/
	rm -fr pip_package/claid/dispatch/python
	rm -fr pip_package/claid/external
	mv pip_package/claid/setup.py pip_package/setup.py
	cp MANIFEST.in pip_package/MANIFEST.in
	cp README.md pip_package/README.md
	cp LICENSE.txt pip_package/LICENSE.txt
	cp NOTICES pip_package/NOTICES
	cp -r graphics pip_package/graphics
