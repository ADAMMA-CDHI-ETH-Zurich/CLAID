.PHONY: pip_package
pip_package:
	rm -fr pip_package
	mkdir -p pip_package
	bazel build :py_claid
	cp -r ../../bazel-bin/dispatch/python/py_claid.runfiles/claid-core/ pip_package/claid
	mv pip_package/claid/dispatch/python/** pip_package/claid/
	rm -fr pip_package/claid/dispatch/python

