
import os, re, sys

print("Python Module Loaded!")

def SomeFunction(someNumber):
#
	print("You called SomeFunction(%u)! :D" % (someNumber))
	
	return 45*someNumber
#

class SomeClass():
#
	localVar = 42
	
	def PrintLocalVar(self):
	#
		print("localVar = %u" % (self.localVar))
		self.localVar *= 42
	#
#
