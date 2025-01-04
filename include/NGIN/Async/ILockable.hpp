#pragma once

namespace NGIN::Async
{
    class ILockable
    {
    public:
        virtual ~ILockable()                          = default;
        virtual void Lock() noexcept                  = 0;
        virtual void Unlock() noexcept                = 0;
        [[nodiscard]] virtual bool TryLock() noexcept = 0;
    };
}// namespace NGIN::Async