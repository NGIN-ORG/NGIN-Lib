#pragma once

namespace NGIN::Containers
{
    template <typename T>
    class Vector
    {
    public:
        Vector();
        Vector(const Vector& other);
        Vector(Vector&& other) noexcept;
        ~Vector();

        Vector& operator=(const Vector& other);
        Vector& operator=(Vector&& other) noexcept;

        void PushBack(const T& value);
        void PushBack(T&& value);

        template <typename... Args>
        void EmplaceBack(Args&&... args);

        void PopBack();

        void Erase(std::size_t index);

        void Clear();





    private:

    };
}