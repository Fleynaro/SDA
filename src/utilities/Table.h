#pragma once
#include <main.h>

namespace Utils
{
	template<int PrimaryKeyColumnIdx, typename ...ColumnsType>
	class Table
	{
	public:
		using Tuple = std::tuple<ColumnsType...>;
		using PrimaryKeyColumnType = typename std::tuple_element<PrimaryKeyColumnIdx, Tuple>::type;

		Table() = default;

		void addRow(ColumnsType... values) {
			auto tuple = Tuple(values...);
			m_rows.insert(std::make_pair(std::get<PrimaryKeyColumnIdx>(tuple), tuple));
		}

		bool hasRow(PrimaryKeyColumnType key) {
			return m_rows.find(key) != m_rows.end();
		}

		Tuple* getRow(PrimaryKeyColumnType key) {
			return hasRow(key) ? &m_rows[key] : nullptr;
		}

		void removeRow(PrimaryKeyColumnType key) {
			m_rows.erase(key);
		}

		int size() {
			return (int)m_rows.size();
		}

		void clear() {
			m_rows.clear();
			onClear();
		}

		virtual void onClear() {}

		Tuple* operator [] (PrimaryKeyColumnType key) {
			return getRow(key);
		}

		class Result
		{
		public:
			Result(Table* table)
				: m_table(table)
			{}

			template<int SortColumnIdx>
			Result& orderBy(bool descending = false) {
				return descending ? orderBy<SortColumnIdx, true>() : orderBy<SortColumnIdx, false>();
			}

			template<int SortColumnIdx, bool Descending = false>
			Result& orderBy() {
				return orderBy([](const Tuple& a, const Tuple& b) {
					if constexpr (Descending)
						return std::get<SortColumnIdx>(a) > std::get<SortColumnIdx>(b);
					else return std::get<SortColumnIdx>(a) < std::get<SortColumnIdx>(b);
					});
			}

			Result& orderBy(const std::function<bool(const Tuple&, const Tuple&)>& functor) {
				m_rowsIndexes.sort([&](const PrimaryKeyColumnType& idx1, const PrimaryKeyColumnType& idx2) {
					return functor(*m_table->getRow(idx1), *m_table->getRow(idx2));
					});
				return *this;
			}

			std::list<PrimaryKeyColumnType>& getList() {
				return m_rowsIndexes;
			}
		private:
			Table* m_table;
			std::list<PrimaryKeyColumnType> m_rowsIndexes;
		};

		Result where(const std::function<bool(const Tuple&)>& filter) {
			Result result(this);
			for (auto it : m_rows) {
				if (filter(it.second)) {
					result.getList().push_back(it.first);
				}
			}
			return result;
		}

		Result all() {
			return where([](const Tuple& tuple) { return true; });
		}
	private:
		std::map<PrimaryKeyColumnType, Tuple> m_rows;
	};
};