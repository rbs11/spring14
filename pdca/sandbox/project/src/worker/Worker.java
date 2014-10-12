package worker;

import java.io.*;
import java.net.*;

import common.*;

public class Worker {

  String schedulerAddr;
  int schedulerPort;
  String workerAddr;
  int workerPort;

  public static void main(String[] args) {
    Worker worker = new Worker( args[0], Integer.parseInt(args[1]), Integer.parseInt(args[2]));
    worker.run();
  }

  Worker(String sAddr, int sPort, int wPort) {
    schedulerAddr = sAddr;
    schedulerPort = sPort;
    workerAddr = "localhost";
    workerPort = wPort;
  }

  public void run() {
    ServerSocket serverSocket;
    Socket socket;
    DataInputStream dis;
    DataOutputStream dos;
    int workerId;

    try{
      //create a ServerSocket listening at the specified port
      serverSocket = new ServerSocket(workerPort);

      //connect to scheduler
      socket = new Socket(schedulerAddr, schedulerPort);
      dis = new DataInputStream(socket.getInputStream());
      dos = new DataOutputStream(socket.getOutputStream());
   
      //report itself to the scheduler
      dos.writeInt(Opcode.new_worker);
      dos.writeUTF(workerAddr);
      dos.writeInt(workerPort);
      dos.flush();

      if(dis.readInt() == Opcode.error){
        throw new Exception("scheduler error");
      }
      workerId = dis.readInt();

      dis.close();
      dos.close();
      socket.close();

      System.out.println("This is worker "+workerId);
      
      //repeatedly process scheduler's task assignment
      while(true) {
        //accept an connection from scheduler
        socket = serverSocket.accept();
        dis = new DataInputStream(socket.getInputStream());
        dos = new DataOutputStream(socket.getOutputStream());

        if(dis.readInt() != Opcode.new_tasks)
          throw new Exception("worker error");

        //get jobId, fileName, className, and the assigned task ids
        int jobId = dis.readInt();
        String fileName = new String("fs/."+jobId+".jar");
        String className = dis.readUTF();
        int taskIdStart = dis.readInt();
        int numTasks = dis.readInt();

        //read the job file from shared file system
        Job job = JobFactory.getJob(fileName, className);

        //execute the assigned tasks
        for(int taskId=taskIdStart; taskId<taskIdStart+numTasks; taskId++){
          job.task(taskId);
          //report to scheduler once a task is finished
          dos.writeInt(Opcode.task_finish);
          dos.writeInt(taskId);
          dos.flush();
        }

        //disconnect
        dos.writeInt(Opcode.worker_finish);
        dos.flush();
        dis.close();
        dos.close();
        socket.close();
      }
    } catch(Exception e) {
      e.printStackTrace();
    }
  }

}
