/*
File:   app_pythonPlugin.cpp
Author: Taylor Robbins
Date:   03\03\2018
Description: 
	** Holds the functions that help us load, trigger, and keep track of python extensions 
*/

void PythonCheckError_(const char* fileName, int lineNumber, const char* functionName)
{
	PyObject* pyError = PyErr_Occurred();
	
	if (pyError != nullptr)
	{
		DEBUG_PrintLine("A python error occurred in %s:%d function %s!", fileName, lineNumber, functionName);
		PyErr_Print();
		PyErr_Clear();
	}
}

#define PythonCheckError() PythonCheckError_(__FILE__, __LINE__, __FUNCTION__)

//NOTE: This function is probably quite incomplete but it works for our purposes for now
char* GetModuleNameFromPath(MemoryArena_t* arenaPntr, const char* filePath)
{
	u32 pathLength = (u32)strlen(filePath);
	char* result = ArenaString(arenaPntr, filePath, pathLength);
	StrReplaceInPlace(NtStr(result), "/", ".", 1);
	char* extension = strstr(result, ".py");
	if (extension != nullptr)
	{
		extension[0] = '\0';
	}
	return result;
}

void RemoveModuleFromSysModules(const char* moduleName)
{
	DEBUG_PrintLine("Removing module \"%s\"", moduleName);
	
	PyObject* pyModuleDictionary = PyImport_GetModuleDict();
	if (pyModuleDictionary != nullptr)
	{
		PyObject* pyDictKeys = PyDict_Keys(pyModuleDictionary);
		Py_ssize_t pyNumDictKeys = PyObject_Length(pyDictKeys);
		// DEBUG_PrintLine("Dictionary has %u items", pyNumDictKeys);
		
		for (Py_ssize_t dIndex = 0; dIndex < pyNumDictKeys; dIndex++)
		{
			PyObject* pyIndex = PyLong_FromLong((long)dIndex);
			PyObject* pyDictKey = PyObject_GetItem(pyDictKeys, pyIndex);
			
			if (pyDictKey != nullptr)
			{
				PyObject* pyDictKeyStr = PyObject_Str(pyDictKey);
				if (pyDictKeyStr != nullptr && PyUnicode_Check(pyDictKeyStr))
				{
					char* pyDictKeyStrUtf8 = PyUnicode_AsUTF8(pyDictKeyStr);
					// DEBUG_PrintLine("Module[%u] = %s", dIndex, pyDictKeyStrUtf8);
					
					if (strcmp(pyDictKeyStrUtf8, moduleName) == 0)
					{
						DEBUG_WriteLine("Found the module!");
						
						if (PyDict_DelItem(pyModuleDictionary, pyDictKey) == 0)
						{
							DEBUG_WriteLine("Remove Succeeded!");
						}
						else
						{
							DEBUG_WriteLine("Remove Failed!");
						}
					}
				}
				else { DEBUG_PrintLine("Couldn't get Key[%u] string", dIndex); }
				Py_XDECREF(pyDictKeyStr);
			}
			else { DEBUG_PrintLine("Couldn't get Key[%u]", dIndex); }
			Py_XDECREF(pyDictKey);
		}
		
		Py_XDECREF(pyDictKeys);
	}
	else
	{
		DEBUG_WriteLine("Couldn't open module dictionary");
	}
	
	PythonCheckError();
}

void LoadPlaygroundPythonModule()
{
	if (app->pyPlaygroundModule != nullptr) { Py_XDECREF(app->pyPlaygroundModule); app->pyPlaygroundModule = nullptr; }
	if (app->pyPlaygroundClass != nullptr) { Py_XDECREF(app->pyPlaygroundClass); app->pyPlaygroundClass = nullptr; }
	
	const char* moduleName = "playground";
	DEBUG_WriteLine("Attempting to load module playground module");
	app->pyPlaygroundModule = PyImport_ImportModule(moduleName);
	
	if (app->pyPlaygroundModule != nullptr)
	{
		DEBUG_WriteLine("Loaded Playground module successfully");
		
		if (PyObject_HasAttrString(app->pyPlaygroundModule, "PlaygroundPlugin"))
		{
			app->pyPlaygroundClass = PyObject_GetAttrString(app->pyPlaygroundModule, "PlaygroundPlugin");
		}
		
		if (app->pyPlaygroundClass != nullptr)
		{
			DEBUG_WriteLine("Found PlaygroundPlugin class");
		}
		else
		{
			DEBUG_WriteLine("Couldn't find PlaygroundPlugin class");
		}
	}
	else
	{
		DEBUG_WriteLine("Couldn't load playground module");
	}
}

void UnloadModule(PythonPluginModule_t* pluginModule)
{
	Assert(pluginModule != nullptr);
	
	DEBUG_PrintLine("Dereferencing module \"%s\"", pluginModule->moduleName);
	
	for (u32 pIndex = 0; pIndex < pluginModule->numPlugins; pIndex++)
	{
		PythonPlugin_t* plugin = &pluginModule->plugins[pIndex];
		DEBUG_PrintLine("Dereferencing plugin \"%s\"", plugin->className);
		
		for (u32 fIndex = 0; fIndex < ArrayCount(plugin->pyFunctions); fIndex++)
		{
			Py_XDECREF(plugin->pyFunctions[fIndex]);
		}
		Py_XDECREF(plugin->pyInstance);
		Py_XDECREF(plugin->pyType);
	}
	
	if (pluginModule->plugins != nullptr)
	{
		Assert(pluginModule->allocArena != nullptr);
		ArenaPop(pluginModule->allocArena, pluginModule->plugins);
	}
	
	Py_XDECREF(pluginModule->pyModule);
	
	RemoveModuleFromSysModules(pluginModule->moduleName);
	
	ClearPointer(pluginModule);
	PythonCheckError();
}

u32 FindModuleClasses(BoundedStrList_t* classNameList, PyObject* pyModule)
{
	Assert(pyModule != nullptr);
	u32 result = 0;
	
	// DEBUG_PrintLine("Looking for classes in loaded module %p", pyModule);
	
	PyObject* pyModuleDictionary = PyObject_GenericGetDict(pyModule, nullptr);
	if (pyModuleDictionary != nullptr)
	{
		// DEBUG_PrintLine("Got module dictionary %p", pyModuleDictionary);
		
		PyObject* pyDictKeys = PyDict_Keys(pyModuleDictionary);
		Py_ssize_t pyNumDictKeys = PyObject_Length(pyDictKeys);
		// DEBUG_PrintLine("Dictionary has %u items", pyNumDictKeys);
		
		for (Py_ssize_t dIndex = 0; dIndex < pyNumDictKeys; dIndex++)
		{
			PyObject* pyIndex = PyLong_FromLong((long)dIndex);
			PyObject* pyDictKey = PyObject_GetItem(pyDictKeys, pyIndex);
			
			if (pyDictKey != nullptr)
			{
				PyObject* pyDictKeyStr = PyObject_Str(pyDictKey);
				if (pyDictKeyStr != nullptr && PyUnicode_Check(pyDictKeyStr))
				{
					char* pyDictKeyStrUtf8 = PyUnicode_AsUTF8(pyDictKeyStr);
					// DEBUG_PrintLine("Dict[%u] = %s", dIndex, pyDictKeyStrUtf8);
					
					PyObject* pyClass = PyObject_GetAttrString(pyModule, pyDictKeyStrUtf8);
					if (pyClass != nullptr && PyType_Check(pyClass))
					{
						// DEBUG_PrintLine("Found class \"%s\"", pyDictKeyStrUtf8);
						BoundedStrListAdd(classNameList, pyDictKeyStrUtf8);
						result++;
					}
					else
					{
						// DEBUG_PrintLine("\"%s\" is not a class", pyDictKeyStrUtf8);
					}
					
					Py_XDECREF(pyClass);
				}
				else
				{
					DEBUG_PrintLine("Key[%u] is not a string", dIndex);
				}
				Py_XDECREF(pyDictKeyStr);
			}
			else
			{
				DEBUG_PrintLine("Couldn't get Key[%u]", dIndex);
			}
			
			Py_XDECREF(pyDictKey);
		}
		
		Py_XDECREF(pyDictKeys);
	}
	else
	{
		DEBUG_WriteLine("Couldn't get module dictionary");
	}
	Py_XDECREF(pyModuleDictionary);
	
	PythonCheckError();
	return result;
}

bool IsClassValidPlugin(PyObject* pyClass)
{
	if (pyClass == nullptr) { return false; }
	
	if (PyType_Check(pyClass))
	{
		if (app->pyPlaygroundClass == nullptr || PyObject_IsSubclass(pyClass, app->pyPlaygroundClass))
		{
			return true;
		}
		else
		{
			DEBUG_WriteLine("Not a subclass of PlaygroundPlugin");
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool LoadPluginFromClass(PythonPlugin_t* pluginPntr, const char* className, PyObject* pyClass)
{
	Assert(pluginPntr != nullptr);
	Assert(pyClass != nullptr);
	Assert(IsClassValidPlugin(pyClass));
	
	ClearPointer(pluginPntr);
	
	CopyStringIntoArray(pluginPntr->className, className);
	pluginPntr->pyType = (PyTypeObject*)pyClass;
	
	PyObject* pyCreateArgs = PyTuple_New(0);
	PyObject* pyCreateKeywords = PyTuple_New(0);
	pluginPntr->pyInstance = PyType_GenericNew(pluginPntr->pyType, pyCreateArgs, pyCreateKeywords);
	if (pluginPntr->pyInstance != nullptr)
	{
		DEBUG_PrintLine("Created %s instance", className);
		pluginPntr->loaded = true;
		
		if (PyObject_HasAttrString(pluginPntr->pyInstance, "__init__"))
		{
			PyObject* pyInitFunction = PyObject_GetAttrString(pluginPntr->pyInstance, "__init__");
			PythonCheckError();
			if (pyInitFunction != nullptr)
			{
				DEBUG_WriteLine("Calling __init__ function");
				PyObject_CallFunction(pyInitFunction, "");
			}
		}
		
		if (PyObject_HasAttrString(pluginPntr->pyInstance, "PluginLoaded"))
		{
			pluginPntr->pyFunc_PluginLoaded = PyObject_GetAttrString(pluginPntr->pyInstance, "PluginLoaded");
			PythonCheckError();
			if (pluginPntr->pyFunc_PluginLoaded != nullptr)
			{
				DEBUG_PrintLine("Linked %s.PluginLoaded", pluginPntr->className);
			}
		}
		
		if (PyObject_HasAttrString(pluginPntr->pyInstance, "Run"))
		{
			pluginPntr->pyFunc_Run = PyObject_GetAttrString(pluginPntr->pyInstance, "Run");
			PythonCheckError();
			if (pluginPntr->pyFunc_Run != nullptr)
			{
				DEBUG_PrintLine("Linked %s.Run", pluginPntr->className);
			}
		}
		
		if (PyObject_HasAttrString(pluginPntr->pyInstance, "ButtonPressed"))
		{
			pluginPntr->pyFunc_ButtonPressed = PyObject_GetAttrString(pluginPntr->pyInstance, "ButtonPressed");
			PythonCheckError();
			if (pluginPntr->pyFunc_ButtonPressed != nullptr)
			{
				DEBUG_PrintLine("Linked %s.ButtonPressed", pluginPntr->className);
			}
		}
		
		if (PyObject_HasAttrString(pluginPntr->pyInstance, "MousePressed"))
		{
			pluginPntr->pyFunc_MousePressed = PyObject_GetAttrString(pluginPntr->pyInstance, "MousePressed");
			PythonCheckError();
			if (pluginPntr->pyFunc_MousePressed != nullptr)
			{
				DEBUG_PrintLine("Linked %s.MousePressed", pluginPntr->className);
			}
		}
	}
	else
	{
		DEBUG_PrintLine("Failed to create %s instance", className);
	}
	Py_XDECREF(pyCreateArgs);
	Py_XDECREF(pyCreateKeywords);
	
	if (!pluginPntr->loaded)
	{
		pluginPntr->pyType = nullptr;
		pluginPntr->pyInstance = nullptr;
	}
	else
	{
		Plugin_PluginLoaded(pluginPntr);
	}
	
	PythonCheckError();
	return pluginPntr->loaded;
}

bool LoadPythonPluginModule(MemoryArena_t* arenaPntr, PythonPluginModule_t* pluginModule, const char* filePath)
{
	if (pluginModule->loaded)
	{
		UnloadModule(pluginModule);
	}
	ClearPointer(pluginModule);
	
	pluginModule->allocArena = arenaPntr;
	CopyStringIntoArray(pluginModule->filePath, filePath);
	char* moduleName = GetModuleNameFromPath(TempArena, filePath);
	CopyStringIntoArray(pluginModule->moduleName, moduleName);
	
	DEBUG_PrintLine("Attempting to load module \"%s\" from path \"%s\"", moduleName, filePath);
	pluginModule->pyModule = PyImport_ImportModule(moduleName);
	
	if (platform->GetFileTime(filePath, &pluginModule->fileWriteTime) && pluginModule->pyModule != nullptr)
	{
		pluginModule->loaded = true;
		
		//Find and count how many classes exist
		TempPushMark();
		{
			BoundedStrList_t classNames = {};
			BoundedStrListCreate(&classNames, MAX_PLUGINS_IN_MODULE, MAX_PLUGIN_NAME_LENGTH, TempArena);
			
			DEBUG_WriteLine("Looking for classes");
			u32 numClassesFound = FindModuleClasses(&classNames, pluginModule->pyModule);
			DEBUG_PrintLine("Found %u plugin classes", numClassesFound);
			
			u32 numValidClasses = 0;
			if (classNames.count > 0)
			{
				for (u32 cIndex = 0; cIndex < classNames.count; cIndex++)
				{
					char* className = classNames[cIndex];
					
					PyObject* pyClass = PyObject_GetAttrString(pluginModule->pyModule, className);
					bool isValid = IsClassValidPlugin(pyClass);
					if (isValid) { numValidClasses++; }
					DEBUG_PrintLine("Class[%u]: \"%s\" (%s)", cIndex, className, isValid ? "Valid" : "Invalid");
					Py_XDECREF(pyClass);
				}
			}
			
			pluginModule->numPlugins = numValidClasses;
			if (pluginModule->numPlugins > 0)
			{
				pluginModule->plugins = PushArray(arenaPntr, PythonPlugin_t, pluginModule->numPlugins);
				
				u32 pIndex = 0;
				for (u32 cIndex = 0; cIndex < classNames.count; cIndex++)
				{
					char* className = classNames[cIndex];
					PyObject* pyClass = PyObject_GetAttrString(pluginModule->pyModule, className);
					if (IsClassValidPlugin(pyClass))
					{
						Assert(pIndex < pluginModule->numPlugins);
						PythonPlugin_t* pluginPntr = &pluginModule->plugins[pIndex];
						
						if (LoadPluginFromClass(pluginPntr, className, pyClass))
						{
							DEBUG_PrintLine("Loaded \"%s\" successfully", className);
						}
						else
						{
							DEBUG_PrintLine("Failed to load \"%s\"", className);
							Py_XDECREF(pyClass);
						}
						
						pIndex++;
					}
					else
					{
						Py_XDECREF(pyClass);
					}
				}
			}
			else
			{
				pluginModule->plugins = nullptr;
			}
		}
		TempPopMark();
		
		PythonCheckError();
		pluginModule->checked = true;
		return true;
	}
	else
	{
		Py_XDECREF(pluginModule->pyModule);
		pluginModule->pyModule = nullptr;
		PythonCheckError();
		DEBUG_WriteLine("Unable to load module");
		pluginModule->checked = true;
		return false;
	}
}

void UpdatePythonModuleLoader()
{
	if (app->maxModules == 0)
	{
		//NOTE: On first pass we should allocate a single time for the maximum number of files found in the script folder
		Assert(app->numModules == 0);
		Assert(app->modules == nullptr);
		u32 numFiles = platform->GetNumFilesInFolder(SCRIPTS_FOLDER "/");
		
		app->maxModules = numFiles;
		if (app->maxModules < 8) { app->maxModules = 8; }
		app->modules = PushArray(mainHeap, PythonPluginModule_t, app->maxModules);
		
	}
	Assert(app->modules != nullptr);
	
	TempPushMark();
	u32 fIndex = 0;
	while (char* fileName = platform->GetFileInFolder(TempArena, SCRIPTS_FOLDER, fIndex++))
	{
		if (strstr(fileName, ".py") != nullptr)
		{
			PythonPluginModule_t* modulePntr = nullptr;
			char* filePath = StrCat(TempArena, NtStr(SCRIPTS_FOLDER), NtStr(fileName));
			
			for (u32 mIndex = 0; mIndex < app->numModules; mIndex++)
			{
				if (strcmp(filePath, app->modules[mIndex].filePath) == 0)
				{
					modulePntr = &app->modules[mIndex];
					break;
				}
			}
			
			FileTime_t newFileTime;
			if (modulePntr == nullptr)
			{
				DEBUG_PrintLine("Found new python module: \"%s\"", GetFileNamePart(filePath));
				if (app->numModules >= app->maxModules)
				{
					//not enough space for new module
					u32 newMax = app->maxModules + 8;
					DEBUG_PrintLine("Resizing module array allocation to hold %u modules", newMax);
					PythonPluginModule_t* newSpace = PushArray(mainHeap, PythonPluginModule_t, newMax);
					memcpy(newSpace, app->modules, app->maxModules * sizeof(PythonPluginModule_t));
					ArenaPop(mainHeap, app->modules);
					app->modules = newSpace;
					app->maxModules = newMax;
				}
				modulePntr = &app->modules[app->numModules];
				ClearPointer(modulePntr);
				
				LoadPythonPluginModule(mainHeap, modulePntr, filePath);
				app->numModules++;
				Assert(app->numModules <= app->maxModules);
			}
			else if (platform->GetFileTime(filePath, &newFileTime) && platform->CompareFileTimes(&newFileTime, &modulePntr->fileWriteTime) != 0)
			{
				DEBUG_PrintLine("%s module \"%s\"", modulePntr->loaded ? "Reloading" : "Loading", GetFileNamePart(filePath));
				LoadPythonPluginModule(mainHeap, modulePntr, filePath);
			}
		}
		
		TempPopMark();
		TempPushMark();
	}
	TempPopMark();
}

void UnloadAllPythonModules()
{
	DEBUG_WriteLine("Unloading all modules");
	
	if (app->numModules > 0)
	{
		for (u32 mIndex = 0; mIndex < app->numModules; mIndex++)
		{
			PythonPluginModule_t* modulePntr = &app->modules[mIndex];
			if (modulePntr->loaded)
			{
				UnloadModule(modulePntr);
			}
		}
		
		app->numModules = 0;
	}
	
	if (app->modules != nullptr)
	{
		ArenaPop(mainHeap, app->modules);
		app->modules = nullptr;
		app->maxModules = 0;
	}
}

PythonPlugin_t* GetPluginByName(const char* nameStr)
{
	for (u32 mIndex = 0; mIndex < app->numModules; mIndex++)
	{
		PythonPluginModule_t* modulePntr = &app->modules[mIndex];
		if (modulePntr->loaded)
		{
			for (u32 pIndex = 0; pIndex < modulePntr->numPlugins; pIndex++)
			{
				PythonPlugin_t* pluginPntr = &modulePntr->plugins[pIndex];
				if (strcmp(pluginPntr->className, nameStr) == 0)
				{
					return pluginPntr;
				}
			}
		}
	}
	
	return nullptr;
}

// +--------------------------------------------------------------+
// |                       Plugin Functions                       |
// +--------------------------------------------------------------+
void Plugin_PluginLoaded(const PythonPlugin_t* pluginPntr)
{
	Assert(pluginPntr != nullptr);
	
	if (pluginPntr->pyFunc_PluginLoaded != nullptr)
	{
		DEBUG_PrintLine("Calling %s.PluginLoaded()", pluginPntr->className);
		
		PythonCheckError();
		PyObject* pyReturnValue = PyObject_CallFunction(pluginPntr->pyFunc_PluginLoaded, nullptr);
		if (pyReturnValue != nullptr)
		{
			// DEBUG_WriteLine("Call Success!");
		}
		else
		{
			DEBUG_WriteLine("Call Failed!");
		}
		
		Py_XDECREF(pyReturnValue);
	}
	
	PythonCheckError();
}

void Plugin_Run(const PythonPlugin_t* pluginPntr)
{
	Assert(pluginPntr != nullptr);
	
	if (pluginPntr->pyFunc_Run != nullptr)
	{
		DEBUG_PrintLine("Calling %s.Run()", pluginPntr->className);
		
		PythonCheckError();
		PyObject* pyReturnValue = PyObject_CallFunction(pluginPntr->pyFunc_Run, nullptr);
		if (pyReturnValue != nullptr)
		{
			// DEBUG_WriteLine("Call Success!");
		}
		else
		{
			DEBUG_WriteLine("Call Failed!");
		}
		
		Py_XDECREF(pyReturnValue);
	}
	
	PythonCheckError();
}

void Plugin_ButtonPressed(const PythonPlugin_t* pluginPntr, const char* button)
{
	Assert(pluginPntr != nullptr);
	
	if (pluginPntr->pyFunc_ButtonPressed != nullptr)
	{
		// DEBUG_PrintLine("Calling %s.ButtonPressed(%s)", pluginPntr->className, button);
		
		PythonCheckError();
		PyObject* pyReturnValue = PyObject_CallFunction(pluginPntr->pyFunc_ButtonPressed, "s", button);
		if (pyReturnValue != nullptr)
		{
			// DEBUG_WriteLine("Call Success!");
		}
		else
		{
			DEBUG_WriteLine("Call Failed!");
		}
		
		Py_XDECREF(pyReturnValue);
	}
	
	PythonCheckError();
}

void Plugin_MousePressed(const PythonPlugin_t* pluginPntr, v2 mousePos)
{
	Assert(pluginPntr != nullptr);
	
	if (pluginPntr->pyFunc_MousePressed != nullptr)
	{
		// DEBUG_PrintLine("Calling %s.MousePressed(%f, %f)", pluginPntr->className, mousePos.x, mousePos.y);
		
		PythonCheckError();
		PyObject* pyReturnValue = PyObject_CallFunction(pluginPntr->pyFunc_MousePressed, "f,f", mousePos.x, mousePos.y);
		if (pyReturnValue != nullptr)
		{
			// DEBUG_WriteLine("Call Success!");
		}
		else
		{
			DEBUG_WriteLine("Call Failed!");
		}
		
		Py_XDECREF(pyReturnValue);
	}
	
	PythonCheckError();
}

// +--------------------------------------------------------------+
// |                 All Plugin Function Routers                  |
// +--------------------------------------------------------------+
void AllPlugins_ButtonPressed(const char* button)
{
	for (u32 mIndex = 0; mIndex < app->numModules; mIndex++)
	{
		PythonPluginModule_t* modulePntr = &app->modules[mIndex];
		if (modulePntr->loaded)
		{
			for (u32 pIndex = 0; pIndex < modulePntr->numPlugins; pIndex++)
			{
				PythonPlugin_t* pluginPntr = &modulePntr->plugins[pIndex];
				if (pluginPntr->loaded) { Plugin_ButtonPressed(pluginPntr, button); }
			}
		}
	}
}
void AllPlugins_MousePressed(v2 mousePos)
{
	for (u32 mIndex = 0; mIndex < app->numModules; mIndex++)
	{
		PythonPluginModule_t* modulePntr = &app->modules[mIndex];
		if (modulePntr->loaded)
		{
			for (u32 pIndex = 0; pIndex < modulePntr->numPlugins; pIndex++)
			{
				PythonPlugin_t* pluginPntr = &modulePntr->plugins[pIndex];
				if (pluginPntr->loaded) { Plugin_MousePressed(pluginPntr, mousePos); }
			}
		}
	}
}


bool RunCommand(const char* command)
{
	PythonPlugin_t* pluginPntr = GetPluginByName(command);
	if (pluginPntr != nullptr)
	{
		Plugin_Run(pluginPntr);
		return true;
	}
	else
	{
		return false;
	}
}