#!/bin/bash

rm -rf jobs.jar class/jobs/*
javac -d class/ -cp class/ src/jobs/*.java
jar cf jobs.jar -C class/ jobs
