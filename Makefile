all: fabric

clock:
	cd ntp-tool; make
	cp ntp-tool/ntp ./

fabric:
	./python-packages/install_packages.sh

	
