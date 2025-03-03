#include "StringHash.h"



StringHash::StringHash(const std::string & content)
{
	m_content = content;
	m_hash = sgs_HashString(content.c_str());
}

StringHash::StringHash(const StringHash& other)
{
	m_content = other.getString();
	m_hash = other.m_hash;
}


sgs_Hash StringHash::getHash() const
{
	return m_hash;
}



size_t StringHash::getComplexHash() const
{
	return std::hash<std::string>{}(m_content);
}



const std::string & StringHash::getString() const
{
	return m_content;
}


bool StringHash::equals(const sgs_Variable& var) const
{
	return var.type == SGS_VT_STRING && m_hash == var.data.S->hash && strcmp(sgs_var_cstr(&var), m_content.c_str()) == 0;
}

