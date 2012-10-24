#ifndef __REF_HPP__
#define __REF_HPP__

template< typename T > struct DeleterBase;

template< typename T >
class ref
{
public:
	ref()
	: ptr_(0)
	, deleter_(0)
	, ref_count_(0)
	{
	}

	ref(T *p, DeleterBase<T> *del = 0)
		: ptr_(p)
		, deleter_(del)
		, ref_count_(0)
	{
		if ( ptr_ ) {
			ref_count_ = new unsigned int(1);
		}
	}
	
	virtual ~ref()
	{
		release();
	}
	
	ref(const ref<T> &other)
		: ptr_(other.ptr_)
		, deleter_(other.deleter_)
		, ref_count_(other.ref_count_)
	{
		if ( ptr_ ) {
			add();
		}
	}
	
	ref<T> & operator=(const ref<T> &other)
	{
		if ( (this == &other) || (ptr_ == other.ptr_) ) {
			return *this;
		}
		
		if ( ptr_ ) {
			release();
		}
		
		ptr_       = other.ptr_;
		deleter_   = other.deleter_;
		ref_count_ = other.ref_count_;
		add();
		
		return *this;
	}

	template < class U >
	ref(const ref<U> &other)
		: ptr_( other.ptr_ )
		, deleter_( other.deleter_ )
		, ref_count_( other.ref_count_ )
	{
		if (ptr_) {
			ref_count_ = other.ref_count_;
			add(); 
		}
	}

	operator T*() const
	{
		return ptr_; 
	}
	
	T * operator->() const
	{
		return ptr_;
	}
	
	bool operator==(const ref<T> &other) const
	{
		return ptr_ == other.ptr_;
	}
	
	bool operator!=(const ref<T>& other) const 
	{
		return ptr_ != other.ptr_;
	}

	operator bool () const
	{
		return ptr_ != 0;
	}

	T *get() const
	{
		return ptr_;
	}
protected:
	void add()
	{
		if (ref_count_) {
			++(*ref_count_);
		}
	}
	
	void release()
	{
		if (ref_count_) {
			--(*ref_count_);
			if ( 0 == (*ref_count_)) {
				if (deleter_) {
					deleter_->doDelete(ptr_);
				} else {
					delete ptr_;
				}
				ptr_ = 0;
				delete ref_count_;
			}
		}
	}

	
protected:
	T *ptr_;
	DeleterBase<T> *deleter_;
	unsigned int *ref_count_;
};


//T of DeleterBase have to be the same as T of ref.
//TODO: Comparison of type
template< typename T >
struct DeleterBase
{
	virtual ~DeleterBase() {}
	virtual void doDelete(T *)=0;
};


//for static object
template<typename T>
struct NullDeleter : public DeleterBase<T>
{
	virtual void doDelete(T *)
	{
	}
};

//uses for ref<void>
template<typename ActualType>
struct CastDeleter : public DeleterBase<void>
{
	virtual void doDelete(void *p)
	{
		ActualType *t = static_cast<ActualType*>(p);
		delete t;
	}
};

#endif

