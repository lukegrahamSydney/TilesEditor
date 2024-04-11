#include <unordered_map>
#include <functional>
#include <locale>
#include "GS1Converter.h"
#include <gs1/parse/Parser.hpp>
#include <gs1/parse/Lexer.hpp>
#include <gs1/parse/Source.hpp>
#include <gs1/parse/Diag.hpp>
#include <qdebug>

static auto endsWith = [](std::string const& str, std::string const& suffix) -> bool {
	if (str.length() < suffix.length()) {
		return false;
	}
	return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
	};

static std::string trim(const std::string& s)
{
	static auto _isspace = [](int a) -> bool {
		return a == ' ' || a == '\r' || a == '\n' || a == '\t' || a == '\f' || a == '\v';
	};

	std::string::const_iterator it = s.begin();
	while (it != s.end() && _isspace(*it))
		it++;

	std::string::const_reverse_iterator rit = s.rbegin();
	while (rit.base() != it && _isspace(*rit))
		rit++;

	return std::string(it, rit.base());

}

static std::unordered_map<std::string, std::vector<std::string> > validActions = {
	{"created", {"onCreated"}},
	{"playerenters", {"onPlayerEntersLevel", "P"}},
	{"playerknown", {"onPlayerKnown", "P"}},
	{"playertouchsme", {"onPlayerTouchesMe", "P"}},
	{"playertouchsother", {"onPlayerTouchesOther", "P", "N"}},
	{"playerchats", {"onPlayerChats", "P", "C"}},
	{"timeout", {"onTimeout"}},
	{"washit", {"onWasHit", "P"}},
	{"wasthrown", {"onWasThrown", "P"}},
	{"exploded", {"onWasExploded", "P"}},
	{"waspelt", {"onWasPelt", "P", "N"}},
	{"wasshot", {"onWasShot", "P"}},
	{"weaponfired", {"onWeaponFired", "P"}},
	{"playerlaysitem", {"onPlayerLaysItem", "P", "N"}},
	{"playerendsreading", {"onPlayerEndsReading", "P"}},
	{"compusdied", {"onBaddiesDied"}},
	{"compusdead", {"onBaddiesDied"}}
};

static std::unordered_map<std::string, std::pair<bool, std::string> > identifierReplacements;

static gs1::PrototypeMap cmds = {
	{"addtiledef2", {true, true, false, false}},
	{ "lay", {true}},
	{ "take", {true}},
	{ "hurt", {false}},
	{ "set", {true} },
	{ "play", {true}},
	{ "unset", {true}},
	{ "setgif", {true}},
	{ "setimg", {true}},
	{ "say", {false}},
	{ "say2", {true}},
	{ "tokenize", {true}},
	{ "callnpc", {false, true}},
	{ "removecompus", {}},
	{ "setlevel", {true}},
	{ "setskincolor", {true}},
	{ "setcoatcolor", {true} },
	{ "setsleevecolor", {true} },
	{ "setshoecolor", {true} },
	{ "setbeltcolor", {true} },
	{ "setplayerdir", {true} },
	{ "putcomp", {true, false, false} },
	{ "takeplayercarry", {}},
	{ "dontblocklocal", {}},
	{ "drawoverplayer", {} },
	{ "enableweapons", {} },
	{ "disableweapons", {} },
	{ "shootarrow", {false}},
	{ "shootfireball", {false} },
	{ "shootfireblast", {false} },
	{ "shootnuke", {false} },
	{ "putbomb", {false, false, false} },
	{ "putexplosion", {false, false, false} },
	{ "putexplosion2", {false, false, false, false} },
	{ "putleaps", {false, false, false} },
	{ "shootball", {}},
	{ "timershow", {}},
	{ "playeringuild", {true}},
	
	{ "toinventory", {true}},
	{ "followplayer", {} },
	{ "openurl", {true} },
	{ "seturllevel", {true} },
	{"seteffect", {false, false, false, false}},
	{ "stopmidi", {}},
	{ "blockagainlocal", {}},
	{ "drawunderplayer", {}},
	{ "puthorse", {true, false, false}},
	{ "canbecarried", {}},
	{ "cannotbecarried", {}},
	{ "noplayerkilling", {}},
	{ "canbepushed", {}},
	{ "cannotbepushed", {}},
	{ "canbepulled", {}},
	{ "cannotbepulled", {}},
	{ "message", {true}},
	{ "setsword", {true, false}},
	{ "setshield", {true, false}},
	{ "putnewcomp", {true, false, false, true, false}},
	{ "putnpc", {true, true, false, false}},
	{ "destroy", {}},
	{ "freezeplayer", {false}},
	{ "sethead", {true}},
	{ "toweapons", {true}},
	{ "showimg", {false, true, false, false}},
	{ "changeimgvis", {false, false}},
	{ "hideimg", {false}},
	{ "hidesword", {false}},
	{ "hitcompu", {false, false, false, false}},
	{ "hitplayer", {false, false, false, false}},
	{ "setbow", {true}},
	{ "updateboard",{false, false, false, false}},
	{ "putobject", {true, false, false}},
	{ "setarray", {false, false}},
	{ "sleep", {false}},
	{ "hideplayer", {false}},
	{ "timereverywhere", {} },
	{ "setminimap", {true, true, false, false}},
	{ "setmap", {true, true, false, false}},
	{ "showcharacter", {}},
	{ "setplayerprop", {true, true}},
	{ "setcharprop", {true, true}},
	{ "showfile", {true}},
	{ "setstring", {true, true}},
	{ "setbackpal", {true}},
	{ "setletters", {true}},
	{ "setgifpart", {true, false, false, false, false}},
	{ "setimgpart", {true, false, false, false, false}},
	{ "setfocus", {false, false}},
	{ "resetfocus", {} },
	{ "setbody", {true}},
	{ "carryobject", {true}},
	{ "throwcarry", {}},
	{ "showlocal", {}},
	{ "hidelocal", {}},
	{"visible", {}},
	{ "dontblock", {} },
	{ "blockagain", {}},
	{ "setbacktile", {false}},
	{ "takeplayerhorse", {} },
	{ "removebomb", {}},
	{ "removearrow", {}},
	{ "removeitem", {}},
	{ "removeexplo", {}},
	{ "removehorse", {}},
	{ "explodebomb", {}},
	{ "reflectarrow", {}},
	{ "lay2", {true, false, false}},
	{ "take2", {false} },
	{ "takehorse", {false}},
	{ "playlooped", {true} },
	{ "stopmidi", {} },
	{ "stopsound", {true}},
	{ "changeimgpart", {false, false, false, false, false}},
	{ "setlevel2", {true, false, false}},
	{ "hitnpc", {false, false, false, false}},
	{ "canwarp", {} },
	{ "setani", {true, true} },
	{ "setcharani", {true, true}},
	{ "show", {} },
	{ "hide", {} },
	{ "setshape", {false, false, false} },
	{ "callweapon", { false, true } },
	{ "setcoloreffect", {false, false, false, false } },
	{ "triggeraction", {false, false, true, true } }
	
};

static gs1::PrototypeMap funcs = {
	{"strtofloat", {true}},
	{"strequals", {true, true}},
	{"streequals", {true, true}},
	{ "playeringuild", {true}},
	{"strcontains", {true, true}},
	{"startswith", {true, true}},
	{"strlen", {true}},
	{"playersays", {true}},
	{"playersays2", {true}},
	{"onwall", {false, false}},
	{"hasweapon", {true}},
	{"setletters", {true}},
	{"abs", {false}},
	{"sin", {false}},
	{"cos", {false}},
	{"arctan", {false}},
	{"arraylen", {false}},
	{"indexof", {true, true}},
	{"keydown", {false}},
	{"vecx", {false}},
	{"vecy", {false}},
	{"onmapx", {true}},
	{"onmapy", {true}},
};

static std::string escape_string(const std::string& s) {
	ByteStream o;
	for (auto c = s.cbegin(); c != s.cend(); c++) {
		switch (*c) {
		case '"': o << "\\\""; break;
		case '\\': o << "\\\\"; break;
		case '\b': o << "\\b"; break;
		case '\f': o << "\\f"; break;
		case '\n': o << "\\n"; break;
		case '\r': continue;
		case '\t': o << "\\t"; break;
		default:

			o << *c;
		}
	}
	return o.Text();

}

void GS1Converter::increaseTab()
{
	m_tab.append("\t");
}

void GS1Converter::decreaseTab()
{
	m_tab = m_tab.substr(0, m_tab.length() - 1);
}

std::string GS1Converter::getTab() const
{

	return std::string(m_tab.length() * 4, ' ');
}

bool GS1Converter::convert(const std::string& input, bool clientSide)
{
	m_clientSide = clientSide;
	auto observer = [](const gs1::Diag& d) ->void
		{
			printf("ERROR: %s\n", d.message.c_str());
		};


	//The boolean value indicates wether we should prepend "this." in front (if there is no other object in front)
	identifierReplacements = {
		{"true", {false, "true"}},
		{"false", {false, "false"}},
		{"up", {false, "0"}},
		{"left", {false, "1"}},
		{"down", {false, "2"}},
		{"right", {false, "3"}},
		{"x", {true, "gx"}},
		{"y", {true, "gy"}},
		{"ap", {true, "ap"}},
		{"isleader", {true, getPlayerList() + "[0] == player"}},
		{"visible", {true, "visibleLocal(player)"}},
		{"setcoloreffect", {true, "setColourEffect("}},
		{"drawaslight", {true, "drawAsLight()"}},
		{"isweapon", {true, "isWeapon"}},
		{"sprite", {true, "gsprite"}},
		{"shieldpower", {true, "shieldPower"}},
		{"swordpower", {true, "swordPower"}},
		{"timeout", {true, "timeout"}},
		{"hearts", {true, "hearts"}},
		{"fullhearts", {true, "fullHearts"}},
		{"dir", {true, "dir"}},
		{"hurtdx", {true, "hurtdx"}},
		{"hurtdy", {true, "hurtdy"}},
		{"id", {true, "id"}},
		{"rupees", {true, "rupees"}},
		{"bombs", {true, "bombs"}},
		{"darts", {true, "arrows"}},
		{"glovepower", {true, "glovePower"}},
		{"save", {true, "save"}},
		{"tokenscount", {true, "tokens.size"}},
		{"canbepushed", {true, "pushPower = -1"}},
		{"canbepulled", {true, "pullPower = -1"}},
		{"weaponscount", {false, "player.weapons.size"}},
		{"carrying", {false, "player.carryObject !== null"}},
		{"playerx", {false, "player.gx"}},
		{"playery", {false, "player.gy"}},
		{"playerap", {false, "player.ap"}},
		{"playerdir", {false, "player.dir"}},
		{"playerid", {false, "player.id"}},
		{"playersprite", {false, "player.gsprite"}},
		{"playerhearts", {false, "player.hearts"}},
		{"playerfullhearts", {false, "player.fullHearts"}},
		{"playerswordpower", {false, "player.swordPower"}},
		{"playershieldpower", {false, "player.shieldPower"}},
		{"playerrupees", {false, "player.rupees"}},
		{"playerbombs", {false, "player.bombs"}},
		{"playerdarts", {false, "player.arrows"}},
		{"throwcarry", {false, "player.throwCarry()"}},
		{"playerglovepower", {false, "player.glovePower"}},
		{"carriesstone", {false, "(player.carryObject != null && player.carryObject.className==\"LightRock\")"}},
		{"carriesblackstone", {false, "(player.carryObject != null && player.carryObject.className==\"HeavyRock\")"} },
		{"peltwithblackstone", {false, "(npc != null && npc.className==\"HeavyRock\")"} },
		{"peltwithnpc", {false, "(npc != null)"} },
		{"peltwithstone", {false, "(npc != null && npc.className==\"LightRock\")"} },
		{"peltwithvase", {false, "(npc != null && npc.className==\"Vase\")"} },
		{"peltwithsign", {false, "(npc != null && npc.className==\"SmallSign\")"} },
		{"players", {true, getPlayerList()}},
		{"playerscount", {true, getPlayerList() + ".size"}},
		
		{"npcs", {true, getNPCList()}},
		{"npcscount", {true, getNPCList() + ".size"}},

		{"compus", {true, getBaddiesList()}},
		{"compuscount", {true, getBaddiesList() + ".size"}},

		{"drawoverplayer", {true, "drawOverPlayer()"}},
		{"drawunderplayer", {true, "drawUnderPlayer()"}},
		{"dontblock", {true, "dontBlock()"}},
		{"blockagain", {true, "blockAgain()"}},

		{"canbecarried", {true, "carryPower = -1"}},
		{"timereverywhere", {false, "//canSuspend = false"}},
		{"hide", {true, "hide()"}},
		{"show", {true, "show()"}},
		{"destroy", {true, "destroy()"}},
		{"int", {false, "toint("}},
		{"arctan", {false, "atan("}},
		{"setshape", {true, "setShape("}},
		{"shootarrow", {true, "shootArrow("}},
		{"shootfireball", {true, "shootFireball("}},
		{"random", {false, "random("}},
		{"updateboard", {false, "this.level.updateBoard("}},

	};

	m_currentStream = &m_output;
	gs1::DiagBuilder diagBuilder(observer);

	gs1::MemorySource source(input.c_str(), input.length());
	gs1::Lexer lexer(diagBuilder, source);

	gs1::Parser parser(diagBuilder, lexer, cmds, funcs);

	auto rootNode = parser.Parse();

	if (rootNode.get()->children.size() > 0)
	{
		increaseTab();
		for (auto i = 0U; i < rootNode.get()->children.size(); ++i)
		{
			auto child = rootNode.get()->children[i];

			if(child->GetType() == "StmtIf" || child->GetType() == "StmtFunctionDecl")
				child->Accept(this);
			else {
				if (child->GetType() != "SyntaxTerminal" && child->GetType() != "StmtEmpty" && child->GetType() != "SyntaxNode")
				{
					m_currentStream = &m_parseActionFunction;

					output() << getTab();
					child->Accept(this);
					if (child->hasEndOfLine())
						output() << ";\n";
					m_currentStream = &m_output;
				}
			}

		}

		output() << "\n";

		//Generate our action (example function onPlayerTouchesMe(player){})
		for (auto it : m_actionCode)
		{
			auto& actionName = it.first;
			auto actionCode = std::string(it.second.first.Text());
			auto hasParseGS1Call = it.second.second;

			auto it2 = validActions.find(actionName);
			if (it2 != validActions.end())
			{
				auto& funcHeader = it2->second;

				m_output << "function " << funcHeader[0] << "(";

				bool hasContextualPlayer = false;
				for (auto i = 1U; i < funcHeader.size(); ++i)
				{
					if (funcHeader[i] == "P") {
						m_output << "player";
						hasContextualPlayer = true;
					}
					else if (funcHeader[i] == "C")
						m_output << "_chatText";

					else if (funcHeader[i] == "N")
						m_output << "npc";

					if (i != funcHeader.size() - 1)
						m_output << ", ";
				}
				m_output << ")\n";
				m_output << "{\n";

				if (m_createCode.Length() && actionName == "created")
				{
					m_output << m_createCode << "\n";
					m_createCode.Clear();
				}

				if (!hasContextualPlayer && (actionCode.find("player.") != std::string::npos || actionName == "timeout"))
				{
					if (m_clientSide)
						m_output << "    player = localPlayer();\n";
					else {
						if (m_isWeapon)
							m_output << "    player = this.player;\n";
						else m_output << "    player = if(this." << getPlayerList() << ".size, " << "this." << getPlayerList() << "[0], null); \n";
					}
					//For timeout, do a check to make sure there exists a "player".
					//Otherwise reset the timeout for 1 more second
					if (actionName == "timeout")
					{
						m_output << "    if(!player){\n";
						m_output << "        this.timeout = 1.0;\n";
						m_output << "        return;\n";
						m_output << "    }\n";
					}
				}



				m_output << actionCode << "\n";

				if (m_addParseGS1ToAllActions && !hasParseGS1Call)
				{
					m_output << getTab() << "this.parseGS1Action(\"";
					m_output << actionName << "\", player, ";

					std::string contextualNPC = "null";

					auto it = validActions.find(actionName);
					if (it != validActions.end())
					{
						auto& funcHeader = it->second;
						for (auto i = 1U; i < funcHeader.size(); ++i)
						{
							if (funcHeader[i] == "N")
							{
								contextualNPC = "npc";
							}
						}

						m_output << contextualNPC;
						m_output << ");\n";
					}
				}

				m_output << "}\n\n";
			}
		}

		//Our "parseG1Action" function is for more complex conditions that check multiple
		//actions in a single if statement
		if (m_parseActionFunction.Length() > 0)
		{
			m_output << "function parseGS1Action(_actionName, player, npc)\n";
			m_output << "{\n";
			m_output << m_parseActionFunction;
			m_output << "}\n";
		}

		if (m_generatedFunctionsCode.Length() > 0)
		{
			m_output << m_generatedFunctionsCode;
		}
		return true;
	}
	return false;
}

std::string GS1Converter::convert2(const std::string& input, bool clientSide)
{
	GS1Converter converter;
	if (converter.convert(input, clientSide))
		return converter.result();
	return "";

}

// templated version of my_equal so it could work with both char and wchar_t
template<typename charT>
struct my_equal {
	my_equal(const std::locale& loc) : loc_(loc) {}
	bool operator()(charT ch1, charT ch2) {
		return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
	}
private:
	const std::locale& loc_;
};

// find substring (case insensitive)
template<typename T>
int ci_find_substr(const T& str1, const T& str2, const std::locale& loc = std::locale())
{
	typename T::const_iterator it = std::search(str1.begin(), str1.end(),
		str2.begin(), str2.end(), my_equal<typename T::value_type>(loc));
	if (it != str1.end()) return it - str1.begin();
	else return std::string::npos; // not found
}


std::string GS1Converter::convert3(const std::string& input)
{
	std::string retval = "//sgscript\n";
	auto pos = ci_find_substr(input, std::string("//clientside"));
	if(pos == std::string::npos)
		pos = ci_find_substr(input, std::string("//#clientside"));
		
		

	if (pos != std::string::npos)
	{
		auto serverCode = input.substr(0, pos);
		auto clientCode = input.substr(pos);

		retval += GS1Converter::convert2(serverCode, false) + "\n//#clientside\n";
		retval += GS1Converter::convert2(clientCode, true) + "\n";
	}
	else {
		retval += GS1Converter::convert2(input, false) + "\n";
	}

	retval += "/*\n//gs1script\n" + input + "*/";
	return retval;

}

std::string GS1Converter::revert(const std::string& input)
{
	auto pos = ci_find_substr(input, std::string("//gs1script"));

	if (pos != std::string::npos)
	{
		//skip first line
		while (pos < input.length())
		{
			if (input[pos++] == '\n')
				break;
		}

		int commentCount = 1;
		for (auto i = pos; i < input.length(); ++i)
		{
			if (pos + 2 < input.length())
			{
				if (input[i] == '/' && input[i + 1] == '*')
					++commentCount;

				else if (input[i] == '*' && input[i + 1] == '/')
				{
					--commentCount;

					if (commentCount == 0)
					{
						return input.substr(pos, i - pos);
					}
				}
			}
		}
	}
	return "";
}

std::string GS1Converter::result() const
{
	return std::string(m_output.Text());
}

bool GS1Converter::replaceshowimg(std::vector<gs1::Expr*>& args)
{
	generateFunction("showimg");
	if (args.size() >= 4)
	{
		output() << "this.showimg(player, ";
		args[0]->Accept(this);
		output() << ", ";
		args[1]->Accept(this);
		output() << ", ";
		args[2]->Accept(this);
		output() << ", ";
		args[3]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacechangeimgvis(std::vector<gs1::Expr*>& args)
{
	generateFunction("changeimgvis");
	if (args.size() >= 2)
	{
		output() << "this.changeimgvis(";
		args[0]->Accept(this);
		output() << ", ";
		args[1]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacearctan(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "atan(";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacehideimg(std::vector<gs1::Expr*>& args)
{
	generateFunction("hideimg");
	if (args.size() >= 1)
	{
		output() << "this.hideimg(";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacechangeimgpart(std::vector<gs1::Expr*>& args)
{
	generateFunction("changeimgpart");
	if (args.size() >= 5)
	{
		output() << "this.changeimgpart(";
		args[0]->Accept(this);
		output() << ", ";
		args[1]->Accept(this);
		output() << ", ";
		if (args[2])args[2]->Accept(this);
		output() << ", ";
		if (args[3])args[3]->Accept(this);
		output() << ", ";
		if (args[4])args[4]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replaceshowcharacter(std::vector<gs1::Expr*>& args)
{
	generateFunction("showcharacter");
	output() << "this.showcharacter()";
	return true;
}

bool GS1Converter::replacesetcharprop(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
		if (args[0] && args[0]->GetType() == "ExprStringLiteral")
		{
			bool wasColour = false;
			auto propName = args[0] ? formatStringProp(static_cast<gs1::ExprStringLiteral*>(args[0])->literal->token.text, false, &wasColour) : "";

			if (!propName.empty())
			{
				output() << propName << " = ";

				args[1]->Accept(this);

				return true;
			}
		}
	}

	return false;
}

bool GS1Converter::replacesetplayerprop(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
		if (args[0]->GetType() == "ExprStringLiteral")
		{
			bool wasColour = false;
			auto propName = formatStringProp(static_cast<gs1::ExprStringLiteral*>(args[0])->literal->token.text, true, &wasColour);

			if (!propName.empty())
			{
				output() << propName << "=";

				args[1]->Accept(this);
				return true;
			}
		}
	}

	return false;
}

bool GS1Converter::replacetokenize(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "this.tokens = string_explode(";
		args[0]->Accept(this);
		output() << ", \" \")";

		return true;

	}
	return false;
}

bool GS1Converter::replaceplay(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		if (args[0] && args[0]->GetType() == "ExprStringLiteral")
		{
			auto fileName = trim(static_cast<gs1::ExprStringLiteral*>(args[0])->literal->token.text);

			if (endsWith(fileName, ".mid"))
			{
				fileName = fileName.substr(0, fileName.length() - 4) + ".ogg";

				output() << "player.playMusic(\"";
				output() << fileName;
				output() << "\")";
				return true;
			}

		}

		output() << "this.playSound(";
		args[0]->Accept(this);
		output() << ")";
		
		return true;

	}
	return false;
}

bool GS1Converter::replacestopmidi(std::vector<gs1::Expr*>& args)
{
	output() << "player.stopMusic()";
	return true;
}

bool GS1Converter::replaceenableweapons(std::vector<gs1::Expr*>& args)
{
	output() << "player.enableWeapons()";
	return true;
}

bool GS1Converter::replacedisableweapons(std::vector<gs1::Expr*>& args)
{
	output() << "player.disableWeapons()";
	return true;
}

bool GS1Converter::replacesetgif(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "this.imageName=";
		args[0]->Accept(this);

		return true;
	}
	return false;
}

bool GS1Converter::replaceset(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		if (args[0]->GetType() == "ExprStringLiteral")
		{
			auto expr = static_cast<gs1::ExprStringLiteral*>(args[0]);
			auto flagName = expr->literal->token.text;

			auto identifier = fixIdentifier(trim(flagName));
			output() << "{if(!" << identifier << "){" << identifier << " = true;";
			output() << "this.fireLevelEvent(\"parseGS1Action\", \"\", player, null);}}";
			return true;
		}
		output() << "player.setFlag(";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replaceunset(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		if (args[0]->GetType() == "ExprStringLiteral")
		{
			auto expr = static_cast<gs1::ExprStringLiteral*>(args[0]);
			auto flagName = fixIdentifier(trim(expr->literal->token.text));

			auto index = flagName.find_last_of('.');
			if (index != std::string::npos)
			{
				output() << "unset(" << flagName.substr(0, index) << ", \"" << flagName.substr(index + 1) << "\")";
			} else 	output() << flagName << " = null";
			return true;
		}
		output() << "player.unsetFlag(";
		if(args[0])if(args[0])args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}


bool GS1Converter::replaceputhorse(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 3)
	{
		output() << "this.createObject((";
		args[1]->Accept(this);
		output() << ") * 16" << getLevelX() << ", (";
		if (args[2])args[2]->Accept(this);
		output() << ") * 16" << getLevelY() << ", ";
		output() << "\"horse\"";
		output() << ", ";
		args[0]->Accept(this);
		output() << ")";

		return true;
	}
	return false;
}

bool GS1Converter::replaceputcomp(std::vector<gs1::Expr*>& args)
{
	static std::unordered_map<std::string, int> baddyTypes = {
		{"graysoldier", 0},
		{"bluesoldier", 1},
		{"redsoldier", 2},
		{"shootingsoldier", 3},
		{"swampsoldier", 4},
		{"frog", 5},
		{"octopus", 6},
		{"spider", 6},
		{"goldenwarrior", 7},
		{"lizardon", 8},
		{"dragon", 9},
	};

	if (args.size() >= 3)
	{
		int baddyIndex = -1;
		if (args[0] && args[0]->GetType() == "ExprStringLiteral")
		{
			auto it = baddyTypes.find(static_cast<gs1::ExprStringLiteral*>(args[0])->literal->token.text);
			if (it != baddyTypes.end())
				baddyIndex = it->second;
		}

		output() << "this.createObject((";
		args[1]->Accept(this);
		output() << ") * 16" << getLevelX() << ", (";
		if (args[0])args[2]->Accept(this);
		output() << ") * 16" << getLevelY() << ", ";
		output() << "\"Baddy\"";
		output() << ", ";

		if (baddyIndex >= 0)
			output() << std::to_string(baddyIndex);
		else if (args[0])
			args[0]->Accept(this);

		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replaceputnewcomp(std::vector<gs1::Expr*>& args)
{
	static std::unordered_map<std::string, int> baddyTypes = {
			{"graysoldier", 0},
			{"bluesoldier", 1},
			{"redsoldier", 2},
			{"shootingsoldier", 3},
			{"swampsoldier", 4},
			{"frog", 5},
			{"octopus", 6},
			{"spider", 6},
			{"goldenwarrior", 7},
			{"lizardon", 8},
			{"dragon", 9},
	};

	if (args.size() >= 5)
	{
		int baddyIndex = -1;
		if (args[0]->GetType() == "ExprStringLiteral")
		{
			auto it = baddyTypes.find(static_cast<gs1::ExprStringLiteral*>(args[0])->literal->token.text);
			if (it != baddyTypes.end())
				baddyIndex = it->second;
		}

		output() << "this.createObject((";
		args[1]->Accept(this);
		output() << ") * 16" << getLevelX() << ", (";
		if (args[0])args[2]->Accept(this);
		output() << ") * 16" << getLevelY() << ", ";
		output() << "\"Baddy\"";
		output() << ", ";

		if (baddyIndex >= 0)
			output() << std::to_string(baddyIndex);
		else if (args[0])
			args[0]->Accept(this);

		output() << ", ";
		args[3]->Accept(this);
		output() << ", ";
		args[4]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replaceplayersays(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.chat==";
		args[0]->Accept(this);

		m_tempActionsList.insert("playerchats");
		return true;
	}
	return false;

}

bool GS1Converter::replacesay(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.showSign(";
		args[0]->Accept(this);
		output() << ", true)";
		return true;
	}
	return false;
}

bool GS1Converter::replacesay2(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.showSignText(";
		args[0]->Accept(this);
		output() << ", true)";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetani(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.setAni(";
		args[0]->Accept(this);
		output() << " $ \".gani\"";
		if (args.size() >= 2)
		{
			output() << ", ";
			args[1]->Accept(this);
		}
		output() << ")";

		return true;
	}
	return false;
}

bool GS1Converter::replacesetcharani(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "this.setAni(";
		args[0]->Accept(this);
		output() << " $ \".gani\"";
		if (args.size() >= 2)
		{
			output() << ", ";
			args[1]->Accept(this);
		}
		output() << ")";

		return true;
	}
	return false;
}

bool GS1Converter::replacedontblocklocal(std::vector<gs1::Expr*>& args)
{
	output() << "this.dontBlockLocal(player)";
	return true;
}

bool GS1Converter::replaceblockagainlocal(std::vector<gs1::Expr*>& args)
{
	output() << "this.blockAgainLocal(player)";
	return true;
}

bool GS1Converter::replacevisible(std::vector<gs1::Expr*>& args)
{
	output() << "this.visibleLocal(player)";
	return true;
}

bool GS1Converter::replacecarryobject(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		if (args[0]->GetType() == "ExprStringLiteral")
		{
			auto className = static_cast<gs1::ExprStringLiteral*>(args[0])->literal->token.text;
			if (className == "blackstone")
				className = "HeavyRock";

			output() << "player.carryObject = this.createObject(player.x, player.y, \"" + className + "\")";
		}


		return true;
	}
	return false;
}

bool GS1Converter::replacetakeplayercarry(std::vector<gs1::Expr*>& args)
{
	output() << "if(player.carryObject){player.carryObject.destroy();}";
	return true;
}

bool GS1Converter::replacesetarray(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
		if (args[0] && args[0]->GetType() == "ExprId")
		{
			auto name = static_cast<gs1::ExprId*>(args[0])->name->token.text;

			auto identifier = fixIdentifier(name);
			output() << identifier << " = if(typeof(" << identifier << ") == \"array\", " << identifier << ", array_sized(";
			args[1]->Accept(this);
			output() << "))";
			return true;
		}

	}
	return false;
}

bool GS1Converter::replaceonwall(std::vector<gs1::Expr*>& args)
{
	generateFunction("onwall");
	if (args.size() >= 2)
	{
		output() << "this.onwall(";
		args[0]->Accept(this);
		output() << ", ";
		args[1]->Accept(this);
		output() << ")";
		return true;

	}
	return false;
}

bool GS1Converter::replacemessage(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "this.chat=";
		args[0]->Accept(this);
		return true;
	}
	return false;
}

bool GS1Converter::replacelay(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "this.layItemName(";
		args[0]->Accept(this);
		output() << ", this.x, this.y)";

		return true;
	}
	return false;
}

bool GS1Converter::replacelay2(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 3)
	{
		output() << "this.layItemName(";
		args[0]->Accept(this);
		output() << ", (";
		args[1]->Accept(this);
		output() << ") * 16" << getLevelX()  << ", (";
		if (args[2])args[2]->Accept(this);
		output() << ") * 16" << getLevelY() <<  ")";

		return true;
	}
	return false;
}

bool GS1Converter::replacerandom(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
		output() << "(";
		args[0]->Accept(this);
		output() << "+";

		output() << "(rand()%(";
		args[1]->Accept(this);
		output() << "-";
		args[0]->Accept(this);
		output() << ")))";
		return true;
	}
	return false;
}

bool GS1Converter::replaceputleaps(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 3)
	{
		output() << "this.putLeaps(";
		args[0]->Accept(this);
		output() << ", (";
		args[1]->Accept(this);
		output() << ") * 16" << getLevelX() << ", (";
		if (args[2])args[2]->Accept(this);
		output() << ") * 16" << getLevelY() << ")";

		return true;
	}
	return false;
}

bool GS1Converter::replaceremovecompus(std::vector<gs1::Expr*>& args)
{
	generateFunction("removecompus");
	output() << "this.removeBaddies()";
	return true;
}

bool GS1Converter::replacefreezeplayer(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.idleFreeze(";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetlevel(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.levelName = ";
		args[0]->Accept(this);
		return true;
	}
	return false;
}

bool GS1Converter::replacetoweapons(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "this.toWeapons(player, ";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetgifpart(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 5)
	{
		output() << "this.setImagePart(";
		args[0]->Accept(this);
		output() << ", ";
		args[1]->Accept(this);
		output() << ", ";
		if (args[2])args[2]->Accept(this);
		output() << ", ";
		if (args[3])args[3]->Accept(this);
		output() << ", ";
		if (args[4])args[4]->Accept(this);
		output() << ");\n";

		output() << getTab() << "this.setShape(0, ";
		if (args[3])args[3]->Accept(this);
		output() << ", ";
		if (args[4])args[4]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacetake(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "this.takeItemName(";
		args[0]->Accept(this);
		output() << ", npc)";
		return true;
	}
	return false;
}


bool GS1Converter::replacestrequals(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
	

		output() << "(";
		args[0]->Accept(this);
		output() << "==";
		args[1]->Accept(this);
		output() << ")";
		return true;

	}
	return false;
}

bool GS1Converter::replacestartswith(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
		output() << "string_compare(";
		args[1]->Accept(this);
		output() << ", ";
		args[0]->Accept(this);
		output() << ", ";
		args[0]->Accept(this);
		output() << ".size) == 0";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetstring(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{

		if (args[1] && args[1]->GetType() == "ExprStringLiteral")
		{
			auto value = static_cast<gs1::ExprStringLiteral*>(args[1])->literal->token.text;


			bool wasColour = false;
			auto propName = formatStringProp(value, true, &wasColour);

			if (!propName.empty())
			{
				if (args[0] && args[0]->GetType() == "ExprStringLiteral")
					output() << fixIdentifier(static_cast<gs1::ExprStringLiteral*>(args[0])->literal->token.text);

				output() << " = " << propName;
						
				return true;
			}


			
		}
		
		
	}
	return false;
}

bool GS1Converter::replacestrtofloat(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		if (args[0] && args[0]->GetType() == "ExprStringLiteral")
		{
			auto name = static_cast<gs1::ExprStringLiteral*>(args[0])->literal->token.text;

			bool wasColour = false;
			auto propName = formatStringProp(name, true, &wasColour);

			if (!propName.empty())
			{
				output() << "toreal(" << propName << ")";
				return true;
			}

		}
	}
	return false;
}

bool GS1Converter::replacehitplayer(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 4)
	{
		output() << "this.hurtPlayer(this." << getPlayerList() << "[";
		args[0]->Accept(this);
		output() << "], ";
		args[1]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetbackpal(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.changeTilesetPalette(player.level.tileset, ";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacehitcompu(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 4)
	{
		output() << "this." << getBaddiesList() << "[";
		args[0]->Accept(this);
		output() << "].onWasHit(player, ";
		args[1]->Accept(this);
		output() << ", (";
		if (args[2])args[2]->Accept(this);
		output() << ") * 16" << getLevelX() << ", (";
		if (args[3])args[3]->Accept(this);
		output() << ") * 16" << getLevelY() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacehurt(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "this.hurtPlayer(player, ";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacevecx(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "vecx[";
		args[0]->Accept(this);
		output() << "]";
		return true;
	}
	return false;
}

bool GS1Converter::replacevecy(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "vecy[";
		args[0]->Accept(this);
		output() << "]";
		return true;
	}
	return false;
}

bool GS1Converter::replaceputbomb(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 3)
	{
		output() << "this.createObject((";
		args[1]->Accept(this);
		output() << ") * 16" << getLevelX() << ", (";
		if (args[2])args[2]->Accept(this);
		output() << ") * 16" << getLevelY() << ", \"bomb\", ";
		args[0]->Accept(this);

		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replaceputexplosion(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 3)
	{
		output() << "this.createExplosion(";
		args[0]->Accept(this);
		output() << ", (";
		args[1]->Accept(this);
		output() << ") * 16" << getLevelX() << ", (";
		if (args[2])args[2]->Accept(this);
		output() << ") * 16" << getLevelY();
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replaceputexplosion2(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 4)
	{
		output() << "this.createExplosion(";
		args[1]->Accept(this);
		output() << ", (";
		if (args[2])args[2]->Accept(this);
		output() << ") * 16" << getLevelX() << ", (";
		if (args[3])args[3]->Accept(this);
		output() << ") * 16" << getLevelY();
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetletters(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.private.letters = ";
		args[0]->Accept(this);
		return true;
	}
	return false;
}

bool GS1Converter::replacesetlevel2(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 3)
	{
		output() << "player.warp(";
		args[0]->Accept(this);
		output() << ", (";
		args[1]->Accept(this);
		output() << ") * 16, (";
		if (args[2])args[2]->Accept(this);
		output() << ") * 16";
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesethead(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.head = ";
		args[0]->Accept(this);
		return true;
	}
	return false;
}

bool GS1Converter::replacesetskincolor(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.colours[BODY_SKIN] = stringColour(";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetsleevecolor(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.colours[BODY_SLEEVE] = stringColour(";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetcoatcolor(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.colours[BODY_COAT] = stringColour(";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetbeltcolor(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.colours[BODY_BELT] = stringColour(";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetshoecolor(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.colours[BODY_SHOES] = stringColour(";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacesetshield(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.shield = ";
		args[0]->Accept(this);
		return true;
	}
	return false;
}

bool GS1Converter::replacesetsword(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.sword = ";
		args[0]->Accept(this);
		return true;
	}
	return false;
}

bool GS1Converter::replacestrlen(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		args[0]->Accept(this);
		output() << ".length";
		return true;
	}
	return false;
}

bool GS1Converter::replacehidelocal(std::vector<gs1::Expr*>& args)
{
	output() << "this.hideLocal(player)";
	return true;
}

bool GS1Converter::replaceshowlocal(std::vector<gs1::Expr*>& args)
{
	output() << "this.showLocal(player)";
	return true;
}

bool GS1Converter::replacehasweapon(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		output() << "player.hasWeaponName(";
		if(args[0])args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacearraylen(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 1)
	{
		args[0]->Accept(this);
		output() << ".size";
		return true;
	}
	return false;
}

bool GS1Converter::replaceindexof(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
		output() << "string_find(";
		args[1]->Accept(this);
		output() << ", ";
		args[0]->Accept(this);
		output() << ")";
		return true;
	}
	return false;
}

bool GS1Converter::replacestrcontains(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
		output() << "string_find(";
		args[0]->Accept(this);
		output() << ", ";
		args[1]->Accept(this);
		output() << ") !== null";
		return true;
	}
	return false;
}

bool GS1Converter::replacekeydown(std::vector<gs1::Expr*>& args)
{
	generateFunction("keycodes");
	if (args.size() >= 1)
	{
		output() << "player.keyDown(this.keyCodes[";
		args[0]->Accept(this);
		output() << "])";
		return true;
	}
	return false;
}

bool GS1Converter::replacecallweapon(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
		if (args[1]->GetType() == "ExprStringLiteral")
		{
			auto funcName = static_cast<gs1::ExprStringLiteral*>(args[1])->literal->token.text;

			auto it = validActions.find(funcName);
			if (it != validActions.end())
			{
				auto& values = it->second;
				if (values.size() > 0)
				{
					output() << "player.weapons[";
					args[0]->Accept(this);
					output() << "]." << values[0] << "(player)";
					return true;
				}
			}
			else {
				output() << "player.weapons[";
				args[0]->Accept(this);
				output() << "]." << funcName << "(player)";
				return true;
			}

		
		}

	}
	return false;
}

bool GS1Converter::replacecallnpc(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 2)
	{
		if (args[1]->GetType() == "ExprStringLiteral")
		{
			auto funcName = static_cast<gs1::ExprStringLiteral*>(args[1])->literal->token.text;

			auto it = validActions.find(funcName);
			if (it != validActions.end())
			{
				auto& values = it->second;
				if (values.size() > 0)
				{
					output() << "this." << getNPCList() << "[";
					args[0]->Accept(this);
					output() << "]." << values[0] << "(player)";
					return true;
				}
			}
			else {
				output() << "this." << getNPCList() << "[";
				args[0]->Accept(this);
				output() << "]." << funcName << "(player)";
				return true;
			}


		}

	}
	return false;
}

bool GS1Converter::replacetriggeraction(std::vector<gs1::Expr*>& args)
{
	if (args.size() >= 4)
	{
		generateFunction("triggeraction");
		output() << "this.triggeraction(player, ";
		args[0]->Accept(this);
		output() << ", ";
		args[1]->Accept(this);
		output() << ", ";
		args[2]->Accept(this);
		output() << ", ";
		args[3]->Accept(this);
		output() << ")";

		return true;
	}
	return false;
}

bool GS1Converter::replaceshootball(std::vector<gs1::Expr*>& args)
{
	output() << "this.shootBall(player)";
	return true;
}





bool GS1Converter::commandReplacement(const std::string& name, std::vector<gs1::Expr*>& args)
{
	//When we come across the following functions/commands
	//We will output something different
	static std::unordered_map<std::string, bool (GS1Converter::*)(std::vector<gs1::Expr*>&)> replacements = {
		{"arctan", &GS1Converter::replacearctan},
		{"puthorse", &GS1Converter::replaceputhorse},
		{"showimg", &GS1Converter::replaceshowimg},
		{"hideimg", &GS1Converter::replacehideimg},
		{"changeimgvis", &GS1Converter::replacechangeimgvis},
		{"visible", &GS1Converter::replacevisible},
		{"carryobject", &GS1Converter::replacecarryobject},
		{"takeplayercarry", &GS1Converter::replacetakeplayercarry},
		{"changeimgpart", &GS1Converter::replacechangeimgpart},
		{"putcomp", &GS1Converter::replaceputcomp},
		{"putnewcomp", &GS1Converter::replaceputnewcomp},
		{"take", &GS1Converter::replacetake},
		{"playersays", &GS1Converter::replaceplayersays},
		{"say", &GS1Converter::replacesay},
		{"say2", &GS1Converter::replacesay2},
		{"setani", &GS1Converter::replacesetani},
		{"setcharani", &GS1Converter::replacesetcharani},
		{"setcharprop", &GS1Converter::replacesetcharprop},
		{"setplayerprop", &GS1Converter::replacesetplayerprop},
		{"setstring", &GS1Converter::replacesetstring},
		{"strtofloat", &GS1Converter::replacestrtofloat},
		{"showcharacter", &GS1Converter::replaceshowcharacter},
		{"play", &GS1Converter::replaceplay},
		{"playlooped", &GS1Converter::replaceplay},
		{"stopmidi", &GS1Converter::replacestopmidi},
		{"enableweapons", &GS1Converter::replaceenableweapons},
		{"disableweapons", &GS1Converter::replacedisableweapons},
		{"setgif", &GS1Converter::replacesetgif},
		{"setimg", &GS1Converter::replacesetgif},
		{"set", &GS1Converter::replaceset},
		{"toinventory", &GS1Converter::replaceset},
		{"unset", &GS1Converter::replaceunset},
		{"setarray", &GS1Converter::replacesetarray},
		{"onwall", &GS1Converter::replaceonwall},
		{"message", &GS1Converter::replacemessage},
		{"lay", &GS1Converter::replacelay},
		{"lay2", &GS1Converter::replacelay2},
		{"putleaps", &GS1Converter::replaceputleaps},
		{"removecompus", &GS1Converter::replaceremovecompus},
		{"freezeplayer", &GS1Converter::replacefreezeplayer},
		{"setlevel", &GS1Converter::replacesetlevel},
		{"toweapons", &GS1Converter::replacetoweapons},
		{"setgifpart", &GS1Converter::replacesetgifpart},
		{"setimgpart", &GS1Converter::replacesetgifpart},
		{"strequals", &GS1Converter::replacestrequals},
		{"streequals", &GS1Converter::replacestrequals},
		{"startswith", &GS1Converter::replacestartswith},

		{"tokenize", &GS1Converter::replacetokenize},
		{"hitplayer", &GS1Converter::replacehitplayer},
		{"setbackpal", &GS1Converter::replacesetbackpal},


		{ "hitcompu", &GS1Converter::replacehitcompu},
		{"hurt", &GS1Converter::replacehurt},
		{"vecx", &GS1Converter::replacevecx},
		{"vecy", &GS1Converter::replacevecy},
		{"putbomb", &GS1Converter::replaceputbomb},
		{"putexplosion", &GS1Converter::replaceputexplosion},
		{"putexplosion2", &GS1Converter::replaceputexplosion2},
		{"setletters", &GS1Converter::replacesetletters},
		{"sethead", &GS1Converter::replacesethead},
		{"setskincolor", &GS1Converter::replacesetskincolor},
		{"setsleevecolor", &GS1Converter::replacesetsleevecolor},
		{"setcoatcolor", &GS1Converter::replacesetcoatcolor},
		{"setbeltcolor", &GS1Converter::replacesetbeltcolor},
		{"setshoecolor", &GS1Converter::replacesetshoecolor},
		{"setsword", &GS1Converter::replacesetsword},
		{"setshield", &GS1Converter::replacesetshield},
		{"setlevel2", &GS1Converter::replacesetlevel2},
		{"strlen", &GS1Converter::replacestrlen},
		{"hidelocal", &GS1Converter::replacehidelocal},
		{"showlocal", &GS1Converter::replaceshowlocal},
		{"dontblocklocal", &GS1Converter::replacedontblocklocal},
		{"blockagainlocal", &GS1Converter::replaceblockagainlocal},
		{"hasweapon", &GS1Converter::replacehasweapon},
		{"arraylen", &GS1Converter::replacearraylen},
		{"indexof", &GS1Converter::replaceindexof},
		{"strcontains", &GS1Converter::replacestrcontains},
		{"keydown", &GS1Converter::replacekeydown},
		{"callweapon", &GS1Converter::replacecallweapon},
		{"callnpc", &GS1Converter::replacecallnpc},
		{"triggeraction", &GS1Converter::replacetriggeraction},
		{"shootball", &GS1Converter::replaceshootball},
		
	};

	auto it = replacements.find(name);
	if (it != replacements.end())
	{
		auto& func = it->second;
		return (this->*func)(args);
	}
	return false;
}



bool GS1Converter::generateshowcharacter()
{
	m_generatedFunctionsCode << "function showcharacter()\n"
		"{\n"
		"    if(!this.useGraalCharacterOffset)\n"
		"    {\n"
		"        this.tileX += 0.5;\n"
		"        this.tileY += 1.0;\n"
		"        this.useGraalCharacterOffset = true;\n"
		"        this.hearts = this.fullHearts = 3;\n"
		"        this.setShape(0, 32, 32);\n"
		"        this.gsprite=0;\n"
		"        this.body=\"body.png\";\n"
		"        this.solid = true;\n"
		"    }\n"

		"}\n";

	return true;
}

bool GS1Converter::generateonwall()
{
	if (m_useLocalCoordinates)
	{
		m_generatedFunctionsCode << "function onwall(x, y)\n"
			"{\n"
			"    if(tileGetType(this.level.getTile(x, y )) & this.blockingTiles)\n"
			"        return true;\n"
			"        \n"
			"    if(this.isOnPlayer(x * 16" << getLevelX() << ", y * 16" << getLevelY() << ", 16, 16))\n"
			"        return true;\n"
			"    \n"
			"    return false;\n"
			"}\n";
	}
	else {
		m_generatedFunctionsCode << "function onwall(x, y)\n"
			"{\n"
			"    if(tileGetType(this.getTile(x, y)) & this.blockingTiles)\n"
			"        return true;\n"
			"        \n"
			"    if(this.isOnPlayer(x * 16" << getLevelX() << ", y * 16" << getLevelY() << ", 16, 16))\n"
			"        return true;\n"
			"    \n"
			"    return false;\n"
			"}\n";
	}
	return true;
}

bool GS1Converter::generatetriggeraction()
{
	m_generatedFunctionsCode << "functon triggeraction(player, x, y, action, params)\n"
		"{\n"
		"    other = this.testForObject(x * 16" << getLevelX() << ", y * 16" << getLevelY() << ", 16, 16);\n"
		"    if(other && other.parseGS1Action)\n"
		"        other.parseGS1Action(\"action\" $ action, player, null);\n"
		"}\n\n";
	return true;
}

bool GS1Converter::generateshowimg()
{
	m_actionCode["created"];
	m_createCode << "    this.displayItems=map();\n";
	m_generatedFunctionsCode << "function showimg(player, id, name, x, y)\n"
		"{\n"
		"    id = toint(id);\n"
		"    item = this.displayItems[id];\n"
		"    if(item)\n"
		"    {\n"
		"        if(item.screenItem)\n"
		"        {\n"
		"            item.x = x;\n"
		"            item.y = y;\n"
		"        } else {\n"
		"            item.x = x * 16" << getLevelX() << ";\n"
		"            item.y = y * 16" << getLevelY() << ";\n"
		"        }\n"
		"        item.image = name;\n"
		"        this.addDisplayItem(item);\n"
		"        return;\n"
		"    }\n"
		"    item = new DisplayItem(x * 16" << getLevelX() << ", y * 16" << getLevelY() << ", if(id >= 200, false, true));\n"
		"    item.image = name;\n"
		"    this.displayItems[id] = item;\n"
		"    this.addDisplayItem(item, if(id >= 200, player, null));\n"
		"    return item;\n"
		"}\n\n";

	return true;
}

bool GS1Converter::generatechangeimgvis()
{
	m_generatedFunctionsCode << "function changeimgvis(id, value)\n"
		"{\n"
		"    id = toint(id);\n"
		"    item = this.displayItems[id];\n"
		"    if(item)\n"
		"        item.screenItem = value == 4;\n"
		"}\n\n";
	return false;
}

bool GS1Converter::generatehideimg()
{
	m_generatedFunctionsCode << "function hideimg(id)\n"
		"{\n"
		"    id = toint(id);\n"
		"    item = this.displayItems[id];\n"
		"    if(item) {\n"
		"        this.removeDisplayItem(item);\n"
		"        unset(this.displayItems, id);\n"
		"    }\n"
		"}\n\n";

	return true;
}

bool GS1Converter::generatechangeimgpart()
{
	m_generatedFunctionsCode << "function changeimgpart(id, x, y, width, height)\n"
		"{\n"
		"    id = toint(id);\n"
		"    item = this.displayItems[id];\n"
		"    if(item) {\n"
		"        item.setImagePart(x, y, width, height);\n"
		"    }\n"
		"}\n\n";

	return true;
}



bool GS1Converter::generatekeycodes()
{
	m_generatedFunctionsCode << "global keyCodes = [KEY_UP, KEY_LEFT, KEY_DOWN, KEY_RIGHT, KEY_S, KEY_A, KEY_D, KEY_M, KEY_TAB, KEY_Q, KEY_P];\n";
	return true;
}

bool GS1Converter::generatesave()
{
	m_actionCode["created"];
	m_createCode << "    this.save = map();\n";
	return true;
}

bool GS1Converter::generateremoveBaddies()
{
	m_generatedFunctionsCode << "function removeBaddies()\n"
		"{\n"
		"    for(i = this.level.baddies.size - 1; i >= 0; --i)\n"
		"        this.level.baddies[i].destroy();\n"
		"}\n\n";
	return true;
}



bool GS1Converter::generateFunction(const std::string& name)
{
	static std::unordered_map<std::string, bool (GS1Converter::*)()> generators = {
		{"showimg", &GS1Converter::generateshowimg},
		{"hideimg", &GS1Converter::generatehideimg},
		{"changeimgvis", &GS1Converter::generatechangeimgvis},
		{"changeimgpart", &GS1Converter::generatechangeimgpart},
		{"showcharacter", &GS1Converter::generateshowcharacter}, 
		{"onwall", &GS1Converter::generateonwall},
		{"keycodes", &GS1Converter::generatekeycodes},
		{"save", &GS1Converter::generatesave},
		{"removecompus", &GS1Converter::generateremoveBaddies},
		{"triggeraction", &GS1Converter::generatetriggeraction},
		
	};

	if (m_generatedFunctions.find(name) == m_generatedFunctions.end())
	{
		m_generatedFunctions.insert(name);

		auto it = generators.find(name);
		if (it != generators.end())
		{
			auto& func = it->second;
			return (this->*func)();
		}
	}
	return false;
}

std::string GS1Converter::getLevelX() const
{
	if (m_useLocalCoordinates)
		return " + this.level.x";
	return "";
}

std::string GS1Converter::getLevelY() const
{
	if (m_useLocalCoordinates)
		return " + this.level.y";
	return "";
}

std::string GS1Converter::getPlayerList() const
{
	if (m_useLocalPlayerList)
		return "level.players";
	return "players";
}

std::string GS1Converter::getNPCList() const
{
	return "level.npcs";
}

std::string GS1Converter::getBaddiesList() const
{
	return "level.baddies";
}

std::string GS1Converter::fixIdentifierCoordinates(std::string identifier) const
{
	if (m_useLocalCoordinates)
	{
		auto pos = identifier.find("#x#");
		if(pos != std::string::npos)
			identifier.replace(pos, 3, "localTileX");
		
		pos = identifier.find("#y#");
		if(pos != std::string::npos)
			identifier.replace(pos, 3, "localTileY");
	}
	else {
		auto pos = identifier.find("#x#");
		if (pos != std::string::npos)
			identifier.replace(pos, 3, "tileX");

		pos = identifier.find("#y#");
		if (pos != std::string::npos)
			identifier.replace(pos, 3, "tileY");
	}
	return identifier;
}



void GS1Converter::Visit(gs1::Stmt* node)
{

}

std::string GS1Converter::formatStringProp(const std::string& input, bool player, bool* wasColour, bool encloseFirstString)
{
	auto getInnerParams = [](const std::string& name, size_t* pos) -> std::string
		{
			int parentThesisCount = 0;
			if (name[*pos] == '(')
			{
				for (size_t startPos = ++(*pos); (*pos) < name.length(); ++(*pos))
				{
					if (name[*pos] == '(')
						++parentThesisCount;

					else if (name[*pos] == ')')
					{
						if (parentThesisCount == 0)
						{
							size_t len = *pos - startPos;
							std::string retval = name.substr(startPos, len);
							++(*pos);
							return retval;
						}
						else --parentThesisCount;
					}

				}
			}
			
			return "";

		};



	std::string retval = "";
	std::string currentLiteral = "";
	bool firstString = true;
	auto addExpression = [&](const std::string& expr)
		{
			if (!retval.empty())
				retval += " $ ";

			if (!currentLiteral.empty()) {
				retval += "\"" + escape_string(currentLiteral) + "\"";

				firstString = false;
				if (!expr.empty())
				{
					retval += " $ ";
				}
				currentLiteral = "";
			}

			retval += expr;
		};

	auto convert = [&](const std::string& text) -> std::string
	{
		auto observer = [](const gs1::Diag& d) ->void
			{
			};

		gs1::DiagBuilder diagBuilder(observer);

		gs1::MemorySource source(text.c_str(), text.length());
		gs1::Lexer lexer(diagBuilder, source);

		gs1::Parser parser(diagBuilder, lexer, cmds, funcs);


		auto rootNode = parser.Parse();
		auto oldStream = m_currentStream;

		ByteStream temp;
		m_currentStream = &temp;

		for (auto child : rootNode->children)
			child->Accept(this);
		m_currentStream = oldStream;
		return temp.ToString();
	};

	std::string self = player ? "player." : "this.";
	auto getTarget = [&](const std::string& input, size_t* pos) -> std::string
	{
		auto params = getInnerParams(input, pos);
		if (!params.empty())
		{
			auto expr = trim(convert(params));
			if (expr == "-1")
			{
				return "this.";
			}
			else if (expr == "0")
			{
				return "player.";
			}
			else {
				return "this.level.players[" + expr + "].";
			}
		}
		return self;
	};



	for (size_t pos = 0; pos < input.length();)
	{
		if (input[pos] == '#' && pos <= input.length() - 2)
		{
			if (input[pos + 1] == 'a')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);

				addExpression(owner + "username");

				continue;
			}
			else if (input[pos+1] == 'v' || input[pos + 1] == 's')
			{
				pos += 2;
				auto params = getInnerParams(input, &pos);
				if (params.length())
				{
					addExpression(convert(params));
				}
				continue;
			}
			else if (input[pos + 1] == 't')
			{
				pos += 2;
				auto params = getInnerParams(input, &pos);
				if (params.length())
				{
					addExpression("this.tokens[" + convert(params) + "]");
				}
				continue;
			}
			else if (input[pos + 1] == 'f' || input[pos + 1] == 'F')
			{
				addExpression("this.imageName");
				pos += 2;
				continue;
			}
			else if (input[pos + 1] == 'n' || input[pos + 1] == 'N')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);
				addExpression(owner + "nickName");

				continue;
			}
			else if (input[pos + 1] == 'm')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);
				addExpression(owner + "aniFile");
				
				continue;
			}
			else if (input[pos + 1] == 'c')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);
				addExpression(owner + "chat");
				
				continue;
			}
			else if (input[pos + 1] == 'L')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);
				addExpression(owner + "levelName");

				continue;
			}
			else if (input[pos + 1] == 'w' || input[pos + 1] == 'W')
			{
				pos += 2;
				auto params = getInnerParams(input, &pos);
				if (params.length() > 0)
				{
					bool a;

					addExpression(self+ "weapons[" + convert(params) + "].weaponName");
					continue;
				}

				addExpression(self + "selectedWeapon.weaponName");
				continue;
			}
			else if (input[pos + 1] == '1')
			{
				pos += 2;

				auto owner = getTarget(input, &pos);
				addExpression(owner + "sword");
				
				continue;
			}
			else if (input[pos + 1] == '2')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);

				addExpression(owner + "shield");

				continue;
			}
			else if (input[pos + 1] == '3')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);

				addExpression(owner + "head");

				continue;
			}
			else if (input[pos + 1] == '5')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);

				addExpression(owner + "private.horseImage");

				continue;
			}
			else if (input[pos + 1] == '7')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);
				addExpression(owner + "bow");

				continue;
			}
			else if (input[pos + 1] == '8')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);
				addExpression(owner + "body");

				continue;
			}
			else if (input[pos + 1] == 'g')
			{
				pos += 2;
				auto owner = getTarget(input, &pos);
				addExpression(owner + "guild");

				continue;
				}
			else if (input[pos + 1] == 'P' && pos <= input.length() - 3)
			{
				auto index = input[pos + 2] - '1';
				addExpression(self + "aniAttribs[" + std::to_string(index) + "]");
				pos += 3;
				continue;

			}
			else if (input[pos + 1] == 'C' && pos <= input.length() - 3)
			{
				static std::string colourNames[] = {
					"BODY_SKIN",
					"BODY_COAT",
					"BODY_SLEEVE",
					"BODY_SHOES",
					"BODY_BELT"

				};

				auto index = input[pos + 2] - '0';
				pos += 3;

				auto owner = getTarget(input, &pos);
				if (index < sizeof(colourNames) / sizeof(colourNames[0]))
				{
					if (wasColour)
						*wasColour = true;
					addExpression(owner + "colours[" + colourNames[index] + "]");
				} else addExpression(owner + "colours[BODY_BELT]");
				
				continue;
			}
		}
		currentLiteral += input[pos++];
	}

	if (!currentLiteral.empty()) {
		addExpression("");
	}

	if (retval.empty())
		return "\"\"";

	return retval;
}


void GS1Converter::Visit(struct gs1::SyntaxTerminal* node)
{
	//output() << node->token.text;
}


void GS1Converter::Visit(struct gs1::StmtEmpty* node)
{

}

void GS1Converter::Visit(gs1::StmtBlock* node)
{
	++m_blockLevel;
	output() << "{\n";
	increaseTab();
	for (auto i = 0; i < node->children.size(); ++i)
	{
		auto child = node->children[i];

		if (child->GetType() != "SyntaxTerminal" && child->GetType() != "StmtEmpty" && child->GetType() != "SyntaxNode")
		{
			output() << getTab();
			child->Accept(this);

			if (child->hasEndOfLine())
				output() << ";\n";

		}


	}
	decreaseTab();
	output() << getTab() << "}\n";
	--m_blockLevel;
}

void GS1Converter::Visit(gs1::StmtIf* node)
{
	if (m_blockLevel == 0 && node->elseBody == nullptr)
	{
		if (node->cond->GetType() == "ExprId")
		{
			auto expression = static_cast<gs1::ExprId*>(node->cond);
			auto actionName = expression->name->token.text;


			if (validActions.find(actionName) != validActions.end())
			{
				if (actionName == "weaponfired")
					m_isWeapon = true;
				m_isPossiblePlayerFlag = m_isIfExpression = false;
				m_currentStream = &m_actionCode[actionName].first;

				++m_blockLevel;

				if (node->thenBody->GetType() == "StmtBlock")
				{
					for (auto i = 0; i < node->thenBody->children.size(); ++i)
					{
						auto child = node->thenBody->children[i];

						if (child->GetType() != "SyntaxTerminal" && child->GetType() != "StmtEmpty" && child->GetType() != "SyntaxNode")
						{
							output() << getTab();
							child->Accept(this);

							if (child->hasEndOfLine())
								output() << ";\n";

						}
					}
				}
				else {
					output() << getTab();
					node->thenBody->Accept(this);
					if (node->thenBody->hasEndOfLine())
						output() << ";\n";
				}

				m_currentStream = &m_output;
				--m_blockLevel;
				return;
			}
		}
		else if (node->cond->GetType() == "ExprCall")
		{
			auto expression = static_cast<gs1::ExprCall*>(node->cond);
			if (expression->left->GetType() == "ExprId")
			{
				auto& actionName = static_cast<gs1::ExprId*>(expression->left)->name->token.text;

				if (validActions.find(actionName) != validActions.end())
				{
					if (actionName == "playersays")
					{
						m_currentStream = &m_actionCode[actionName].first;

						output() << getTab() << "if(_chatText==";
						expression->args[0]->Accept(this);
						output() << ")\n";

						output() << getTab();
						node->thenBody->Accept(this);

						m_currentStream = &m_output;
						return;
					}
				}
			}
		}
	}





	bool removedActions = false;
	std::string removedActionName = "";
	auto oldStream = m_currentStream;
removeActions:

	ByteStream tempStream;
	m_currentStream = &tempStream;
	if (m_blockLevel == 0)
	{
		m_tempActionsList.clear();
		output() << getTab();

	}

	output() << "if(";
	m_isPossiblePlayerFlag = m_isIfExpression = true;
	node->cond->Accept(this);
	m_isPossiblePlayerFlag = m_isIfExpression = false;
	output() << ")\n";

	if (node->thenBody)
	{
		if (node->thenBody->GetType() != "StmtBlock")
			increaseTab();
		output() << getTab();

		node->thenBody->Accept(this);
		if (node->thenBody->GetType() != "StmtBlock")
		{

			if (node->thenBody->hasEndOfLine())
				output() << ";\n";
			decreaseTab();
		}
	}
	else output() << getTab() << "{}\n";

	if (node->elseBody)
	{
		output() << getTab() << "else ";
		++m_blockLevel;
		node->elseBody->Accept(this);

		if (node->elseBody->GetType() != "StmtBlock")
		{
			if (node->elseBody->hasEndOfLine())
				output() << ";\n";
		}
		--m_blockLevel;

		
	}

	m_currentStream = oldStream;


	if (m_blockLevel == 0)
	{

		//If only one action Flag
		if (node->elseBody == nullptr && m_tempActionsList.size() == 1 || removedActions)
		{

			if (!removedActions)
			{
				//One action flag was detected. Simply append this code to the end of that function
				auto actionName = *m_tempActionsList.begin();
				removedActionName = actionName;
				if (!removedActions)
				{
					removedActions = true;
					goto removeActions;
				}
			}

			m_actionCode[removedActionName].first << "\n" << tempStream << "\n";

			return;
		}
		else if (m_tempActionsList.size() == 0)
		{
			//No action flags were detected in the code....just add it to our parseGS1Thing
			m_parseActionFunction << tempStream << "\n";
			m_addParseGS1ToAllActions = true;
			return;
		}
		
		
		//This if statement has checks for multiple different actions
		//Append this code to "parseGS1Action" and in each of the action functions
		//call parseGS1Action()



		for (auto action : m_tempActionsList)
		{
			auto& thing = m_actionCode[action];

			if (!thing.second)
			{
				thing.second = true;

				ByteStream stream = ByteStream() << getTab() << "this.parseGS1Action(\"";
				stream << action << "\", player, ";

				std::string contextualNPC = "null";

				auto it = validActions.find(action);
				if (it != validActions.end())
				{
					auto& funcHeader = it->second;
					for (auto i = 1U; i < funcHeader.size(); ++i)
					{
						if (funcHeader[i] == "N")
						{
							contextualNPC = "npc";
						}
					}

					stream << contextualNPC;
					stream << ");\n";
				}
				m_actionCode[action].first = m_actionCode[action].first << stream;

			}
		}

		//This code includes multiple "action" flag checks. Add it to the end of "parseGS1Action".
		m_parseActionFunction << "\n" << tempStream << "\n";




	}
	else {
		output() << tempStream << "\n";
	}

}

void GS1Converter::Visit(gs1::StmtFor* node)
{
	output() << "for(";
	if (node->init)
	{
		node->init->Accept(this);
	}
	output() << "; ";
	if (node->cond)
	{
		node->cond->Accept(this);
	}
	output() << "; ";
	if (node->step)
	{

		node->step->Accept(this);
	}
	output() << ")\n";

	if (node->body->GetType() != "StmtBlock")
		increaseTab();
	output() << getTab();

	node->body->Accept(this);
	if (node->body->GetType() != "StmtBlock")
	{
		if(node->body->hasEndOfLine())
			output() << ";\n";
		decreaseTab();
	}
}



void GS1Converter::Visit(gs1::StmtWhile* node)
{
	output() << "while(";
	node->cond->Accept(this);
	output() << ")\n";

	if (node->body->GetType() != "StmtBlock")
		increaseTab();
	output() << getTab();


	node->body->Accept(this);
	if (node->body->GetType() != "StmtBlock")
	{
		if (node->body->hasEndOfLine())
			output() << ";\n";
		decreaseTab();
	}
}

void GS1Converter::Visit(gs1::StmtBreak* node)
{
	output() << "break";
}

void GS1Converter::Visit(gs1::StmtContinue* node)
{
	output() << "continue";
}

void GS1Converter::Visit(gs1::StmtReturn* node)
{
	output() << "return";
}

void GS1Converter::Visit(gs1::StmtCommand* node)
{
	auto isIfExpression = m_isIfExpression;
	m_isIfExpression = false;

	if (!commandReplacement(node->name->token.text, node->args))
	{
		auto commandName = node->name->token.text;

		bool skipOpeningParenthesis = false;
		auto it = identifierReplacements.find(commandName);
		if (it != identifierReplacements.end())
		{
			commandName = fixIdentifierCoordinates(it->second.second);

			if (it->second.first) {
				if (commandName.find(".") == std::string::npos)
					commandName = "this." + commandName;
			}

			if (commandName[commandName.length() - 1] != '(')
			{
				m_isIfExpression = isIfExpression;
				output() << commandName;
				return;
			}
			else skipOpeningParenthesis = true;
		}


		output() << commandName << (skipOpeningParenthesis ? "" : "(");

		for (size_t i = 0; i < node->args.size(); ++i)
		{
			auto arg = node->args[i];
			if(arg)
				arg->Accept(this);

			if (i != node->args.size() - 1)
			{
				output() << ", ";
			}

		}

		output() << ")";
	}
	m_isIfExpression = isIfExpression;
}

void GS1Converter::Visit(gs1::StmtFunctionDecl* node)
{
	output() << "function " << node->name->token.text << "(player)\n";
	decreaseTab();	
	node->body->Accept(this);
	increaseTab();
}

void GS1Converter::Visit(gs1::ExprId* node)
{
	auto identifier = node->name->token.text;
	if (m_isIfExpression && validActions.find(identifier) != validActions.end())
	{
		if (identifier == "weaponfired")
			m_isWeapon = true;

		node->isAction = true;
		output() << "_actionName==\"" << identifier << "\"";
		m_tempActionsList.insert(identifier);
	}
	else if (m_isIfExpression && identifier.rfind("action") == 0)
	{
		node->isAction = true;
		output() << "_actionName==\"" << identifier << "\"";
	}
	else {


		qDebug() << "IDENT: " << QString::fromStdString(identifier) << " : " << node->dontAddThis << "\n";
		auto it = identifierReplacements.find(identifier);
		if (it != identifierReplacements.end())
		{
			if (it->second.first && !node->dontAddThis) {
				if (identifier.rfind("this.", 0) != 0)
					output() << "this.";
			}
			if (identifier == "save") {
				generateFunction("save");
			}
			output() << fixIdentifierCoordinates(it->second.second);
		}
		else {

			if (identifier.find('.') == std::string::npos)
			{
				if (identifier.rfind("sleep", 0) == 0)
				{
					auto number = identifier.substr(5);
					output() << "sleep(" + number + ")";
					return;
				}

				output() << "player.temp." << identifier;
			}
			else output() << fixIdentifier(identifier);

			//if (m_isPossiblePlayerFlag && m_isIfExpression && identifier.find('.') == std::string::npos)
			//{
			//	output() << "player.flagSet(\"" << identifier << "\")";
			//} else output() << fixIdentifier(identifier);
		}
	}

}

void GS1Converter::Visit(gs1::ExprList* node)
{
	output() << "[";
	for (auto i = 0U; i < node->elements.size(); ++i)
	{
		auto element = node->elements[i];
		element->Accept(this);

		if (i != node->elements.size() - 1)
			output() << ", ";
	}

	output() << "]";
}



void GS1Converter::Visit(gs1::ExprStringLiteral* node)
{

	auto escapedString = formatStringProp(trim(node->literal->token.text), true, nullptr);
	output() << escapedString;
}

void GS1Converter::Visit(gs1::ExprNumberLiteral* node)
{
	auto value = node->literal->token.text;

	if (value.find('.') == 0)
		output() << "0" << value;
	else output() << value;
}

void GS1Converter::Visit(gs1::ExprBinaryOp* node)
{
	if (node->left)
		node->left->dontAddThis = node->dontAddThis;


	if (node->op->token.text == "^")
	{
		output() << "pow(";
		node->left->Accept(this);
		output() << ", ";
		node->right->Accept(this);
		output() << ")";
	} else if (node->op->token.text == "%")
	{
		output() << "mod(";
		node->left->Accept(this);
		output() << ", ";
		node->right->Accept(this);
		output() << ")";
	}
	else if (node->op->token.text == "in")
	{
		if (node->right && node->right->GetType() == "ExprRange")
		{

			output() << "inRange(";
			if(node->left)node->left->Accept(this);
			output() << ", ";
			auto range = static_cast<gs1::ExprRange*>(node->right);

			if(range && range->lower)range->lower->Accept(this);
			output() << ", ";
			if(range && range->upper)range->upper->Accept(this);
			output() << ")";


		}
		else if(node->right && node->left) {
			node->right->Accept(this);
			output() << ".find(";
			node->left->Accept(this);
			output() << ") !== null";
		}
	}
	else {
;
		if(node->grouped)
			output() << "(";

		if (!(node->op->token.text == "&&" || node->op->token.text == "||"))
			m_isPossiblePlayerFlag = false;

		if (!node->left->isAction)
		{
			node->left->Accept(this);

			if (node->right && !node->right->isAction)
			{
				//fix
				if(node->op->token.text == "=" && m_isIfExpression)
					output() << " == ";
				else if(node->op->token.text == "=>")
					output() << " >= ";
				else if (node->op->token.text == "=<")
					output() << " <= ";
				else output() << " " << node->op->token.text << " ";
			}
		}


		if (node->right && !node->right->isAction)
			node->right->Accept(this);

		if(node->grouped)
			output() << ")";
	}
}

void GS1Converter::Visit(gs1::ExprUnaryOp* node)
{
	if (node->prefix)
	{
		if (node->op->token.text == "!")
		{
			if (node->expr && node->expr->GetType() == "ExprBinaryOp" && !node->expr->grouped)
			{
				output() << node->op->token.text << "(";
				node->expr->Accept(this);
				output() << ")";
				return;
			}
		}
		output() << node->op->token.text;

		if (node->expr)
			node->expr->Accept(this);
	}
	else {
		if (node->expr)
			node->expr->Accept(this);
		output() << node->op->token.text;
	}
}

void GS1Converter::Visit(gs1::ExprTernaryOp* node)
{
	output() << "if(";
	node->cond->Accept(this);

	output() << ", ";
	node->thenValue->Accept(this);
	output() << ", ";

	node->elseValue->Accept(this);
	output() << ")";
}

void GS1Converter::Visit(gs1::ExprCall* node)
{
	if (node->left->GetType() == "ExprId")
	{
		auto expressID = static_cast<gs1::ExprId*>(node->left);

		auto identifier = expressID->name->token.text;;

		auto isIfExpression = m_isIfExpression;
		m_isIfExpression = false;

		bool skipOpeningParenthesis = false;
		if (!commandReplacement(identifier, node->args))
		{
			auto isHandled = false;
			for (auto it : funcs)
			{
				if (it.first == identifier)
				{
					isHandled = true;
					break;
				}
			}
			
			
			if (!isHandled)
			{
				auto it = identifierReplacements.find(identifier);
				if (it != identifierReplacements.end())
				{
					isHandled = true;
					identifier = fixIdentifierCoordinates(it->second.second);

					if (it->second.first) {
						if (identifier.find(".") == std::string::npos)
							identifier = "this." + identifier;
					}

					if (identifier[identifier.length() - 1] != '(')
					{
						m_isIfExpression = isIfExpression;
						output() << identifier;
						return;
					}
					else skipOpeningParenthesis = true;
				}
			}

			if (!isHandled)
			{
				output() << "this.";
			}

			output() << identifier << (skipOpeningParenthesis ? "" : "(");

			if (!isHandled)
				output() << "player" << (node->args.size() ? ", " : "");
			for (size_t i = 0; i < node->args.size(); ++i)
			{
				auto arg = node->args[i];

				if (arg)
				{
					arg->Accept(this);
				}
				if (i != node->args.size() - 1)
				{
					output() << ", ";
				}

			}
			output() << ")";
		}
		m_isIfExpression = isIfExpression;
	}


}

void GS1Converter::Visit(struct gs1::ExprIndex* node)
{
	node->left->dontAddThis = node->dontAddThis;
	if (node->left->GetType() == "ExprId")
	{
		auto leftExpr = static_cast<gs1::ExprId*>(node->left);
		if (leftExpr->name->token.text == "tiles" || leftExpr->name->token.text == "board")
		{
			if (node->moreIndexes.size() == 0)
			{
				output() << "this.level.gtiles[";
				node->index->Accept(this);
				output() << "]";
			}
			else {
				output() << "this.level.gtiles[(";
				node->moreIndexes[0]->Accept(this);
				output() << ") * 64 + (";
				node->index->Accept(this);
				output() << ")]";
			}

			return;
		}

	}

	node->left->Accept(this);
	output() << "[";
	node->index->Accept(this);

	for (auto i = 0U; i < node->moreIndexes.size(); ++i)
	{
		output() << ", ";
		node->moreIndexes[i]->Accept(this);
	}
	output() << "]";

}

void GS1Converter::Visit(struct gs1::ExprIndexDotLookup* node)
{
	for(auto i = 0U; i < node->children.size()-2; ++i)
	{
		auto child = node->children[i];
		if (child->GetType() == "ExprId")
			static_cast<gs1::ExprId*>(child.get())->dontAddThis = node->dontAddThis;

		child->Accept(this);
	}

	output() << ".";

	qDebug() << "LOL: " << QString::fromStdString(node->id->GetType()) << "\n";
	node->id->dontAddThis = true;
	node->id->Accept(this);

}

std::string GS1Converter::fixIdentifier(std::string identifier, bool asIndex) const
{
	if (identifier.rfind("sleep", 0) == 0)
	{
		auto number = identifier.substr(5);
		return "sleep(" + number + ")";
	}

	if (identifier.rfind("level.", 0) == 0)
	{
		if (asIndex)
			return "this.level.temp[\"" + identifier.substr(6) + "\"]";
		else return "this.level.temp." + identifier.substr(6);
	}

	if (identifier.rfind("local.", 0) == 0)
	{
		if (asIndex)
			return "this.level.temp[\"" + identifier.substr(6) + "\"]";
		else return "this.level.temp." + identifier.substr(6);
	}

	if (identifier.rfind("this.", 0) == 0)
	{
		if(asIndex)
			return "this.temp[\"" + identifier.substr(5) + "\"]";
		else return "this.temp." + identifier.substr(5);
	}

	if (identifier.find('.') == std::string::npos)
	{
		if(asIndex)
			return "player.temp[\"" + identifier + "\"]";
		else return "player.temp." + identifier;
	}
	return identifier;
}

