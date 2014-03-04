#ifndef PYTHONWRAPPER_TYPES__H
#define PYTHONWRAPPER_TYPES__H

namespace py
{

template<typename user_type>
class type_base
{
public:

    type_base()
    {
    }

    ~type_base()
    {
    }

    static bool check_exact( PyObject* op )
    {
        return op->ob_type == &_type;
    }

    static user_type& ensure_exact( PyObject* o)
    {
        if ( ! check_exact(o) )
        {
            throw exception(PyExc_TypeError);
        }

        return *reinterpret_cast<user_type*>(o);
    }

    static bool check( PyObject* op )
    {
        return check_exact(op) || Type_IsSubtype(op->ob_type, &_type);
    }

    static user_type& ensure( PyObject* o)
    {
        if ( ! check(o) )
        {
            throw exception(PyExc_TypeError);
        }

        return *reinterpret_cast<user_type*>(o);
    }

    static void initialize(const char* tp_name, PyTypeObject* supertype=&PyBaseObject_Type)
    {
        _supertype = supertype;

        _type.tp_name = tp_name;
        _type.tp_basicsize = sizeof(user_type);
        _type.tp_base = _supertype;
        _type.tp_flags |= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;

        _type.tp_dealloc = wrappers::destructor<user_type>;

        PyType_Ready(&_type);
    }

    static void add_to_module(PyObject* module, const char* name)
    {
        Module_AddObject( module, name, borrow(&_type) );
    }

    static void tp_dealloc(user_type* d)
    {
        d->~user_type();
        _supertype->tp_dealloc(reinterpret_cast<PyObject*>(d));
    }

//    static ref<user_type> build()
//    {
//        ref<user_type> p = alloc();
//
//        new (p) user_type();
//
//        return p;
//    }
//
//    template<typename T1>
//    static ref<user_type> build(const T1& t1)
//    {
//        ref<user_type> p = alloc();
//
//        new (p) user_type(t1);
//
//        return p;
//    }
//
//    template<typename T1>
//    static ref<user_type> build(T1& t1)
//    {
//        ref<user_type> p = alloc();
//
//        new (p) user_type(t1);
//
//        return p;
//    }
//    template<typename T1, typename T2>
//    static ref<user_type> build(T1& t1, T2& t2)
//    {
//        ref<user_type> p = alloc();
//
//        new (p) user_type(t1, t2);
//
//        return p;
//    }
//
//    template<typename T1, typename T2, typename T3>
//    static ref<user_type> build(T1& t1, T2& t2, T3& t3)
//    {
//        ref<user_type> p = alloc();
//
//        new (p) user_type(t1, t2, t3);
//
//        return p;
//    }

    template<typename... Ts>
    static ref<user_type> build(const Ts&... ts)
    {
        ref<user_type> p = alloc();

        new (p) user_type(ts...);

        return p;
    }

    template<typename T, typename... Ts>
    static ref<user_type> build(T& t, Ts&... ts)
    {
        ref<user_type> p = alloc();

        new (p) user_type(t, ts...);

        return p;
    }

    static PyTypeObject* _supertype;
    static PyTypeObject _type;

    typedef type_base<user_type> base;

private:

    PyObject_HEAD

    static ref<user_type> alloc()
    {
        ref<user_type> p( _type.tp_alloc(&_type, 0) );
        exception::check();
        return p;
    }
};

template<typename user_type>
PyTypeObject* type_base<user_type>::_supertype;

template<typename user_type>
PyTypeObject type_base<user_type>::_type = {
    PyObject_HEAD_INIT(NULL)
    0
};

template<typename user_type>
class type_base_with_new :
    public type_base<user_type>
{
public:

    static PyObject* base_construct(PyTypeObject *subtype, PyObject * /*args*/, PyObject * /*kwds*/)
    {
        return user_type::_type.tp_alloc(subtype, 0);
    }

    static void construct(user_type* p, PyObject* args, PyObject* kwds)
    {
        new (p) user_type();
    }

    static void initialize(const char* tp_name, PyTypeObject* supertype=&PyBaseObject_Type)
    {
        user_type::_type.tp_new = wrappers::newfunc<user_type>;
        type_base<user_type>::initialize(tp_name, supertype);
    }

    static user_type* tp_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds)
    {
        ref<user_type> p( user_type::base_construct(subtype, args, kwds) );
        exception::check();

        user_type::construct(p, args, kwds);

        return p.release();
    }

    typedef type_base_with_new<user_type> base;
};

template<typename user_metatype>
class metatype_base
{
public:

    metatype_base()
    {
    }

    ~metatype_base()
    {
    }

    static void initialize(const char* tp_name, PyTypeObject* supertype=&PyType_Type)
    {
        _supertype = supertype;

        _type.tp_name = tp_name;
        _type.tp_basicsize = sizeof(user_metatype);
        _type.tp_base = _supertype;
        _type.tp_flags |= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;

        _type.tp_dealloc = wrappers::destructor<user_metatype>;

        PyType_Ready(&_type);
    }

    static void tp_dealloc(user_metatype* d)
    {
        d->~user_metatype();
        _supertype->tp_dealloc(reinterpret_cast<PyObject*>(d));
    }

    static PyTypeObject* _supertype;
    static PyTypeObject _type;

    typedef metatype_base<user_metatype> base;

private:

    PyHeapTypeObject _heaptype;
};

template<typename user_metatype>
PyTypeObject* metatype_base<user_metatype>::_supertype;

template<typename user_metatype>
PyTypeObject metatype_base<user_metatype>::_type = {
    PyObject_HEAD_INIT(NULL)
    0
};

} // namespace py

#endif // PYTHONWRAPPER_TYPES__H