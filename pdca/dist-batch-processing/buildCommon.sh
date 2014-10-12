#!/bin/bash

rm -rf class/common/*
javac -d class/ -cp class/ src/common/*.java
