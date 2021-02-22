#ifndef CRUST_RAII_CHECKER_HPP
#define CRUST_RAII_CHECKER_HPP


#include <unordered_map>
#include <memory>

#include "utility.hpp"


namespace test {
class RAIIRecorder {
private:
    struct Record {
    };

    std::unordered_map<void *, Record> record;

public:
    RAIIRecorder() noexcept: record{} {}

    void construct(void *self) {
        CRUST_ASSERT(record.find(self) == record.end());
        record.emplace(self, Record{});
    }

    void deconstruct(void *self) {
        auto ptr = record.find(self);
        CRUST_ASSERT(ptr != record.end());
        record.erase(ptr);
    }

    ~RAIIRecorder() { CRUST_ASSERT(record.empty()); }
};

class RAIIChecker {
private:
    std::shared_ptr<RAIIRecorder> recorder;

public:
    explicit RAIIChecker(std::shared_ptr<RAIIRecorder> recorder) noexcept:
            recorder{std::move(recorder)} {
        this->recorder->construct(this);
    }

    RAIIChecker(const RAIIChecker &other) noexcept: recorder{other.recorder} {
        this->recorder->construct(this);
    }

    RAIIChecker(RAIIChecker &&other) noexcept: recorder{other.recorder} {
        this->recorder->construct(this);
    }

    RAIIChecker &operator=(const RAIIChecker &other) {
        CRUST_ASSERT(recorder == other.recorder);
        return *this;
    }

    RAIIChecker &operator=(RAIIChecker &&other) {
        CRUST_ASSERT(recorder == other.recorder);
        return *this;
    }

    ~RAIIChecker() { recorder->deconstruct(this); }
};
}


#endif //CRUST_RAII_CHECKER_HPP
