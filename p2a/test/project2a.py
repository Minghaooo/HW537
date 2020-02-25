import os, subprocess, shutil

import toolspath

from testing import Test, BuildTest

shell_prompt = "smash> "
call_shell = "smash <"
err_message = "An error has occurred\n"

def readall(filename):
  f = open(filename, 'r')
  # modified by yien
  s = f.read().strip()
  f.close()
  return s

class ShellBuildTest(Test):
  targets = ['smash']
  point_value = 5
  name = 'build'
  status = 0
  description = 'compile smash.c'
  timeout = 10
  
  def run(self):
    command = ['gcc','smash.c','-o','smash']

    self.run_util(command)
    self.done()
   # self.clean(['smash', '*.o'])
   # if not self.make(self.targets):
   #   self.run_util(['gcc', 'smash.c', '-o', 'smash'])
   # self.done()

class ShellTest(Test):

  def run(self, command = None, stdout = None, stderr = None, 
          addl_args = []):
    in_path = self.test_path + '/' + self.name + '/in'
    if command == None:
      command = ['./smash']

    out_path = self.test_path + '/' + self.name + '/out'
    err_path = self.test_path + '/' + self.name + '/err'
    self.command = call_shell + in_path + \
        "\n and check out the test folder\n " + self.test_path \
        + '/' + self.name + \
        "\n to compare your output with reference outputs. "

    # process the input file with input redirection
    stdin = readall(in_path)

    if stdout == None:
      stdout = readall(out_path)
    if stderr == None:
      stderr = readall(err_path)

    self.runexe(command, status=self.status, 
                stdin=stdin, stderr=stderr, stdout=stdout)
    self.done()



######################### Built-in Commands #########################


class CD(ShellTest):
  name = 'cd'
  description = 'cd with argument, and cd without argument'
  timeout = 10
  status = 0
  point_value = 10
  def run(self):
    out1 = subprocess.Popen(['/bin/ls', '/u'], 
                             stdout=subprocess.PIPE).communicate()[0]
    #out2 = subprocess.Popen(['/bin/ls', os.environ['HOME']], 
    #                        stdout=subprocess.PIPE).communicate()[0]
    super(CD, self).run(stdout = shell_prompt + 
                        shell_prompt + out1 + 
                        shell_prompt + 
                        shell_prompt + out1 +
                        'smash>')
class BadCD(ShellTest):
  name = 'badcd'
  description = 'cd to a bad directory'
  timeout = 10
  status = 0
  point_value = 5

class Exec(ShellTest):
  name = 'exec'
  description = 'path should be /bin'
  timeout = 10
  status = 0
  point_value = 5

class BadExec(ShellTest):
  name = 'badexec'
  description = 'run programs that do not exist in /bin'
  timeout = 10
  status = 0
  point_value = 5

class Path(ShellTest):
  name = 'path'
  description = 'run path and a program that exists in a given path'
  timeout = 10
  status = 0
  point_value = 5

class Path2(ShellTest):
  name = 'path2'
  description = 'run path with a long pathname and many different paths'
  timeout = 10
  status = 0
  point_value = 5


######################### Formatting ###########################

class Line(ShellTest):
  name = 'line'
  description = 'line with maximum allowed length'
  timeout = 10
  status = 0
  point_value = 5

class BadLine(ShellTest):
  name = 'badline'
  description = 'a line that is too long'
  timeout = 10
  status = 0
  point_value = 5


class WhiteSpace(ShellTest):
  name = 'whitespace'
  description = 'leading and trailing whitespace, full whitespaces or empty string'
  timeout = 10
  status = 0
  point_value = 5


######################### Redirection #############################

class Rdr(ShellTest):
  name = 'rdr'
  description = 'simple redirection with no standard error'
  timeout = 10
  status = 0
  point_value = 10
  
  def run(self):
    super(Rdr, self).run()
    #print('path: %s ' % self.project_path)
    if os.path.isfile(self.project_path + "/outputfile") == False:
      self.fail("missing redirected standard output outputfile")
    out = readall(self.project_path + "/outputfile")
    ref_out = readall(self.test_path + '/' + self.name + '/outputfile')
    if (out != ref_out): self.fail("standard output does not match redirected output")

    
    self.done()

class Rdr2(ShellTest):
  name = 'rdr2'
  description = 'simple redirection with standard error'
  timeout = 10
  status = 0
  point_value = 5
  
  def run(self):
    super(Rdr2, self).run()
    #print('path: %s ' % self.project_path)
    if os.path.isfile(self.project_path + "/outputfile") == False:
      self.fail("missing redirected standard output outputfile")
    out = readall(self.project_path + "/outputfile")
    ref_out = readall(self.test_path + '/' + self.name + '/outputfile')
    #print('out: %s' % out)
    #print('ref_out: %s' % ref_out)
    if (out != ref_out): self.fail("standard output does not match redirected output")

     
    self.done()

#############SP2020#################
class Par(ShellTest):
    name = 'par'
    description = 'running commands in parallel'
    timeout = 10
    status = 0
    point_value = 5

class Mul(ShellTest):
    name = 'mul'
    description = 'running multiple commands at once'
    timeout = 10
    status = 0
    point_value = 5

#=========================================================================

all_tests = [ 
  # Built-in Command
   CD,
   BadCD,
   Exec,
   BadExec,
   Path,
   Path2,

  # # Formatting
   Line,
   WhiteSpace,

  # # Redirection
   Rdr,
   Rdr2,

  # # sp2020
   Par,
   Mul,
  ]

build_test = ShellBuildTest

from testing.runtests import main
main(build_test, all_tests)
