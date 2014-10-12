package jobs;

import java.io.*;
import common.*;

//Matrix Vector Multiplication
public class Mvm extends Job {

  @Override
  public void config() {
    setNumTasks(4);
  }

  @Override
  public void task(int tId) {
    try {
      int nRow, nCol;
      int[][] mat;
      int[] vec;
      int[] product;

      File file;
      BufferedReader reader;
      PrintWriter printWriter;
      String line;
      String[] elements;

      //read the shared matrix file
      file = new File("fs/example/matrix.txt");
      reader =  new BufferedReader(new FileReader(file));
      line = reader.readLine(); //the first line is dimension
      elements = line.split(" ");
      nRow = Integer.parseInt(elements[0]);
      nCol = Integer.parseInt(elements[1]);

      mat = new int[nRow][nCol];
      for(int i=0; i<nRow; i++) {
        line = reader.readLine();
        elements = line.split(" ");
        for(int j=0; j<nCol; j++) {
          mat[i][j] = Integer.parseInt(elements[j]);
        }
      }
      reader.close();

      //read specific vector file based on task ID
      file = new File("fs/example/vector"+tId+".txt");
      reader =  new BufferedReader(new FileReader(file));
      line = reader.readLine();
      elements = line.split(" ");
      if(Integer.parseInt(elements[0]) != nCol || Integer.parseInt(elements[1]) != 1)
        throw new Exception("dimension error");

      vec = new int[nCol];
      for(int i=0; i<nCol; i++) {
        line = reader.readLine();
        vec[i] = Integer.parseInt(line);
      }
      reader.close();

      //calculate the produc
      product = new int[nRow];
      for(int i=0; i<nRow; i++) {
        product[i] = 0;
        for(int j=0; j<nCol; j++) {
          product[i] += mat[i][j] * vec[j];
        }
      }
     
      //write output file
      file = new File("fs/example/out"+tId+".txt");
      printWriter = new PrintWriter(file);

      printWriter.println(nRow+" 1");
      for(int i=0; i<nRow; i++) {
        printWriter.println(product[i]);
      }
      printWriter.close();

    } catch(Exception e) {
      e.printStackTrace();
    }
  }
}
