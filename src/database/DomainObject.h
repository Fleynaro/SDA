#pragma once

namespace DB
{
	using Id = int;

	class IMapper;
	class IDomainObject
	{
	public:
		virtual ~IDomainObject() {}
		virtual Id getId() = 0;
		virtual void setId(Id id) {}
		virtual bool isInDB() = 0;
		virtual IMapper* getMapper() { return nullptr; }
		virtual void setMapper(IMapper* mapper) {}
	};

	class DomainObject : virtual public IDomainObject
	{
	public:
		DomainObject(Id id = 0);

		Id getId() override;

		void setId(Id id) override;

		bool isInDB() override;

		IMapper* getMapper() override;

		void setMapper(IMapper* mapper) override;
	private:
		Id m_id;
		IMapper* m_mapper = nullptr;
	};
};