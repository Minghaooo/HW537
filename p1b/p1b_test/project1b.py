#! /bin/env python

import toolspath
from testing import Xv6Build, Xv6Test

class filecount1(Xv6Test):
   name = "test1"
   description = "call getfilenum() test test, value > 0"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1

class filecount2(Xv6Test):
   name = "test2"
   description = "call getfilenum() twice, sleep(50) between"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1

class filecount3(Xv6Test):
   name = "test3"
   description = "called getfilenum() with 5 opens in between"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   
class filecount4(Xv6Test):
   name = "test4"
   description = "called getfilenum() with max (13) opens in between"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
class filecount5(Xv6Test):
   name = "test5"
   description = "called getfilenum() with opens and closes"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
class filecount6(Xv6Test):
   name = "test6"
   description = "called getfilenum() for a forked process"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1

import toolspath
from testing.runtests import main
main(Xv6Build, [filecount1, filecount2, filecount3,filecount4,filecount5,filecount6])
