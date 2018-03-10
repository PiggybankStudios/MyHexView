
import SomeOtherName as pg

class PlaygroundPlugin():
#
	def __init__(self):
	#
		print("PlaygroundPlugin __init__")
		self.number = 10
	#
	
	def PluginLoaded(self):
	#
		pass # print("PlaygroundPlugin PluginLoaded")
	#
	
	def ButtonPressed(self, button):
	#
		pass # print("PlaygroundPlugin ButtonPressed(%s)" % (button))
	#
	
	def MousePressed(self, mouseX, mouseY):
	#
		pass # print("PlaygroundPlugin MousePressed(%d, %d)" % (mouseX, mouseY))
	#
#