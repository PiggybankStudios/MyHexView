/*
File:   app_pythonFunctions.cpp
Author: Taylor Robbins
Date:   03\10\2018
Description: 
	** Holds all the functions that python can call to perform actions in the program 
*/

PyObject* Py_SetBackgroundColor(PyObject* self, PyObject* args)
{
	int colorIndex;
	if (!PyArg_ParseTuple(args, "i", &colorIndex)) { return nullptr; }
	
	DEBUG_PrintLine("Called SetBackgroundColor(%u)", colorIndex);
	defData->backgroundColorIndex = colorIndex;
	return PyLong_FromLong(colorIndex);
}

PyMethodDef PgMethods[] = {
	{"SetBackgroundColor", Py_SetBackgroundColor, METH_VARARGS, "Change the background color"},
	{nullptr, nullptr, 0, nullptr},
};

struct PyModuleDef PgModule = {
	PyModuleDef_HEAD_INIT,
	"SomeOtherName",
	nullptr, //module documentation
	-1, //size of module state
	PgMethods
};

PyMODINIT_FUNC Py_PgInit()
{
	DEBUG_WriteLine("Py_PgInit Called");
	return PyModule_Create(&PgModule);
}

void AddPythonFunctions()
{
	int returnValue = PyImport_AppendInittab("SomeOtherName", Py_PgInit);
	DEBUG_PrintLine("PyImport_AppendInittab = %d", returnValue);
	PyObject* pyPgModule = PyImport_ImportModule("SomeOtherName");
	DEBUG_PrintLine("pyPgModule = %p", pyPgModule);
	//TODO: Figure out why this is returning nullptr!
}
