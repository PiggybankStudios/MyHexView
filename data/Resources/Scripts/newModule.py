
import os, sys, playground


class ButtonCuratorPlugin(playground.PlaygroundPlugin):
#
	def ButtonPressed(self, button):
	#
		if (button == "Enter"):
		#
			print("I love %s!!!!" % (button))
		#
		else:
		#
			print("I HATE %s!" % (button))
		#
	#
#

