#!/bin/bash

rm -rf scheduler.jar class/scheduler/*
javac -d class/ -cp class/ src/scheduler/*.java
jar cfe scheduler.jar scheduler/Scheduler -C class/ scheduler -C class/ common
