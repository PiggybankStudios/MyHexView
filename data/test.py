import Resources.Scripts.myModule

print("Running Test.py!")

dictionary = Resources.Scripts.myModule.__dict__
print("Dictionary has %u items" % (len(dictionary)))

for item in dictionary:
#
	print("Item: " + str(item))
#

print("Item[0] = " + str(dictionary[0]))
