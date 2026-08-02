#include "KF.hpp"
namespace KF
{
    namespace KTIMER
    {
        KTimer::KTimer(std::string name, KTimeUnit unit)
            : name_(std::move(name)), unit_(unit)
        {
        }

        void KTimer::start()
        {
            if (!running_)
            {
                start_point_ = Clock::now();
                running_ = true;
            }
        }

        void KTimer::pause()
        {
            if (running_)
            {
                auto now = Clock::now();
                accumulated_ += now - start_point_;
                running_ = false;
            }
        }

        void KTimer::clear()
        {
            accumulated_ = std::chrono::nanoseconds{0};
            running_ = false;
        }

        void KTimer::setUnit(KTimeUnit u)
        {
            unit_ = u;
        }

        void KTimer::setName(std::string n)
        {
            name_ = std::move(n);
        }

        double KTimer::getTime() const
        {
            std::chrono::nanoseconds total = accumulated_;
            if (running_)
            {
                auto now = Clock::now();
                total += now - start_point_;
            }

            switch (unit_)
            {
            case KTimeUnit::us:
                return std::chrono::duration<double, std::micro>(total).count();
            case KTimeUnit::ms:
                return std::chrono::duration<double, std::milli>(total).count();
            case KTimeUnit::s:
                return std::chrono::duration<double>(total).count();
            default:
                return 0.0;
            }
        }

        void KTimer::print() const
        {
            std::cout << "[ " << name_ << " ] Time: "
                    << getTime() << " " << unitStr() << "\n";
        }

        std::string KTimer::unitStr() const
        {
            switch (unit_)
            {
            case KTimeUnit::us: return "us";
            case KTimeUnit::ms: return "ms";
            case KTimeUnit::s:  return "s";
            default: return "";
            }
        }

        ///@brief  ========== KTimerManager 实现 ==========
        void KTimerManager::create(const std::string& name, KTimeUnit unit)
        {
            if (exists(name))
            {
                timers_.erase(name);
            }
            // 新建计时器
            KTimer t(name, unit);
            t.start(); // 自动开启
            timers_[name] = std::move(t);
        }

        KTimer& KTimerManager::get(const std::string& name)
        {
            if (!exists(name))
            {
                throw std::runtime_error("Timer [" + name + "] not found!");
            }
            return timers_[name];
        }

        bool KTimerManager::exists(const std::string& name)
        {
            return timers_.find(name) != timers_.end();
        }
        void KTimerManager::remove(const std::string& name)
        {
            timers_.erase(name);
        }

    }
}