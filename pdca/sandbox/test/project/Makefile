#!/bin/bash

rm -rf class/common/*
javac -d class/ -cp class/ src/common/*.java

rm -rf scheduler.jar class/scheduler/*
javac -d class/ -cp class/ src/scheduler/*.java
jar cfe scheduler.jar scheduler/Scheduler -C class/ scheduler -C class/ common

rm -rf worker.jar class/worker/*
javac -d class/ -cp class/ src/worker/*.java
jar cfe worker.jar worker/Worker -C class/ worker -C class/ common

rm -rf jobs.jar class/jobs/*
javac -d class/ -cp class/ src/jobs/*.java
jar cf jobs.jar -C class/ jobs

rm -rf client.jar class/client/*
javac -d class/ -cp class/ src/client/*.java
jar cfe client.jar client/Client -C class/ client -C class/ common
