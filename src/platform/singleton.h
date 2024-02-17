#pragma once

template <class T>
class Singleton {
    public:
        Singleton() = default;
        Singleton(const Singleton &other) = delete;
        Singleton(Singleton &&other) = delete;
        Singleton &operator=(const Singleton &other) = delete;

        static T* GetInstance() 
        {
            static T* instance = nullptr;
            if (instance == nullptr) {
                instance = new T();
            }
            return instance;
        }
};
