all: build test

build: .FORCE
	python setup.py build

test:
	python _ax25_test.py


.FORCE:
