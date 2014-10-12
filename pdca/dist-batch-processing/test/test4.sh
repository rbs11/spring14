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

w=$(jps -l | grep "worker" | cut -d" " -f1)

(java -jar worker.jar localhost 51000 51005) &
(java -jar worker.jar localhost 51000 51006) &
(java -jar worker.jar localhost 51000 51007) &
(java -jar worker.jar localhost 51000 51008) &

sleep 1
(./time.sh Job_256_250 | grep _info; sleep 1; ./kill.sh) &

sleep 4
for i in $w
do
  kill -kill $i
done

wait
