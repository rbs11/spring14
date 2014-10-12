#!/bin/bash

sleep 1
./kill.sh
sleep 1
(java -jar scheduler.jar 51000) &
sleep 1
(java -jar worker.jar localhost 51000 51001) &
(java -jar worker.jar localhost 51000 51002) &
(java -jar worker.jar localhost 51000 51003) &
(java -jar worker.jar localhost 51000 51004) &
sleep 1

(./time.sh Job_256_250 | grep _info) &
j=$!
(./time.sh Job_16_4000 | grep _info) &
j=$(echo $j $!)

wait $j

./kill.sh
wait
