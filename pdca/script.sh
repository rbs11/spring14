java -jar client.jar localhost 51000 jobs.jar jobs.Hello >hello.log &
java -jar client.jar localhost 51000 jobs.jar jobs.Mvm >matrix.log &
java -jar client.jar localhost 51000 jobs.jar jobs.hello2 >hello2.log &
java -jar client.jar localhost 51000 jobs.jar jobs.mvm2 >mvm2.log &
