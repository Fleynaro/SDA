
class WarningException : public std::exception {
public: WarningException(const std::string& message) : std::exception(message.c_str()) {}
};

class NotFoundException : public WarningException {
public: NotFoundException(const std::string& message) : WarningException(message) {}
};