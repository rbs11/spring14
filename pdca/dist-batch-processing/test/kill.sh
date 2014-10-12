#!/bin/bash

for i in $(jps | grep "jar" | cut -d" " -f1)
do
  kill -kill $i
done
