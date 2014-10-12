package scheduler;

import java.io.*;
import java.net.*;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Iterator;

import common.*;

public class Scheduler {
	int schedulerPort;
	Cluster cluster;
	Object lock1;
	JobDescriptor jobDescriptor;
	LinkedList<JobInfo> JobQueue;
	int jobIdNext;
	Socket socketx;

	Scheduler(int p) {
		schedulerPort = p;
		cluster = new Cluster();
		jobDescriptor = new JobDescriptor();
		jobIdNext = 1;
		lock1 = new Object();
	}

	public static void main(String[] args) {
		Scheduler scheduler = new Scheduler(Integer.parseInt(args[0]));
		scheduler.run();
	}

	/*
	 * This runnable is implemented such that, each task spawns its own thread
	 * and waits for a free worker in the worker pool. It is oblivious of its
	 * parent job or other running jobs and tasks
	 */
	public class DispatchTask implements Runnable {
		int jobId;
		int flag = 0;
		int taskId = 0;
		DataInputStream dis;
		DataOutputStream dos;
		String className;
		int numTasks = 1;

		public DispatchTask(int jobIdarg, DataInputStream disarg,
				DataOutputStream dosarg, int taskID_arg, String className_arg) {
			jobId = jobIdarg;
			flag = 0;
			dis = disarg;
			dos = dosarg;
			className = className_arg;
			taskId = taskID_arg;
		}

		public void run() {
			/* This flag is used to handle */
			while (flag == 0) {
				try {
					// get a free worker
					WorkerNode n = cluster.getFreeWorkerNode();

					// assign the tasks to the worker
					Socket workerSocket = new Socket(n.addr, n.port);
					DataInputStream wis = new DataInputStream(
							workerSocket.getInputStream());
					DataOutputStream wos = new DataOutputStream(
							workerSocket.getOutputStream());

					wos.writeInt(Opcode.new_tasks);
					wos.writeInt(jobId);
					wos.writeUTF(className);
					wos.writeInt(taskId);
					wos.writeInt(numTasks);
					wos.flush();
					/*
					 * Since multiple threads access this section which requires
					 * syncronous behaviour for socket communication with the
					 * client.
					 */
					synchronized (lock1) {
						// repeatedly process the worker's feedback

						if (wis.readInt() == Opcode.task_finish) {
							int taskID = 0;
							taskID = wis.readInt();
							dos.writeInt(Opcode.job_print);
							dos.writeUTF("task " + taskID
									+ " finished on worker " + n.id);
							flag = 1;
							// System.out.println("Sched: Worker "+n.id+" now free");
							dos.flush();
						}
					}
					// disconnect and free the worker
					wis.close();
					wos.close();
					workerSocket.close();
					cluster.addFreeWorkerNode(n);
				} catch (Exception e) {
					// e.printStackTrace();
					System.out.println("Worker killed, assigning task to "
							+ "next free worker");
				}
			}

		}
	}

	/*
	 * This is the core scheduler thread which picks up jobs from the job queue
	 * in round robin fashion. For every job that's picked up, one task is
	 * dispatched on a free worker.
	 */
	public class Core implements Runnable {
		Socket socket;
		int MaxJobs, MaxTasks;

		public Core() {
			MaxJobs = 1000;
			MaxTasks = 100000;
		}

		public void run() {
			try {
				while (JobQueue.isEmpty()) {
					Thread.sleep(100);
				}
				Thread[][] tt = new Thread[MaxJobs][MaxTasks];

				while (true) {
					while (JobQueue.isEmpty()
							|| (cluster.GetNumfreeNodes() == 0)) {
						Thread.sleep(10);
					}// should sleep
					Iterator<JobInfo> itr = JobQueue.iterator();

					while (itr.hasNext()) {

						JobInfo job = new JobInfo();
						job = itr.next();

						int jobId = job.jobId;
						int numTasks = job.numtasks;
						int currentTaskId = job.nextTaskId;

						socket = job.socket;
						DataInputStream dis = new DataInputStream(
								socket.getInputStream());
						DataOutputStream dos = new DataOutputStream(
								socket.getOutputStream());

						// System.out.println("Reading Job entry with ID "+jobId+" Cur Task "+currentTaskId+"<"+numTasks+"> FreeNodes "+cluster.GetNumfreeNodes());
						if (numTasks > currentTaskId) {
							/*
							 * Create an array of threads to keep track of each
							 * thread's state
							 */
							if (currentTaskId == 0) {
								// notify the client
								dos.writeInt(Opcode.job_start);
								dos.flush();
							}
							tt[jobId][currentTaskId] = new Thread(
									new DispatchTask(jobId, dis, dos,
											currentTaskId, job.ClassName));
							tt[jobId][currentTaskId].start();
							currentTaskId++;
							job.nextTaskId = currentTaskId;
						} else { /*
								 * Wait for completion if all tasks have been
								 * submitted
								 */
							/* Wait for all threads to complete */
							currentTaskId = 0;
							while (currentTaskId < numTasks) {
								tt[jobId][currentTaskId].join();
								// System.out.println("Completion of Job[Task ID] ="+jobId+"["+currentTaskId+"");
								currentTaskId++;
							}
							// notify the client
							// System.out.println("Job "+jobId+" completed");
							jobDescriptor.removeJobfromQueue(job.ClassName,
									jobId);
							dos.writeInt(Opcode.job_finish);
							/* Close socket */
							dis.close();
							dos.close();
							// socket.close();
						}
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public void run() {
		try {
			// create a ServerSocket listening at specified port
			ServerSocket serverSocket = new ServerSocket(schedulerPort);
			int initDone = 0;

			while (true) {
				// accept connection from worker or client

				Socket socket = serverSocket.accept();

				DataInputStream dis = new DataInputStream(
						socket.getInputStream());
				DataOutputStream dos = new DataOutputStream(
						socket.getOutputStream());

				int code = dis.readInt();

				// a connection from worker reporting itself
				if (code == Opcode.new_worker) {
					// include the worker into the cluster
					WorkerNode n = cluster.createWorkerNode(dis.readUTF(),
							dis.readInt());
					if (n == null) {
						dos.writeInt(Opcode.error);
					} else {
						dos.writeInt(Opcode.success);
						dos.writeInt(n.id);
						System.out.println("Worker " + n.id + " " + n.addr
								+ " " + n.port + " created");
					}
					dos.flush();
					dis.close();
					dos.close();
				}

				// a connection from client submitting a job
				if (code == Opcode.new_job) {

					if (initDone == 0) {
						Runnable core = new Core();
						new Thread(core).start();
						initDone = 1;
					}
					String className = dis.readUTF();
					long len = dis.readLong();

					int jobId = jobIdNext++;

					// send out the jobId

					dos.writeInt(jobId);
					dos.flush();

					// receive the job file and store it to the shared
					// filesystem

					String fileName = new String("fs/." + jobId + ".jar");
					FileOutputStream fos = new FileOutputStream(fileName);
					int count;
					byte[] buf = new byte[65536];
					while (len > 0) {
						count = dis.read(buf);
						if (count > 0) {
							fos.write(buf, 0, count);
							len -= count;
						}
					}
					fos.flush();
					fos.close();

					// get the tasks
					int taskIdStart = 0;
					int numTasks = JobFactory.getJob(fileName, className)
							.getNumTasks();

					jobDescriptor.createNewJob(className, jobId, numTasks,
							taskIdStart, socket);

					// System.out.println("Added New Job to queue, Name "+className+" Num "+JobQueue.size());

					dos.flush();
				}
			}

		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	// the data structure for a cluster of worker nodes
	class Cluster {
		ArrayList<WorkerNode> workers; // all the workers
		LinkedList<WorkerNode> freeWorkers; // the free workers

		Cluster() {
			workers = new ArrayList<WorkerNode>();
			freeWorkers = new LinkedList<WorkerNode>();
		}

		WorkerNode createWorkerNode(String addr, int port) {
			WorkerNode n = null;
			Iterator<WorkerNode> itr = workers.iterator();
			WorkerNode tmp = null;
			/*
			 * First need to check if this worker is already present before
			 * allocating a new worker. This prevents duplicates
			 */
			synchronized (workers) {
				while (itr.hasNext()) {
					tmp = itr.next();
					if (tmp.port == port) {
						System.out.println("Restarting Worker " + tmp.id);
						addFreeWorkerNode(tmp);
						return tmp;
					}
				}
				n = new WorkerNode(workers.size(), addr, port);
				workers.add(n);
			}
			addFreeWorkerNode(n);

			return n;
		}

		WorkerNode getFreeWorkerNode() {
			WorkerNode n = null;

			try {
				synchronized (freeWorkers) {
					while (freeWorkers.size() == 0) {
						freeWorkers.wait();
					}
					n = freeWorkers.remove();
				}
				n.status = 2;
			} catch (Exception e) {
				e.printStackTrace();
			}

			return n;
		}

		int GetNumfreeNodes() {
			return freeWorkers.size();
		}

		void addFreeWorkerNode(WorkerNode n) {
			n.status = 1;
			Iterator<WorkerNode> itr = freeWorkers.iterator();
			WorkerNode tmp = null;
			synchronized (freeWorkers) {
				while (itr.hasNext()) {
					tmp = itr.next();
					if (tmp.port == n.port) {
						System.out.println("Already in free list " + tmp.id);
						return;
					}
				}
				freeWorkers.add(n);
				freeWorkers.notifyAll();
			}
		}
	}

	// the data structure of a worker node
	class WorkerNode {
		int id;
		String addr;
		int port;
		int status; // WorkerNode status: 0-sleep, 1-free, 2-busy, 4-failed

		WorkerNode(int i, String a, int p) {
			id = i;
			addr = a;
			port = p;
			status = 0;
		}
	}

	class JobDescriptor {

		JobDescriptor() {
			JobQueue = new LinkedList<JobInfo>();
		}

		void createNewJob(String className, int jobId, int numTasks,
				int currentTaskId, Socket socket) {
			JobInfo job = null;

			synchronized (JobQueue) {
				job = new JobInfo();
				job.ClassName = className;
				job.jobId = jobId;
				job.numtasks = numTasks;
				job.nextTaskId = currentTaskId;
				job.socket = socket;
				JobQueue.add(job);
			}
		}

		void removeJobfromQueue(String className, int jobId) {
			Iterator<JobInfo> itr = JobQueue.iterator();
			JobInfo job = null;
			synchronized (JobQueue) {
				// job = new JobInfo();
				while (itr.hasNext()) {
					job = itr.next();
					if (job.jobId == jobId && job.ClassName == className) {
						if (!JobQueue.contains(job))
							System.out
									.println("Something fishy, Job should be present ");
						else {
							JobQueue.remove(job);
							// System.out.println("Job with ID "+ jobId
							// +" deleted. Remaining "+JobQueue.size());
						}
					}
				}
			}
		}
	}

	class JobInfo {
		int jobId;
		String ClassName;
		int numtasks;
		int nextTaskId;
		Socket socket;
	}

}
