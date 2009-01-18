/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module.h"

namespace kroll
{
	void PythonUnitTestSuite::Run(Host *host)
	{
		SharedValue tv1 = new Value(1);
		SharedPtr<PyObject> v1 = PythonUtils::ToObject(tv1);
		KR_ASSERT(PyInt_Check(v1));
		KR_ASSERT(PyInt_AsLong(v1)==1);

		SharedValue tv2 = new Value(1.0);
		PyObject* v2 = PythonUtils::ToObject(tv2);
		KR_ASSERT(PyFloat_Check(v2));
		KR_ASSERT(PyFloat_AsDouble(v2)==1.0);

		SharedValue tv3 = new Value("abc");
		PyObject* v3 = PythonUtils::ToObject(tv3);
		KR_ASSERT(PyString_Check(v3));
		KR_ASSERT_STR(PyString_AsString(v3),"abc");

		std::string s1("abc");
		SharedValue tv4 = new Value(s1);
		PyObject* v4 = PythonUtils::ToObject(tv4);
		KR_ASSERT(PyString_Check(v4));
		KR_ASSERT_STR(PyString_AsString(v4),"abc");

		SharedValue tv5 = new Value();
		PyObject* v5 = PythonUtils::ToObject(tv5);
		KR_ASSERT(Py_None==v5);

		SharedValue tv6 = Value::Undefined;
		PyObject* v6 = PythonUtils::ToObject(tv6);
		KR_ASSERT(Py_None==v6);

		SharedValue tv7 = Value::Null;
		PyObject* v7 = PythonUtils::ToObject(tv7);
		KR_ASSERT(Py_None==v7);

		SharedValue tv8 = new Value(true);
		PyObject* v8 = PythonUtils::ToObject(tv8);
		KR_ASSERT(PyBool_Check(v8));
		KR_ASSERT(v8 == Py_True);

		std::string script1;
		script1+="class Foo:\n";
		script1+="	i = 12345\n";
		script1+="	def bar(self):\n";
		script1+="		return 'hello,world'\n";
		script1+="	def blah(self,x):\n";
		script1+="		return x\n";
		script1+="	def blok(self,fn):\n";
		script1+="		fn()\n";
		script1+="\n";

		PyRun_SimpleString(script1.c_str());

		PyObject* main_module = PyImport_AddModule("__main__");
		PyObject* global_dict = PyModule_GetDict(main_module);
		PyObject* expression = PyDict_GetItemString(global_dict, "Foo");
		SharedValue cl1 = PythonUtils::ToValue(expression);
		KR_ASSERT(cl1->IsObject());
		SharedPtr<BoundObject> value = new PythonBoundObject(expression);
		//ScopedDereferencer sd1(value);
		SharedValue p1 = value->Get("bar");
		KR_ASSERT(p1->IsMethod());

		// TEST creating instance and invoking methods
		PyRun_SimpleString("foopy = Foo()");
		PyObject* foopy = PyDict_GetItemString(global_dict, "foopy");
		SharedValue cl2 = PythonUtils::ToValue(foopy);
		//ScopedDereferencer r(cl2);
		KR_ASSERT(cl2->IsObject());
		SharedPtr<BoundObject> value2 = new PythonBoundObject(foopy);
		//ScopedDereferencer sd2(value2);

		// TEST method
		SharedValue p2 = value2->Get("bar");
		KR_ASSERT(p2->IsMethod());
		SharedBoundMethod m1 = p2->ToMethod();
		ValueList args;
		SharedValue mv1 = m1->Call(args);
		KR_ASSERT(!mv1->IsNull());
		KR_ASSERT_STR(mv1->ToString(),"hello,world");

		// TEST property accessor
		SharedValue p3 = value2->Get("i");
		KR_ASSERT(p3->IsInt());
		KR_ASSERT(p3->ToInt()==12345);

		// TEST setting properties
		SharedValue set1 = new Value(6789);
		value2->Set("i",set1);
		SharedValue p4 = value2->Get("i");
		KR_ASSERT(p4->IsInt());
		KR_ASSERT(p4->ToInt()==6789);

		// TEST setting invalid property - this should add it dynamically
		SharedValue set2 = new Value(1);
		value2->Set("i2",set2);
		SharedValue p5 = value2->Get("i2");
		KR_ASSERT(p5->IsInt());
		KR_ASSERT(p5->ToInt()==1);

		// TEST undefined property
		SharedValue p6 = value2->Get("x");
		KR_ASSERT(p6->IsUndefined());

		// TEST invoking a function with wrong parameters and checking exception
		SharedValue p7 = value2->Get("blah");
		KR_ASSERT(p7->IsMethod());
		SharedBoundMethod mb2 = p7->ToMethod();
		SharedValue mv3;
		try
		{
			mv3 = mb2->Call(args);
			KR_ASSERT(false);
		}
		catch (SharedValue e)
		{
			KR_ASSERT_STR(e->ToString(),"blah() takes exactly 2 arguments (1 given)");
		}

		// TEST invoking a function with correct parameters
		ValueList args2;
		SharedValue argsp1 = new Value("hello,world");
		args2.push_back(argsp1);
		SharedValue mv4 = mb2->Call(args2);
		KR_ASSERT(mv4);

		// TEST for Python callable functions
		PyRun_SimpleString("def foopyc(): return 'hello,world'");
		PyObject* foopyc = PyDict_GetItemString(global_dict, "foopyc");
		KR_ASSERT(foopyc);
		SharedValue cl3 = PythonUtils::ToValue(foopyc);
		KR_ASSERT(cl3->IsMethod());
		SharedBoundMethod m2 = cl3->ToMethod();
		SharedValue mv2 = m2->Call(args);
		KR_ASSERT(!mv2->IsNull());
		KR_ASSERT_STR(mv2->ToString(),"hello,world");

		// TEST creating an anonymous method and calling through it
		PyObject* anon1 = PythonUtils::ToObject(cl3->ToMethod());
		KR_ASSERT(PyCallable_Check(anon1));

		// TEST calling through a BoundObject from Python
		SharedBoundObject tbo = cl1->ToObject();
		PyObject* co1 = PythonUtils::ToObject(NULL,NULL,tbo);
		KR_ASSERT(co1);
		PyObject* cop1 = PyObject_GetAttrString(co1, "bar");
		KR_ASSERT(cop1);

		SharedValue cl5 = PythonUtils::ToValue(foopyc);
		KR_ASSERT(cl5->IsMethod());
		SharedBoundMethod m4 = cl5->ToMethod();
		SharedValue mv6 = m4->Call(args);
		KR_ASSERT_STR(mv6->ToString(),"hello,world");

		PyObject *anon2 = PythonUtils::ToObject(m4);
		KR_ASSERT(PyCallable_Check(anon2));
		SharedValue tiv1 = PythonUtils::ToValue(anon2);
		KR_ASSERT(tiv1->IsMethod());
		BoundMethod *tibm1 = tiv1->ToMethod();
		SharedValue tivr1 = tibm1->Call(args);
		KR_ASSERT_STR(tivr1->ToString(),"hello,world");


		PyObject* piv = PythonUtils::ToObject(tibm1);
		SharedValue pivv = PythonUtils::ToValue(piv);
		BoundMethod *pivbm = pivv->ToMethod();
		SharedValue pivbmv = pivbm->Call(args);
		KR_ASSERT_STR(pivbmv->ToString(),"hello,world");

		std::string script2;
		script2+=PRODUCT_NAME".set('x',123)\n";
		script2+="f = "PRODUCT_NAME".get('x') + 1\n";
		script2+=PRODUCT_NAME".set('y',f)\n";
		script2+="\n";

		PyRun_SimpleString(script2.c_str());

		// TEST pulling out the new bound values
		SharedValue x = host->GetGlobalObject()->GetNS("python.x");
		KR_ASSERT(x->ToInt()==123);

		SharedValue y = host->GetGlobalObject()->GetNS("python.y");
		KR_ASSERT(y->ToInt()==124);

		//KR_DECREF(x);
		//KR_DECREF(y);

		SharedPtr<BoundList> list = new StaticBoundList();
		KR_ASSERT(list->Size()==0);
		KR_ASSERT(list->At(0)->IsUndefined());
		SharedValue lista = new Value(1);
		list->Append(lista);
		KR_ASSERT(list->Size()==1);
		//KR_DECREF(lista);

		SharedValue listv = new Value(list);
		PyObject* apylist = PythonUtils::ToObject(listv);
		KR_ASSERT(apylist);
		KR_ASSERT(listv->IsList());
		KR_ASSERT(PyList_Check(apylist));
		KR_ASSERT(PyList_Size(apylist)==list->Size());

		PyObject *pitem = PyList_GetItem(apylist,0);
		KR_ASSERT(pitem);
		KR_ASSERT(pitem != Py_None);
		SharedValue vitem = PythonUtils::ToValue(pitem);
		KR_ASSERT(vitem->IsInt());
		KR_ASSERT(vitem->ToInt()==1);

		SharedPtr<BoundList> plist = new PythonBoundList(apylist);
		KR_ASSERT(plist->Size()==1);
		KR_ASSERT(plist->At(0)->ToInt()==1);
		SharedValue vlist2 = new Value("hello");
		plist->Append(vlist2);
		KR_ASSERT(plist->Size()==2);
		SharedValue vlist3 = plist->At(1);
		KR_ASSERT(std::string(vlist3->ToString())=="hello");
		KR_ASSERT(list->At(10)->IsUndefined());

		//KR_DECREF(vlist2);
		//KR_DECREF(plist);
		Py_DECREF(pitem);
		//KR_DECREF(vitem);
		//KR_DECREF(listv);
		//KR_DECREF(list);
		Py_DECREF(apylist);

	}
}
