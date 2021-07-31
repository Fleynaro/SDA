#pragma once
#include <string>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;


namespace Helper
{
	namespace Algorithm
	{
		template<typename T>
		static bool BinarySearch(const std::vector<T>& vector, T value, int& index) {

			int lower = 0;
			int upper = static_cast<int>(vector.size()) - 1;
			while (lower <= upper) {
				auto mid = (upper + lower) / 2;
				const auto mid_value = vector[mid];
				if (value == mid_value) {
					index = mid;
					return true;
				}
				if (value < mid_value)
					upper = mid - 1;
				else
					lower = mid + 1;
			}

			return false;
		}
	};
	
	namespace File
	{
		// exceptions
		class FileException : public std::exception {
		public: FileException(const char* message);
		};

		extern void LoadFileIntoBuffer(const fs::path& file, char** buffer, int* size);

		extern void SaveBufferIntoFile(char* buffer, int size, const fs::path& file);
	};

	namespace String
	{
		extern std::vector<std::string> Split(const std::string& input, const std::string& regex);
		extern void Replace(std::string& source, const std::string& from, const std::string& to);
		
		extern std::string ToLower(std::string source);
		extern std::string ToUpper(std::string source);

		extern bool Contains(const std::string& str, const std::string& keyword);

		extern uint64_t HexToNumber(std::string source);
		extern std::string NumberToHex(uint64_t number);

		extern bool is_number(const std::string& s);

		std::wstring s2ws(const std::string& str);
		std::string ws2s(const std::wstring& wstr);

		static inline void ltrim(std::string& s);
		static inline void rtrim(std::string& s);

		extern void replaceSymbolWithin(std::string& source, const char bounder[2], const char from, const char to = ' ');

		std::string format(const std::string fmt_str, ...);
	};

	namespace Date
	{
		namespace View {
			constexpr const char* Full = "%Y-%m-%d %H:%M:%S";
			constexpr const char* Date = "%Y-%m-%d";
			constexpr const char* Time = "%H:%M:%S";
		};

		extern std::string format(std::chrono::system_clock::time_point time_point, std::string format = View::Full);
	};
};

