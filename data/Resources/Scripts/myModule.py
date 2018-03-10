
import os, re, sys, random, playground

print("We are being loaded! Everybody line up for roll call!!")

# class ClassToBeDeleted():
# #
# 	def CrappyFunction():
# 	#
# 		print("This class needs to get deleted!")
# 	#
# #

class TestPlugin():
#
	localVar = 42
	
	def PrintLocalVar(self):
	#
		print("localVar = %u" % (self.localVar))
		self.localVar *= 42
	#
	
	def PluginLoaded(self):
	#
		print("Plugin loaded!")
	#
	
	# def ButtonPressed(self, button):
	# #
	# 	print("Button pressed: %s" % (button))
	# #
	
	def MousePressed(self, mouseX, mouseY):
	#
		print("Mouse pressed: (%d, %d)" % (mouseX, mouseY))
	#
#

class AwesomePlugin(playground.PlaygroundPlugin):
#
	def __init__(self):
	#
		print("By initializing AwesomePlugin you have consented to the terms and conditions of my harassment protocol!")
	#
	
	def Run(self):
	#
		print("Running TestPlugin")
		pg.SetBackgroundColor(10)
	#
	
	def PluginLoaded(self):
	#
		print("Please don't click on anything.")
	#
	
	def MousePressed(self, mouseX, mouseY):
	#
		r = random.randint(0, 3)
		if (r == 0): print("Why'd you click there?")
		elif (r == 1): print("Stop clicking on that.")
		elif (r == 2): print("What did you expect was going to happen?")
		elif (r == 3): print("That's not important. Don't click that.")
		else: print("I don't know what's happening...")
	#
#
