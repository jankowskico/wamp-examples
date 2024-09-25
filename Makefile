SHELL=/bin/bash
CXX=g++
CXXFLAGS=-I./include -lboost_thread -lpthread -lssl -lcrypto


## all: install and build all (default)
.PHONY: all
all: env dist


## env: prepare development environment
.PHONY: env
env: include node_modules vendor venv


## include: install cpp libraries
include: include/autobahn/autobahn.hpp include/dotenv/dotenv.h include/websocketpp/version.hpp

include/autobahn/:
	mkdir -p include/autobahn

include/dotenv/:
	mkdir -p include/dotenv

include/websocketpp/:
	mkdir -p include/websocketpp

include/autobahn/autobahn.hpp: include/autobahn/
	[ -d /tmp/autobahn-cpp ] && rm -rf /tmp/autobahn-cpp || true
	git clone https://github.com/crossbario/autobahn-cpp.git /tmp/autobahn-cpp
	cp -rf /tmp/autobahn-cpp/autobahn include/
	rm -rf /tmp/autobahn-cpp

include/dotenv/dotenv.h: include/dotenv/
	[ -d /tmp/dotenv-cpp ] && rm -rf /tmp/dotenv-cpp || true
	git clone https://github.com/laserpants/dotenv-cpp.git /tmp/dotenv-cpp
	cp -rf /tmp/dotenv-cpp/include/laserpants/dotenv include/
	rm -rf /tmp/dotenv-cpp

include/websocketpp/version.hpp: include/websocketpp/
	[ -d /tmp/websocketpp ] && rm -rf /tmp/websocketpp || true
	git clone https://github.com/zaphoyd/websocketpp.git /tmp/websocketpp
	cp -rf /tmp/websocketpp/websocketpp include/
	rm -rf /tmp/websocketpp


## node_modules: install node modules
node_modules:
	npm install


## vendor: install php packages
vendor:
	composer install


## venv: install python packages in virtualenv
venv:
	virtualenv venv
	source venv/bin/activate && pip install -r requirements.txt


## dist: build cpp binaries
.PHONY: dist
dist: dist/example-call.bin dist/example-publish.bin dist/example-register.bin dist/example-subscribe.bin

dist/:
	mkdir dist

dist/example-call.bin: dist/ include/autobahn/autobahn.hpp include/dotenv/dotenv.h include/websocketpp/version.hpp
	$(CXX) $(CXXFLAGS) src/example-call.cpp -o dist/example-call.bin
	chmod 700 dist/example-call.bin

dist/example-publish.bin: dist/ include/autobahn/autobahn.hpp include/dotenv/dotenv.h include/websocketpp/version.hpp
	$(CXX) $(CXXFLAGS) src/example-publish.cpp -o dist/example-publish.bin
	chmod 700 dist/example-publish.bin

dist/example-register.bin: dist/ include/autobahn/autobahn.hpp include/dotenv/dotenv.h include/websocketpp/version.hpp
	$(CXX) $(CXXFLAGS) src/example-register.cpp -o dist/example-register.bin
	chmod 700 dist/example-register.bin

dist/example-subscribe.bin: dist/ include/autobahn/autobahn.hpp include/dotenv/dotenv.h include/websocketpp/version.hpp
	$(CXX) $(CXXFLAGS) src/example-subscribe.cpp -o dist/example-subscribe.bin
	chmod 700 dist/example-subscribe.bin


## clean: remove all
.PHONY: clean
clean: clean-env clean-dist

## clean-env: remove development environment
.PHONY: clean-env
clean: clean-include clean-node_modules clean-vendor clean-venv

## clean-include: remove cpp libraries
.PHONY: clean-include
clean-include:
	rm -rf include

## clean-node_modules: remove node modules
.PHONY: clean-node_modules
clean-node_modules:
	rm -rf node_modules

## clean-vendor: remove php packages
.PHONY: clean-vendor
clean-vendor:
	rm -rf vendor

## clean-venv: remove python packages and virtualenv
.PHONY: clean-venv
clean-venv:
	rm -rf venv

## clean-dist: remove built cpp binaries
.PHONY: clean-dist
clean-dist:
	rm -rf dist


## help: display help
.PHONY: help
help: Makefile
	$(eval LENGTH := $(shell awk '/^## [a-zA-Z0-9_-]+:/ { if ( length($$2) > longest ) { longest = length($$2) } } END { print longest }' $<))
	@awk '/^## [a-zA-Z0-9_-]+:/{ printf "%$(shell echo $$(( $(LENGTH) )) )s", $$2; print substr($$0, length($$2) + 4) } \
	/^## [^a-zA-Z0-9_-]+[^:]/{ printf "%$(shell echo $$(( $(LENGTH) + 1 )) )s", ""; print substr($$0, match($$0, "[a-zA-Z]")) }' $<
