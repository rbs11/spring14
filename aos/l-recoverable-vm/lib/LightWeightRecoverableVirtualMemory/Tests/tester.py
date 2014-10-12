import os
import glob

# compilation
for files in  glob.glob("*.c"):
	outputF = str(files) + ".o"
	os.system("gcc -o " + outputF +" " + str(files) +" librvm.a")

#run
for files in  glob.glob("*.o"):
	 os.system("./" + str(files))
