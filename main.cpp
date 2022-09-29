#include <iostream>
#include <vector>
#include <future>

#include "LockFreeStack.h"

LockFreeStack<int> lfstack{ 100 };

struct St {
    enum  Type { Push, Pop };
    Type type;
    int value;
    int thread;

    St(int thread_,  int value_, Type type_)
        : type(type_), value(value_),  thread(thread_) {}
};

std::vector<St> Func(int id, int minValue, int delay) {
    std::vector<St> result;

    for (int i = 0; i < 10; i++) {

        int temp = minValue + i;
        lfstack.Push(temp);
        result.emplace_back(id, temp, St::Type::Push);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    for (int i = 0; i < 10; i++) {
        if (!lfstack.IsEmpty()) {
            int temp = lfstack.Pop();
            result.emplace_back(id, temp, St::Type::Pop);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    return result;
}

int main() {

    std::future<std::vector<St>> future1 = std::async(Func, 1, 15, 350);
    std::future<std::vector<St>> future2 = std::async(Func, 2, 30, 150);

    std::vector<St> res1 = future1.get();
    std::vector<St> res2 = future2.get();

    std::vector<St> res;
    res.insert(res.begin(), res1.begin(), res1.end());
    res.insert(res.end(), res2.begin(), res2.end());

    for (int i = 0; i < res.size(); ++i) {
        std::cout << "THREAD: " << res[i].thread << " TYPE: " << res[i].type << " VALUE: " << res[i].value << std::endl;
    }
}
