package jobs;

import common.*;

//Hello World
public class Job_16_4000 extends Job {

  @Override
  public void config() {
    setNumTasks(16); //set the number of tasks
  }

  @Override
  public void task(int tId) {
    //System.out.println("task"+tId+": Hello World"); //this string will be printed out from worker instead of client
    try{
      Thread.sleep(4000);
    } catch(Exception e) {
      e.printStackTrace();
    }

    if(tId%2==1)
      System.out.println("_info prog t"+tId+" finished");
  }
}
