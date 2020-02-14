11111111111111111111111111111111111111111111111111111111111111111111111111111111
11111111111111111111111111111111111111111111111111111111111111111111111111111111
## Project 1 REPORT
  In this program, we are asked to implement a simple shell to familiarize ourse
lves with **Process, File, Pipe**that was covered in the lecture. Also, there's 
a great portion of the usage of string parsing and data structure.And we are goi
ng to elaborate on how we implement each of the feature accordingly, since we im
plemented the program based on the suggested phase given on the promopt, we woul
d illustrate them orderly according to the phase number.

#### **Phase 1: Simple command** 
We are told to use the combination of fork + execv + wait to replace the command
system() to run the simple command. Here, we first do a *fork()* to create a 
child, and then let the children execute the command using the *execvp()*. And 
theparent would wait its child to die using *wait()* and then go to the next 
iteration. We test this part by using commands like **ls** and **date**.

#### **Phase 2: Handle the argument**
  In this phase, we start to parse the command line using *strtok()*, using the
whitespace as the deliminator. Then we store the cmd into a commandlist(array 
of string) that can be passed as the argument in execvp() and the last element 
of command list is a null character. We test this part by using commands like 
**ls -l** and **echo helloworld**.

#### **Phase 3: Handle the built-in command**
  For this phase, we implement *pwd()* and *cd()* by ourselves rather than run 
an external program. For pwd(), we use the function *getcwd()*, and we print 
the output directly to stdout. For cd(), we use the function *chdir()* and if we 
can't launch it because the designated direction doesn't exist, it will print 
error to stderr.

#### **Phase 4: Output redirection**
  The first thing we do in this phase is to use *strtok()*, using the 
metacharacter as the deliminator if any. Then we parse the command before the 
output file like we do in the previous phase. We manipulate the file descriptor
before we execute the command. We initialzie a file descriptor fd1, and then 
open the output file. Then, we use *dup2()* to duplicate fd1 into STDOUT. By 
doing this, stdout no longer prints to the terminal but to the output file. 
After this, whatever the output derived by the command would be redirected to 
the output file.

#### **Phase 5: Pipeline commands**
  In this Phase, we create all the pipes we need for the data transportation 
first and then use them sequentially in the loop of piping multiple commands. 
We make every execution in the children process and close all the pipe files 
before every execution. After doing this, the loop switch to the next command 
from pipelist and keep running untill the last command being executed. The only 
two exceptions are happen at the first command and the last command. Instead of 
writing or reading results into pipes, the first command should read from stdin
and the last command should write into stdout.

#### **Phase 6.1: Standard error redirection**
  Like the previous phase, we use *strtok()*, using the ">&" or "|&" as the 
deliminator if any. In this phase, it's a tweak of what we do in phase 4. But 
this time, we use *dup2()* to duplicate fd1 into STDERR. Whatever error would 
be print to terminal now is redirected into the output file.

#### **Phase 6.2: Directory Stack**
  We create a data structure called stack as well as the helper function push 
and pop to help implement this feature. For *pushd*, we use the function 
*chdir* to run cd and if there's no such directory, an error would be printed. 
Other than that, we also push the directory into the stack by using push. For 
*popd*, we use chdir("..") directly togo back to the latest directory pushed 
onto the stack. If there's nothing in the stack, an error would be printed.
Lastly, for the *dirs*，we first get the current directory by getcwd and then 
store it into a string. Then, we run a loop that would truncate the string by 
the top string of the stack and print it during each iteration.
