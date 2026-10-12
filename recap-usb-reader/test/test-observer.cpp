#include "observable.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

using recap::app::observable::Observable;
using TestObservable = Observable<int>;

class Callback {
private:
    int m_total = 0;
    unsigned int m_called = 0;

public:
    [[nodiscard]] int total() const { return m_total; }

    [[nodiscard]] unsigned int called() const { return m_called; }

    void operator()(int i)
    {
        m_total += i;
        m_called += 1;
    }
};

TEST(TestObservable, TestNewObservableHasZeroObservers)
{
    TestObservable obs;
    EXPECT_EQ(obs.num_observers(), 0);
}

TEST(TestObservable, TestSubscribeIncreasesObserverCount)
{
    TestObservable obs;
    auto sub1 = obs.subscribe([](auto) {});
    EXPECT_EQ(obs.num_observers(), 1);
}

TEST(TestObservable, TestUnsubscribeDecreasesObserverCount)
{
    TestObservable obs;
    {
        auto sub1 = obs.subscribe([](auto) {});
        {
            auto sub2 = obs.subscribe([](auto) {});
            EXPECT_EQ(obs.num_observers(), 2);
        }
        EXPECT_EQ(obs.num_observers(), 1);
    }
    EXPECT_EQ(obs.num_observers(), 0);
}

TEST(TestObservable, TestNotifyCallsObserver)
{
    TestObservable obs;
    Callback c;

    obs.notify(10);

    auto sub1 = obs.subscribe([&c](int i) { c(i); });

    obs.notify(5);
    EXPECT_EQ(c.total(), 5);
    EXPECT_EQ(c.called(), 1);

    obs.notify(-10);
    EXPECT_EQ(c.total(), -5);
    EXPECT_EQ(c.called(), 2);
}

TEST(TestObservable, TestNotifyCallsMultipleObservers)
{
    TestObservable obs;
    Callback c1;
    Callback c2;

    obs.notify(10);

    auto sub1 = obs.subscribe([&c1](int i) { c1(i); });
    obs.notify(5);
    EXPECT_EQ(c1.total(), 5);
    EXPECT_EQ(c1.called(), 1);

    {
        auto sub2 = obs.subscribe([&c2](int i) { c2(i); });
        obs.notify(-10);
        EXPECT_EQ(c1.total(), -5);
        EXPECT_EQ(c1.called(), 2);
        EXPECT_EQ(c2.total(), -10);
        EXPECT_EQ(c2.called(), 1);
    }

    obs.notify(20);
    EXPECT_EQ(c1.total(), 15);
    EXPECT_EQ(c1.called(), 3);
    EXPECT_EQ(c2.total(), -10);
    EXPECT_EQ(c2.called(), 1);
}

TEST(TestObservable, TestObserverOutlivesObservable)
{
    TestObservable *obs = new TestObservable();
    Callback cb;
    auto sub = obs->subscribe([&cb](int i) { cb(i); });
    obs->notify(10);
    EXPECT_EQ(cb.called(), 1);
    EXPECT_EQ(cb.total(), 10);
    delete obs;
    EXPECT_EQ(cb.called(), 1);
    EXPECT_EQ(cb.total(), 10);
}

TEST(TestObservable, TestRvaluesAreNotMoved)
{
    Callback cb;
    auto observer_callback = [&](std::vector<int> v) {
        cb(static_cast<int>(v.size()));
        v.clear();
    };

    Observable<std::vector<int>> observable;
    auto obs1 = observable.subscribe(observer_callback);
    auto obs2 = observable.subscribe(observer_callback);

    // observer_callback takes a copy of the vector and clears it, but notify takes a forwarding
    // reference. Test that the rvalue passed to notify is not cleared in the second call to
    // observer_callback. (Effectively we are checking that std::forward is not called twice on the
    // args.)
    observable.notify(std::vector<int>{1, 2, 3});
    EXPECT_EQ(cb.called(), 2);
    EXPECT_EQ(cb.total(), 6);
}

class SelfUnsubscribingObserver {
    class Container {
        SelfUnsubscribingObserver& m_parent;
        Observable<>::Observer m_sub;

    public:
        explicit Container(SelfUnsubscribingObserver& parent) :
            m_parent(parent),
            m_sub(parent.m_observable.subscribe([this]() { m_parent.m_container.reset(); }))
        {}
    };

    Observable<>& m_observable;
    std::unique_ptr<Container> m_container;

public:
    explicit SelfUnsubscribingObserver(Observable<>& observable) :
        m_observable(observable), m_container(std::make_unique<Container>(*this))
    {}

    [[nodiscard]] bool is_reset() const { return m_container == nullptr; }
};

TEST(TestObservable, TestUnsubscribeDuringNotification)

{
    Observable<> observable;
    SelfUnsubscribingObserver observer(observable);
    ASSERT_FALSE(observer.is_reset());
    observable.notify();
    ASSERT_TRUE(observer.is_reset());
}
