#!/bin/bash

rm -rf worker.jar class/worker/*
javac -d class/ -cp class/ src/worker/*.java
jar cfe worker.jar worker/Worker -C class/ worker -C class/ common
