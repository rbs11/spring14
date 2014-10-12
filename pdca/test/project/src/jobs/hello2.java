package jobs;

import common.*;
import java.io.*;
import java.net.*;
import java.util.concurrent.*;;
//Hello World
public class hello2 extends Job {

  @Override
  public void config() {
    setNumTasks(500); //set the number of tasks
  }

  @Override
  public void task(int tId) {
    System.out.println("task"+tId+": _Hello World_"); //this string will be printed out from worker instead of client
    try{
      Thread.sleep(100);
    } catch(Exception e) {
      e.printStackTrace();
    }
  }
}
