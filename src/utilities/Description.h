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
	Description(const std::string& name, const std::string& comment);

	const std::string getName() override;

	void setName(const std::string& name) override;

	const std::string getComment() override;

	void setComment(const std::string& comment) override;
private:
	std::string m_name;
	std::string m_comment;
};