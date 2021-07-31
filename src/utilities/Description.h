#pragma once
#include <string>

class IDescription
{
public:
	virtual const char* getName() = 0;

	virtual void setName(const std::string& name) = 0;

	virtual const char* getComment() = 0;

	virtual void setComment(const std::string& comment) = 0;
};

class Description : virtual public IDescription
{
public:
	Description(const std::string& name, const std::string& comment);

	const char* getName() override;

	void setName(const std::string& name) override;

	const char* getComment() override;

	void setComment(const std::string& comment) override;
private:
	std::string m_name;
	std::string m_comment;
};