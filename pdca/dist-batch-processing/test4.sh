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

w=$(jps -l | grep "worker" | cut -d" " -f1)

(java -jar worker.jar localhost 51580 51585) &
(java -jar worker.jar localhost 51580 51586) &
(java -jar worker.jar localhost 51580 51587) &
(java -jar worker.jar localhost 51580 51588) &

sleep 1
(./time.sh Job_256_250 | grep _info; sleep 1; ./kill.sh) &

sleep 4
for i in $w
do
  kill -kill $i
done

wait
