#ifndef STRINGHASHH
#define STRINGHASHH

#include <string>
#include <string_view>
#include "sgscript/sgs_int.h"



using namespace std::literals;



class StringHash
{
public:
	typedef size_t Hash;




private:
	std::string m_content;
	sgs_Hash m_hash;


public:

	StringHash(const std::string& content = "");
	StringHash(const StringHash& other);

	sgs_Hash getHash() const;
	size_t getComplexHash() const;
	const std::string& getString() const;
	bool equals(const sgs_Variable& var) const;

	size_t length() const { return m_content.length(); }

	bool operator< (const StringHash& a) const {
		return m_hash < a.m_hash;
	}

	bool operator==(const StringHash& other) const
	{
		return m_hash == other.m_hash && getString() == other.getString();
	}

	bool operator< (const sgs_Variable& var) const {
		return m_hash < var.data.S->hash;
	}

	bool operator==(const sgs_Variable& var) const
	{
		return var.type == SGS_VT_STRING && m_hash == var.data.S->hash && strcmp(m_content.c_str(), sgs_var_cstr(&var)) == 0;
	}


public:

	struct Comparer
	{
		using hash_type = std::hash<std::string_view>;
		using is_transparent = void;

		std::size_t operator()(const StringHash& k) const { return k.m_hash; }
		std::size_t operator()(const sgs_Variable& var) const { return var.data.S->hash; }
		std::size_t operator()(const char* str) const { return sgs_HashString(str); }
		std::size_t operator()(std::string_view str) const { return sgs_HashString(str.data()); }
		std::size_t operator()(std::string const& str) const { return sgs_HashString(str.c_str()); }


	};

};



namespace std {

	template <>
	struct hash<StringHash>
	{
		using is_transparent = void;
		std::size_t operator()(const StringHash& k) const
		{
			return k.getHash();
		}

		std::size_t operator()(const sgs_Variable& k) const
		{
			return k.data.S->hash;
		}
	};

}
#endif
