#!/bin/bash
sleep 1
./kill.sh
sleep 1
(java -jar scheduler.jar 51580) &
sleep 1
(java -jar worker.jar localhost 51580 51581) &
sleep 1
./time.sh Job_16_4000 | grep _info

sleep 1
./kill.sh

wait
