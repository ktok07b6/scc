namespace {}

namespace {
	int a;
}

namespace X {
	class A {};
	namespace XX {
	}
}

namespace X2 = X::XX;

using namespace X;

using ::global;
using X::A;

void f() {
namespace X2 = X::XX;

using namespace X;

using ::global;
using X::A;
}
