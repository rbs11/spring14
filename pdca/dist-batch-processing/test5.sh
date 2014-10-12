#!/bin/bash

sleep 1
./kill.sh
sleep 1
(java -jar scheduler.jar 51580) &
sleep 1
(java -jar worker.jar localhost 51580 51581) &
(java -jar worker.jar localhost 51580 51582) &
(java -jar worker.jar localhost 51580 51583) &
(java -jar worker.jar localhost 51580 51584) &
sleep 1

(./time.sh Job_256_250 | grep _info) &
j=$!
(./time.sh Job_16_4000 | grep _info) &
j=$(echo $j $!)

wait $j

./kill.sh
wait
