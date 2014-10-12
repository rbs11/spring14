package common;

public abstract class Job {
  private int numTasks = 0;

  public abstract void config(); //config function will be called in JobFactory when the job is created
  public abstract void task(int taskId); //task function will be executed by Worker

  //number of tasks will be set in config()
  protected void setNumTasks(int n) {
    numTasks = n;
  }

  public int getNumTasks() {
    return numTasks;
  }
}
