#ifndef RECAP_LABELLER_OBSERVABLE_HPP_INCLUDE
#define RECAP_LABELLER_OBSERVABLE_HPP_INCLUDE

#include <functional>
#include <list>
#include <memory>
#include <utility>
#include <vector>

namespace recap::app::observable {

template <typename... Ts> class Observable {
public:
    class Observer;
    using Function = std::function<void(Ts...)>;
    using ObserverList = std::list<Function>;
    using ObserverHandle = ObserverList::iterator;
    using UnsubscribedList = std::vector<ObserverHandle>;

    Observable() : m_unsubscribed(std::make_shared<UnsubscribedList>()) {}

    class Observer {
    private:
        std::weak_ptr<UnsubscribedList> unsubscribed_list_ptr;
        ObserverHandle handle;

        Observer(const std::shared_ptr<UnsubscribedList>& unsubscribed, ObserverHandle handle) :
            unsubscribed_list_ptr(unsubscribed), handle(handle)
        {}

        friend class Observable;

    public:
        ~Observer()
        {
            std::shared_ptr<UnsubscribedList> list = unsubscribed_list_ptr.lock();
            if (list) {
                list->push_back(handle);
            }
        }

        Observer(const Observer&) = delete;
        Observer& operator=(const Observer&) = delete;
        Observer(Observer&&) = delete;
        Observer& operator=(Observer&&) = delete;
    };

    [[nodiscard]] typename ObserverList::size_type num_observers() const
    {
        remove_unsubscribed();
        return m_observers.size();
    }

    template <typename... Args> void notify(Args&&...args) const
    {
        remove_unsubscribed();
        for (const auto& observer : m_observers) {
            observer(args...);
        }
    }

    [[nodiscard]] Observer subscribe(Function function)
    {
        remove_unsubscribed();
        m_observers.emplace_back(std::move(function));
        return Observer(m_unsubscribed, std::prev(m_observers.end()));
    }

private:
    mutable ObserverList m_observers;
    mutable std::shared_ptr<UnsubscribedList> m_unsubscribed;

    void remove_unsubscribed() const
    {
        for (auto& handle : *m_unsubscribed) {
            m_observers.erase(handle);
        }
        m_unsubscribed->clear();
    }
};

template <typename... Ts> using Observer = typename Observable<Ts...>::Observer;

}; // namespace recap::app::observable

#endif // RECAP_LABELLER_OBSERVABLE_HPP_INCLUDE
