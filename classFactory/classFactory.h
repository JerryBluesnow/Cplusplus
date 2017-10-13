struct factory
{
    static factory& get()
    {
        static factory instance;
        return instance;
    }
private:
    factory() {};
    factory(const factory&) = delete;
    factory(factory&&) = delete;
    static std::map<std::string, std::function<Message*()>> map_; 
};