//class

class C1 {};

class C2 {
public:
		int x;
};

class C3 {
public:
		int x1;
		int x2;
protected:
		int y1;
		int y2;
private:
		int z1;
		int z2;
protected:
		int z3;
};


class B{};
namespace zzz{
class C4 : ::B {
public:
    int f() { return 100;}
	int g(), h;
};
}

class C5 : public B, virtual public zzz::C4{
public:
	C5();
	
	C5(int z) 
		: B(), zzz::C4(), a(), b(0), c(f())
	{
	}
	int a;
	int b;
	int c;
	int z() {return 0;}
};

class C6 {
	bool operator<(const int &o) const {
		return true;
	}
	void operator +(const C6 &c) { 
		//::operator+(1, 2); 
	} 
	operator int() {}
};


class C7 {
public:
	class C8;
	struct X {};
}C7;
class C7 c7;


class C7::C8 {
public:
	C7::X x;
	virtual void f0()= 0;
	static const int Z = 100;

	typedef int INT;	
	inline const INT f1() {return 1;}

	typedef bool (C8::*MemFun)();
	friend class C6;
};

class C9 : private C7::C8 {
public:
	C7::C8::Z;
	using typename C7::C8::INT;
};



