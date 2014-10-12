#!/bin/bash

rm -rf client.jar class/client/*
javac -d class/ -cp class/ src/client/*.java
jar cfe client.jar client/Client -C class/ client -C class/ common
