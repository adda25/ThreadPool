#ifndef _THREAD_POOL_TEST_HPP_
#define _THREAD_POOL_TEST_HPP_
#ifdef __cplusplus

#include <chrono>
#include <iostream>
#include <stdlib.h>     
#include <time.h>     
#include "threadpool.hpp"

using namespace std::chrono;
using namespace astp;

/**
*   Temp solution for testing
*   ThreadPool waiting
*   UnitCpp tests.
*/
class ThreadPoolTest
{
public:
    
    ThreadPoolTest() {
        srand(time(NULL));
    };
    ~ThreadPoolTest() {};

    void 
    exc_all() {
        create(ThreadPoolTest::random(-100, 1000));
        resize(ThreadPoolTest::random(1, 10));
        push(ThreadPoolTest::random(0, 1000000));
        varidic_push(ThreadPoolTest::random(0, 1000000));
        set_sleep_time(ThreadPoolTest::random(-100, 1000000000));
        do_job(ThreadPoolTest::random(0, 10000));
        multithreading_access();
        multithreading_access_push();
        dispatch_group();
        dispatch_group_multith();
    }

    void
    create(int n) {
        auto start = start_func(__func__, n);
        auto tp = ThreadPool(n);
        assert(tp.pool_size() == abs(n) || tp.pool_size() == 1);
        end_func(start);
    } 

    void 
    resize(int new_size) {
        auto start = start_func(__func__, new_size);
        auto tp = ThreadPool();
        tp.resize(new_size);
        assert(tp.pool_size() == new_size || tp.pool_size() == 1);
        end_func(start);
    }

    void 
    push(int iterations) {
        auto start = start_func(__func__, iterations);
        auto tp = ThreadPool();
        tp.stop();
        for (int i = 0; i < iterations; i++) {
            tp.push([i] () { int a = i * 64; });    
        }
        assert(tp.queue_size() == iterations);
        end_func(start);
    }

    void
    varidic_push(int iterations) {
        auto start = start_func(__func__, iterations);
        auto tp = ThreadPool();
        tp.stop();
        for (int i = 0; i < iterations; i++) {
            tp.push([i] () { int a = i * 64; }, [i] () { int a = i * 128; } );    
        }
        assert(tp.queue_size() == 2 * iterations);
        end_func(start);
    }

    void 
    set_sleep_time(int n) {
        auto start = start_func(__func__, n);
        auto tp = ThreadPool();
        tp.set_sleep_time_ns(n);
        assert(tp.sleep_time_ns() == abs(n));
        end_func(start);
    }

    void
    do_job(int jobs_num) {
        auto start = start_func(__func__, jobs_num);
        std::cout << "Warning: this could take some time" << std::endl;
        auto tp = ThreadPool();
        for (int i = 0; i < jobs_num; i++) {
            tp.push([&] () { 
                int max = ThreadPoolTest::random(1, 100000);
                std::vector<int> vec;
                for (int i = 0; i < max; i++) {
                    vec.push_back(ThreadPoolTest::random(1, 1000));
                }
             });    
        }
        tp.wait();
        assert(tp.queue_size() == 0);
        end_func(start);
    }

    void
    multithreading_access() {
        auto start = start_func(__func__, "noargs");
        auto tp = ThreadPool();
        auto acc_thread = std::vector<std::thread>(3);
        for (int i = 0; i < 10; i++) {
            acc_thread[0] = std::thread([&tp] () {
                tp.resize(ThreadPoolTest::random(1, 100));  
            });
            acc_thread[1] = std::thread([&tp] () {
                tp.resize(ThreadPoolTest::random(1, 10)); 
            });
            acc_thread[2] = std::thread([&tp] () {
                tp.wait(); 
            });
            for (int i = 0; i < acc_thread.size(); i++) {
                acc_thread[i].join();
            }
            tp.wait();
        }
        end_func(start);
    }

    void
    multithreading_access_push() {
        auto start = start_func(__func__, "noargs");
        auto tp = ThreadPool();
        auto acc_thread = std::vector<std::thread>(3);
        for (int i = 0; i < 2000; i++) {
            acc_thread[0] = std::thread([&tp] () {
                tp.push([](){ThreadPoolTest::random(1, 1000);});  
            });
            acc_thread[1] = std::thread([&tp] () {
                tp.push([](){ThreadPoolTest::random(1, 1000);}); 
            });
            acc_thread[2] = std::thread([&tp] () {
                tp.push([](){}); 
            });
            for (int i = 0; i < acc_thread.size(); i++) {
                acc_thread[i].join();
            }
            tp.wait();
        }
        end_func(start);
    }

    void
    dispatch_group() {
        auto start = start_func(__func__, "noargs");
        auto tp = ThreadPool();
        tp.dispatch_group_enter("group1");
        for (int i = 0; i < 100; i++) {
            tp.dispatch_group_insert("group1", [i]() { auto a = i * 2; });
        }
        tp.dispatch_group_leave("group1");    
        tp.dispatch_group_wait("group1");
        end_func(start);
    }

    void
    dispatch_group_multith() {
        auto start = start_func(__func__, "noargs");
        auto tp = ThreadPool();
        auto acc_thread = std::vector<std::thread>(3);
        acc_thread[0] = std::thread([&tp] () {
            tp.dispatch_group_enter("group1");
            for (int i = 0; i < 1000; i++) {
                tp.dispatch_group_insert("group1", [i]() { auto a = i * 2; });
            }
        });
        acc_thread[1] = std::thread([&tp] () {
            for (int i = 0; i < 1000; i++) {
                tp.dispatch_group_enter("group1");
            }
            tp.dispatch_group_leave("group1");  
            tp.dispatch_group_wait("group1");
        });
        acc_thread[2] = std::thread([&tp] () {
            tp.dispatch_group_enter("group2");
            for (int i = 0; i < 100; i++) {
                tp.dispatch_group_insert("group2", [i]() { auto a = i * 2; });
            }
            tp.dispatch_group_leave("group2");    
            tp.dispatch_group_wait("group2");
        });
        for (int i = 0; i < acc_thread.size(); i++) {
            acc_thread[i].join();
        }
        end_func(start);
    }

    static int
    random(int min, int max) {
        return rand() % max + min;
    }


private:

    template<class ...Args>
    std::pair<std::string, high_resolution_clock::time_point>
    start_func(std::string test, Args... args) {
        std::cout << "\nStart test: " << test << " with args: ";
        print(args...);
        std::cout << std::endl;
        return std::make_pair(test, high_resolution_clock::now());
    }

    template<class T> void 
    print(T p) { 
        std::cout << p << " ";
    }

    void
    end_func(std::pair<std::string, high_resolution_clock::time_point> test) {
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(t2 - test.second);
        std::cout << "End test: " << test.first << " in " << time_span.count() << " seconds" << std::endl;
    }

}; // End ThreadPoolTest

#endif // __cplusplus
#endif // _THREAD_POOL_TEST_HPP_
