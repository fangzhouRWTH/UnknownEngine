//#include "core/handles.hpp"
//#include "memory/resource.hpp"
#include "catch2/catch_test_macros.hpp"
#include "utils/container.hpp"
#include "utils/handle.hpp"

using namespace unknown;

// class HANDLE_NAME : public HandleBase<HANDLE_NAME, Recycle<HANDLE_NAME>> {
//   friend class ArenaVectorContainer<HANDLE_NAME, u32>;
//   friend class HandleBase<HANDLE_NAME, Recycle<HANDLE_NAME>>;
// };

DECL_CONTAINER_HANDLE(BufferHandle, Recycle, VectorContainer, u32);
// DECL_HANDLE(TextureHandle, Recycle); DECL_HANDLE(ResourceHandle,
// NoRecycle);

TEST_CASE("HANDLE", "[standard]") {

  VectorContainer<BufferHandle, u32> container;
  auto &c = container.getContainer();

  auto h1 = container.push(12392);
  REQUIRE(c[0].first == true);
  auto h2 = container.push(345345);
  REQUIRE(c[1].first == true);
  container.release(h1);
  REQUIRE(c[0].first == false);
  auto h3 = container.push(345346);
  REQUIRE(c[0].first == true);

  REQUIRE(h1.get() == 0);
  REQUIRE(h2.get() == 1);
  REQUIRE(h3.get() == 0);
  REQUIRE(container.get(h3) == 345346);
  REQUIRE(container.get(h1) == 345346);

  container.reset();

  for (auto i : c) {
    REQUIRE(i.first == false);
  }

  container.resize(100);
  REQUIRE(c.size() == 100);

  for (auto i : c) {
    REQUIRE(i.first == false);
  }

  // BufferHandle bh0 = BufferHandle::create();
  // BufferHandle bh1 = BufferHandle::create();
  // BufferHandle bh2 = BufferHandle::create();
  // BufferHandle bh3 = BufferHandle::create();
  // bh3.release();
  // BufferHandle bh4 = BufferHandle::create();
  // BufferHandle bh5 = BufferHandle::create();

  // TextureHandle th0 = TextureHandle::create();
  // TextureHandle th1 = TextureHandle::create();
  // TextureHandle th2 = TextureHandle::create();
  // TextureHandle th3 = TextureHandle::create();

  // ResourceHandle rh0 = ResourceHandle::create();
  // ResourceHandle rh1 = ResourceHandle::create();
  // rh1.release();
  // ResourceHandle rh2 = ResourceHandle::create();

  // REQUIRE(bh0.get() == 0);
  // REQUIRE(bh1.get() == 1);
  // REQUIRE(bh2.get() == 2);
  // REQUIRE(bh3.get() == 3);
  // REQUIRE(bh4.get() == 3);
  // REQUIRE(bh5.get() == 4);

  // REQUIRE(th0.get() == 0);
  // REQUIRE(th1.get() == 1);
  // REQUIRE(th2.get() == 2);
  // REQUIRE(th3.get() == 3);

  // REQUIRE(rh0.get() == 0);
  // REQUIRE(rh1.get() == 1);
  // REQUIRE(rh2.get() == 2);
}
