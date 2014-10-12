package client;

import java.io.*;
import java.net.*;

import common.*;

public class Client {
  public static void main(String[] args) throws Exception {

    String schedulerAddr = args[0];
    int schedulerPort = Integer.parseInt(args[1]);
    String fileName = args[2];
    String className = args[3];

    long time0, time1, time2, time3;
    time0 = time1 = time2 = time3 = 0;
    time0 = System.nanoTime()/1000000;

    //connect with scheduler
    Socket socket = new Socket( schedulerAddr, schedulerPort);
    DataInputStream dis = new DataInputStream(socket.getInputStream());
    DataOutputStream dos = new DataOutputStream(socket.getOutputStream());
    File file = new File(fileName);

    dos.writeInt(Opcode.new_job);
    dos.writeUTF(className);
    dos.writeLong(file.length());
    dos.flush();

    //get jobId from scheduler
    int jobId = dis.readInt();

    //submit the job file
    FileInputStream fis = new FileInputStream(file);
    int count;
    byte[] buf = new byte[65536];
    while((count = fis.read(buf)) > 0) {
      dos.write(buf, 0, count);
    }
    fis.close();
    dos.flush();
    time1 = System.nanoTime()/1000000;
    System.out.println("job submitted");

    //repeatedly process scheduler's feedback
    while(true) {
      int code = dis.readInt();

      if(code == Opcode.error) {
        System.out.println("job terminated");
        break;
      }

      if(code == Opcode.job_start) {
        time2 = System.nanoTime()/1000000;
        System.out.println("job started");
      }

      if(code == Opcode.job_finish) {
        time3 = System.nanoTime()/1000000;
        System.out.println("job finished");
        System.out.println("jobId="+jobId+" arrivalTime="+time0+" waitTime="+(time2-time1)+" execTime="+(time3-time2));
        break;
      }

      if(code == Opcode.job_print) {
        System.out.println(dis.readUTF());
      }
    }

    //disconnect
    dos.close();
    dis.close();
    socket.close();

  }
}
