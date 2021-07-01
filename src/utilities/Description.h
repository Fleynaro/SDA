#pragma once
#include <string>

class IDescription
{
public:
	virtual const std::string getName() = 0;

	virtual void setName(const std::string& name) = 0;

	virtual const std::string getComment() = 0;

	virtual void setComment(const std::string& comment) = 0;
};

class Description : virtual public IDescription
{
public:
	Description(const std::string& name, const std::string& comment)
		: m_name(name), m_comment(comment)
	{}

	const std::string getName() override {
		return m_name;
	}

	void setName(const std::string& name) override {
		m_name = name;
	}

	const std::string getComment() override {
		return m_comment;
	}

	void setComment(const std::string& comment) override {
		m_comment = comment;
	}
private:
	std::string m_name;
	std::string m_comment;
};