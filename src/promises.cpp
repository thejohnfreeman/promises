#include <exception>
#include <memory>
#include <promises/promises.hpp>

#include <cstdio>
#include <vector>

int main(int argc, const char** argv) {
    using namespace promises;

    using promise_type = AsyncPromise<int>;
    using value_type = promise_type::value_type;
    using error_type = promise_type::error_type;

    std::printf("sizeof(callback) == %lu\n",
            sizeof(promise_type::callback_type));
    std::printf("sizeof(callbacks_) == %lu\n",
            sizeof(std::vector<std::function<void()>>));
    std::printf("sizeof(value_) == %lu\n", sizeof(value_type));
    std::printf("sizeof(error_) == %lu\n", sizeof(error_type));
    std::printf("sizeof(state_) == %lu\n", sizeof(std::atomic<State>));
    std::printf("sizeof(enable_shared_from_this) == %lu\n",
            sizeof(std::enable_shared_from_this<void>));
    std::printf("sizeof(promise) == %lu\n", sizeof(promise_type));

    {
        auto p1 = promise_type::pending();
        p1->subscribe([](auto p){
            if (p->state() == FULFILLED) {
                std::printf("value == %d\n", p->value());
            } else {
                std::printf("expected a value");
            }
        });
        auto sch = SingleThreadedScheduler::dflt();
        sch->schedule([&](){ p1->fulfill(42); });
        sch->run();
    }

    {
        auto p1 = promise_type::fulfilled('c');
        p1->subscribe([](auto p){
            if (p->state() == FULFILLED) {
                std::printf("value == %d\n", p->value());
            } else {
                std::printf("expected a value");
            }
        });
        auto sch = SingleThreadedScheduler::dflt();
        sch->run();
    }

    {
        auto e = std::runtime_error("hello, world!");
        std::printf("error == %s\n", e.what());
        auto p1 = promise_type::rejected(e);
        p1->subscribe([](auto p){
            if (p->state() == REJECTED) {
                try {
                    std::rethrow_exception(p->error());
                } catch (std::exception const& error) {
                    std::printf("error == %s\n", error.what());
                }
            } else {
                std::printf("expected an error");
            }
        });
        auto sch = SingleThreadedScheduler::dflt();
        sch->run();
    }

    {
        auto p1 = promise_type::pending();
        auto p2 = p1->then([](auto p){
            assert(p->state() == FULFILLED);
            return p->value() + 1;
        });
        auto p3 = p2->then([](auto p){
            assert(p->state() == FULFILLED);
            std::printf("value == %d\n", p->value());
        });
        auto sch = SingleThreadedScheduler::dflt();
        sch->schedule([&](){ p1->fulfill(42); });
        sch->run();
    }

    {
        std::vector<std::shared_ptr<void>> ptrs;
        {
            auto p1 = AsyncPromise<int>::fulfilled(42);
            ptrs.push_back(p1);
        }
        {
            auto p2 = AsyncPromise<double>::fulfilled(3.14);
            ptrs.push_back(p2);
        }
        auto p3 = std::static_pointer_cast<AsyncPromise<int>>(ptrs[0]);
        std::printf("value == %d\n", p3->value());
        auto p4 = std::static_pointer_cast<AsyncPromise<double>>(ptrs[1]);
        std::printf("value == %f\n", p4->value());
    }

    return 0;
}
