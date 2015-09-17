template<typename FuncSignature>
class Callback;

template<typename R, typename A1>
class Callback<R(A1)>
{
public:
    typedef R(*FuncType)(void*, A1);

    Callback() : obj(0), func(0) {}
    Callback(void* o, FuncType f) : obj(o), func(f) {}

    R operator()(A1 a1) const
    {
        return (*func)(obj, a1);
    }

    typedef void* Callback::*SafeBoolType;
    operator SafeBoolType() const
    {
        return func != 0 ? &Callback::obj : 0;
    }

    bool operator!() const
    {
        return func == 0;
    }

private:
    void* obj;
    FuncType func;
};

template<typename R, typename A1> // Undefined on purpose
void operator==(const Callback<R(A1)>&, const Callback<R(A1)>&);
template<typename R, typename A1>
void operator!=(const Callback<R(A1)>&, const Callback<R(A1)>&);

template<typename R, class T, typename A1>
struct DeduceMemCallbackTag
{
    template<R(T::*Func)(A1)>
    static R Wrapper(void* o, A1 a1)
    {
        return (static_cast<T*>(o)->*Func)(a1);
    }

    template<R(T::*Func)(A1)>
    inline static Callback<R(A1)> Bind(T* o)
    {
        return Callback<R(A1)>(o, &DeduceMemCallbackTag::Wrapper<Func>);
    }
};

template<typename R, class T, typename A1>
DeduceMemCallbackTag<R, T, A1> DeduceMemCallback(R(T::*)(A1))
{
    return DeduceMemCallbackTag<R, T, A1>();
}

#define BIND_MEM_CB(memFuncPtr, instancePtr) \
    (DeduceMemCallback(memFuncPtr).Bind<(memFuncPtr)>(instancePtr))