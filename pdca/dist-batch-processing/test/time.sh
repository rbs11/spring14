#!/bin/bash
T=$(date +%s%3N)

java -jar client.jar localhost 51000 jobs.jar jobs.$1

T=$(($(date +%s%3N)-T))
echo "_info $1 time $T"

