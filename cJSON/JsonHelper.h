#ifndef JSONHELPH
#define JSONHELPH

#include <QString>
#include <memory>
#include "cJSON.h"

static QString jsonToString(cJSON* json, bool formatted)
{
	std::unique_ptr<char[]> text(formatted ? cJSON_Print(json) : cJSON_PrintUnformatted(json));
	return text.get();
}

static double jsonGetChildDouble(cJSON* json, const char* name, double def = 0.0)
{
	cJSON* child = cJSON_GetObjectItem(json, name);
	if (child != nullptr && child->type == cJSON_Number)
		return (float)child->valuedouble;
	return def;
}

static int jsonGetChildInt(cJSON* json, const char* name, int def = 0)
{
	cJSON* child = cJSON_GetObjectItem(json, name);
	if (child != nullptr && child->type == cJSON_Number)
		return child->valueint;
	return def;
}

static bool jsonGetChildBool(cJSON* json, const char* name, bool def = false)
{
	cJSON* child = cJSON_GetObjectItem(json, name);
	if (child != nullptr)
	{
		if (child->type == cJSON_True)
			return true;
		else if (child->type == cJSON_False)
			return false;
	}
	return def;
}

static QString jsonGetChildString(cJSON* json, const char* name, const QString& def = "")
{
	cJSON* child = cJSON_GetObjectItem(json, name);
	if (child != nullptr && child->type == cJSON_String)
		return child->valuestring;
	return def;
}




///////

static double jsonGetArrayDouble(cJSON* json, int index, double def = 0.0)
{
	cJSON* child = cJSON_GetArrayItem(json, index);
	if (child != nullptr && child->type == cJSON_Number)
		return (float)child->valuedouble;
	return def;
}

static int jsonGetArrayInt(cJSON* json, int index, int def = 0)
{
	cJSON* child = cJSON_GetArrayItem(json, index);
	if (child != nullptr && child->type == cJSON_Number)
		return child->valueint;
	return def;
}

static bool jsonGetArrayBool(cJSON* json, int index, bool def = false)
{
	cJSON* child = cJSON_GetArrayItem(json, index);
	if (child != nullptr)
	{
		if (child->type == cJSON_True)
			return true;
		else if (child->type == cJSON_False)
			return false;
	}
	return def;
}

static QString jsonGetArrayString(cJSON* json, int index, const QString& def)
{
	cJSON* child = cJSON_GetArrayItem(json, index);
	if (child != nullptr && child->type == cJSON_String)
		return child->valuestring;
	return def;
}

#endif
