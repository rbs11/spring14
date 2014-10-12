package common;

public class Opcode {
  //These opcodes are used in the communication among Client Scheduler and Worker
  public static final int error = -1;
  public static final int success = 1; 

  public static final int new_worker = 10;
  public static final int new_job = 20;
  public static final int new_tasks = 30;

  public static final int worker_finish = 11;

  public static final int job_start = 21;
  public static final int job_finish = 22;
  public static final int job_print = 23;

  public static final int task_finish = 31;
}
