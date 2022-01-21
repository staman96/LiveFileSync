# Live File Syncronization

   This exercise's purpose was to make multiple processes using syscalls(fork/exec), make them communicate through pipes and use low level I/O. Also, bash scripts were used. These tools were used to create a program that syncs files between different directories. Different processes are used for each file that needs to be copied.

Manolas Stamatios  
DIT System Programming 2019 Project 2

Execution command for each client:

      ./mirror_client -n id -c common_dir -i input_dir -m mirror_dir -b buffer_size -l log_file
      
## How each client works:

   Each client uses its input directory to contribute data to all other's mirror directories. In order for the program to work at least 2 clients must be executed simultaneously and in the end each client must have all the files from every client:
   
./mirror_client -n id1 -c common_dir -i input_dir1 -m mirror_dir1 -b buffer_size -l log_file1
./mirror_client -n id2 -c common_dir -i input_dir2 -m mirror_dir2 -b buffer_size -l log_file2

The common_dir is used for the processes's communication (pipes and client IDs) and must the same for every client.

### General:

   The working principal is that each client waits until he finds another client process running. Then, each client process forks into an intermediate controlling process; and 2 children processes, one for sending data and one for receiving data. The log files contain all files exchanged with each client.




  
