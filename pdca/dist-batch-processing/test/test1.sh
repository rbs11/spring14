#!/bin/bash
sleep 1
./kill.sh
sleep 1
(java -jar scheduler.jar 51000) &
sleep 1
(java -jar worker.jar localhost 51000 51001) &
sleep 1
./time.sh Job_16_4000 | grep _info

sleep 1
./kill.sh

wait
