#ifndef eman__typeconverter_h__
#define eman__typeconverter_h__ 1

#define PY_ARRAY_UNIQUE_SYMBOL PyArrayHandle

#include "emobject.h"
#include "transform.h"
#include "geometry.h"
#include "emdata.h"

#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <Numeric/arrayobject.h>

#include <vector>
#include <map>
#include <string>


namespace python = boost::python;
using std::vector;
using std::map;


namespace EMAN {

	class Wrapper {
	public:
		/** Get an EMData image's pixel data as a numeric numpy array.
		 * The array and EMData image share the same memory block.
		 */
		static python::numeric::array em2numpy(EMData *image);

		/** Create an EMData image from a numeric numpy array.
		 * The image and the array share the same memory block.
		 */
		static void numpy2em(python::numeric::array& array, EMData* image);
	

		template <class T>
		static vector<T> list2vector(const python::list& l)
		{
			vector<T> v;
			for (int i = 0; i < python::len(l); i++) {
				v.push_back(python::extract<T>(l[i]));
		
			}
			return v;
		}

		template <class T>
		static python::list vector2list(const vector<T>& v)
		{
			python::list l;
			for (unsigned int i = 0; i < v.size(); i++) {
				l.append(v[i]);
			}
			return l;
		}
    };
    
    template <class T>
    struct vector_to_python : python::to_python_converter<vector<T>,
														  vector_to_python<T> >
    {
		static PyObject* convert(vector<T> const& v)
		{
			python::list result;
	    
			for (size_t i = 0; i < v.size(); i++) {
				result.append(v[i]);
			}
	    
			return python::incref(python::list(result).ptr());
		}
    };

	template <class T>
	struct tuple3_to_python : python::to_python_converter<T, tuple3_to_python<T> >
	{
		static PyObject* convert(T const& p)
		{
			python::tuple result = python::make_tuple(p[0], p[1], p[2]);
			return python::incref(python::tuple(result).ptr());
		}
	};
	
	
    template <class T>
    struct map_to_python : python::to_python_converter<map<std::string, T>,
													   map_to_python<T> >
    {
		static PyObject* convert(map<std::string, T> const& d)
		{
			python::dict result;

			typedef typename map<std::string, T>::const_iterator MI;
			for (MI p = d.begin(); p != d.end(); p++) {
				result[p->first] = p->second;
			}
	
			return python::incref(python::dict(result).ptr());
		}
    };

    struct Dict_to_python : python::to_python_converter<Dict, Dict_to_python>
    {
		static PyObject* convert(Dict const& dd)
		{
			python::dict result;
			vector<std::string> keys = dd.keys();
			vector<EMObject> values = dd.values();
			for (unsigned int i = 0; i < keys.size(); i++) {
				result[keys[i]] = values[i];
			}
	
			return python::incref(python::dict(result).ptr());
		}
    };

    template <class T>
    struct vector_from_python
    {
		vector_from_python()
		{
			python::converter::registry::push_back(&convertible, &construct,
												   python::type_id<vector<T> >());
		}
    
		static void* convertible(PyObject* obj_ptr)
		{
			if (!(PyList_Check(obj_ptr) || PyTuple_Check(obj_ptr)
				  || PyIter_Check(obj_ptr)  || PyRange_Check(obj_ptr))) {
				return 0;
			}
	
			return obj_ptr;
		}

    
		static void construct(PyObject* obj_ptr,
							  python::converter::rvalue_from_python_stage1_data* data)
		{
			void* storage = ((python::converter::rvalue_from_python_storage<vector<T> >*)
							 data)->storage.bytes;
			new (storage) vector<T>();

			data->convertible = storage;

			vector<T>& result = *((vector<T>*) storage);
	
			python::handle<> obj_iter(PyObject_GetIter(obj_ptr));
	
			while(1) {
				python::handle<> py_elem_hdl(python::allow_null(PyIter_Next(obj_iter.get())));
				if (PyErr_Occurred()) {
					python::throw_error_already_set();
				}
	    
				if (!py_elem_hdl.get()) {
					break;
				}
	    
				python::object py_elem_obj(py_elem_hdl);
				python::extract<T> elem_proxy(py_elem_obj);
				result.push_back(elem_proxy());
			}
		}
    };
    
    template <class T>
    struct map_from_python
    {
		map_from_python()
		{
			python::converter::registry::push_back(&convertible, &construct,
												   python::type_id<map<std::string, T> >());
		}
    
		static void* convertible(PyObject* obj_ptr)
		{
			if (!(PyDict_Check(obj_ptr))) {
				return 0;
			}
	
			return obj_ptr;
		}

    
		static void construct(PyObject* obj_ptr,
							  python::converter::rvalue_from_python_stage1_data* data)
		{
			void* storage = ((python::converter::rvalue_from_python_storage<map<std::string, T> >*)
							 data)->storage.bytes;
			new (storage) map<std::string, T>();
			data->convertible = storage;
			map<std::string, T>& result = *((map<std::string, T>*) storage);

			python::dict d = python::extract<python::dict>(obj_ptr);
			
			python::list k = d.keys();
			python::list v = d.values();
			long l = python::len(k);
	
			for(long i = 0; i < l; i++) {
				std::string key = python::extract<std::string>(k[i]);
				T val = python::extract<T>(v[i]);
				result[key] = val;
			}

		}
    };
#if 0	
    struct map_from_python_int
    {
		map_from_python_int()
		{
			python::converter::registry::push_back(&convertible, &construct,
												   python::type_id<map<std::string, int> >());
		}
    
		static void* convertible(PyObject* obj_ptr)
		{
			if (!(PyDict_Check(obj_ptr))) {
				return 0;
			}
	
			return obj_ptr;
		}

    
		static void construct(PyObject* obj_ptr,
							  python::converter::rvalue_from_python_stage1_data* data)
		{
			void* storage = ((python::converter::rvalue_from_python_storage<map<std::string, int> >*)
							 data)->storage.bytes;
			new (storage) map<std::string, int>();
			data->convertible = storage;			
			map<std::string, int>& result = *((map<std::string, int>*) storage);

			python::dict d = python::extract<python::dict>(obj_ptr);
		       
			python::list k = d.keys();
			python::list v = d.values();
			long l = python::len(k);

			for(long i = 0; i < l; i++) {
				std::string key = python::extract<std::string>(k[i]);
				int val = python::extract<int>(v[i]);
				result[key] = val;
			}
		}
    };
#endif
    struct Dict_from_python
    {
		Dict_from_python()
		{
			python::converter::registry::push_back(&convertible, &construct,
												   python::type_id<Dict>());
		}
    
		static void* convertible(PyObject* obj_ptr)
		{
			if (!(PyDict_Check(obj_ptr))) {
				return 0;
			}
	
			return obj_ptr;
		}

    
		static void construct(PyObject* obj_ptr,
							  python::converter::rvalue_from_python_stage1_data* data)
		{
			void* storage = ((python::converter::rvalue_from_python_storage<Dict>*)
							 data)->storage.bytes;
			new (storage) Dict();
			data->convertible = storage;
			Dict& result = *((Dict*) storage);

			python::handle<> obj_handle(obj_ptr);
			python::object dict_obj(obj_handle);
	    
			python::dict d(dict_obj);
	    
			python::list k = d.keys();
			python::list v = d.values();
			long l = python::len(k);
	
			for(long i = 0; i < l; i++) {
				std::string key = python::extract<std::string>(k[i]);
				EMObject val = python::extract<EMObject>(v[i]);
				result.put(key, val);
			}

		}
    };

	template<class T, class T2>
	struct tuple3_from_python
    {
		tuple3_from_python()
		{
			python::converter::registry::push_back(&convertible, &construct,
												   python::type_id<T>());
		}
    
		static void* convertible(PyObject* obj_ptr)
		{
			if (!(PyList_Check(obj_ptr) || PyTuple_Check(obj_ptr)
				  || PyIter_Check(obj_ptr)  || PyRange_Check(obj_ptr))) {
				return 0;
			}
	
			return obj_ptr;
		}

    
		static void construct(PyObject* obj_ptr,
							  python::converter::rvalue_from_python_stage1_data* data)
		{
			void* storage = ((python::converter::rvalue_from_python_storage<T>*)
							 data)->storage.bytes;
			new (storage) T();

			data->convertible = storage;

			T& result = *((T*) storage);
	
			python::handle<> obj_iter(PyObject_GetIter(obj_ptr));
			int i = 0;
			
			while(1) {
				python::handle<> py_elem_hdl(python::allow_null(PyIter_Next(obj_iter.get())));
				if (PyErr_Occurred()) {
					python::throw_error_already_set();
				}
	    
				if (!py_elem_hdl.get()) {
					break;
				}
	    
				python::object py_elem_obj(py_elem_hdl);
				python::extract<T2> elem_proxy(py_elem_obj);
				result[i] = elem_proxy();
				i++;
			}
		}
    };

#if 0
    struct Vec3i_from_python
    {
		Vec3i_from_python()
		{
			python::converter::registry::push_back(&convertible, &construct,
												   python::type_id<Vec3i>());
		}
    
		static void* convertible(PyObject* obj_ptr)
		{
			if (!(PyList_Check(obj_ptr) || PyTuple_Check(obj_ptr)
				  || PyIter_Check(obj_ptr)  || PyRange_Check(obj_ptr))) {
				return 0;
			}
	
			return obj_ptr;
		}

    
		static void construct(PyObject* obj_ptr,
							  python::converter::rvalue_from_python_stage1_data* data)
		{
			void* storage = ((python::converter::rvalue_from_python_storage<Vec3i>*) data)->storage.bytes;
			new (storage) Vec3i();

			data->convertible = storage;

			Vec3i& result = *((Vec3i*) storage);
	
			python::handle<> obj_iter(PyObject_GetIter(obj_ptr));
			int i = 0;
			
			while(1) {
				python::handle<> py_elem_hdl(python::allow_null(PyIter_Next(obj_iter.get())));
				if (PyErr_Occurred()) {
					python::throw_error_already_set();
				}
	    
				if (!py_elem_hdl.get()) {
					break;
				}
	    
				python::object py_elem_obj(py_elem_hdl);
				python::extract<int> elem_proxy(py_elem_obj);
				result[i] = elem_proxy();
				i++;
			}
		}
    };
#endif
	
    struct emobject_farray_from_python
    {
		emobject_farray_from_python()
		{
			python::converter::registry::push_back(&convertible, &construct,
												   python::type_id<EMObject>());
		}
    
		static void* convertible(PyObject* obj_ptr)
		{
			if (!(PyList_Check(obj_ptr) || PyTuple_Check(obj_ptr)
				  || PyIter_Check(obj_ptr)  || PyRange_Check(obj_ptr))) {
				return 0;
			}
	
			return obj_ptr;
		}

    
		static void construct(PyObject* obj_ptr,
							  python::converter::rvalue_from_python_stage1_data* data)
		{
			void* storage = ((python::converter::rvalue_from_python_storage<EMObject>*)
							 data)->storage.bytes;
			new (storage) EMObject();

			data->convertible = storage;

			EMObject& result = *((EMObject*) storage);
	
			python::handle<> obj_iter(PyObject_GetIter(obj_ptr));
			vector<float> farray;
			
			while(1) {
				python::handle<> py_elem_hdl(python::allow_null(PyIter_Next(obj_iter.get())));
				if (PyErr_Occurred()) {
					python::throw_error_already_set();
				}
	    
				if (!py_elem_hdl.get()) {
					break;
				}
	    
				python::object py_elem_obj(py_elem_hdl);
				python::extract<float> elem_proxy(py_elem_obj);
				farray.push_back(elem_proxy());
			}
			result = EMObject(farray);
		}
    };
	
	struct emobject_emdata_from_python
    {
		emobject_emdata_from_python()
		{
			python::converter::registry::push_back(&convertible, &construct,
												   python::type_id<EMObject>());
		}
    
		static void* convertible(PyObject* obj_ptr)
		{
			if (PyInt_Check(obj_ptr) || PyLong_Check(obj_ptr) || PyFloat_Check(obj_ptr) ||
				PyString_Check(obj_ptr) || PyDict_Check(obj_ptr) || PyMapping_Check(obj_ptr) ||
				PyNumber_Check(obj_ptr) || PyType_Check(obj_ptr) || PySequence_Check(obj_ptr) ||
				PyList_Check(obj_ptr) || PyTuple_Check(obj_ptr) || PyIter_Check(obj_ptr)) {
				return 0;
			}
			return obj_ptr;
		}
    
		static void construct(PyObject* obj_ptr,
							  python::converter::rvalue_from_python_stage1_data* data)
		{
			void* storage =
				((python::converter::rvalue_from_python_storage<EMObject>*)
				 data)->storage.bytes;
			new (storage) EMObject();

			data->convertible = storage;
			EMObject& result = *((EMObject*) storage);
			EMData * emdata = python::extract<EMData*>(obj_ptr);	   
			result = EMObject(emdata);
		}
    };
}


#endif
