package common;

import java.net.*;

public class JobFactory {

  //getJob function will create a job from a job file
  public static Job getJob(String file, String name) throws Exception {

    URLClassLoader loader = new URLClassLoader(new URL[] {new URL("file:"+file)});
    Job job = (Job) Class.forName(name, true, loader).newInstance(); //load the job
    job.config(); //config the job
    return job;
  }
}
