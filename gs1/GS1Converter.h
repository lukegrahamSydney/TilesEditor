#ifndef GS1CONVERTERH
#define GS1CONVERTERH

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <gs1/parse/Parser.hpp>
#include <gs1/parse/SyntaxTreeVisitor.hpp>

#include "ByteStream.h"

class GS1Converter :
	public gs1::SyntaxTreeVisitor
{
private:
	bool m_isIfExpression = false;
	bool m_isWeapon = false;
	bool m_isPossiblePlayerFlag = false;
	bool m_addParseGS1ToAllActions = false;
	bool m_clientSide = false;

	bool m_useLocalPlayerList = true;
	bool m_useLocalCoordinates = true;
	std::unordered_set<std::string> m_generatedFunctions;
	std::unordered_set<std::string> m_tempActionsList;
	std::unordered_map<std::string, std::pair<ByteStream, bool> > m_actionCode;
	std::unordered_map<std::string, std::string> m_generatedCode;

	std::string m_tab;
	ByteStream m_createCode;
	ByteStream m_generatedFunctionsCode;
	ByteStream m_output;
	ByteStream m_parseActionFunction;

	ByteStream* m_currentStream = &m_output;
	int m_blockLevel = 0;

	void increaseTab();
	void decreaseTab();

	std::string getTab() const;
	ByteStream& output() { return *m_currentStream; }

	bool replaceshowimg(std::vector<gs1::Expr*>& args);
	bool replacechangeimgvis(std::vector<gs1::Expr*>& args);
	bool replacearctan(std::vector<gs1::Expr*>& args);
	bool replacehideimg(std::vector<gs1::Expr*>& args);
	bool replacechangeimgpart(std::vector<gs1::Expr*>& args);
	bool replaceshowcharacter(std::vector<gs1::Expr*>& args);
	bool replacesetcharprop(std::vector<gs1::Expr*>& args);
	bool replacesetplayerprop(std::vector<gs1::Expr*>& args);
	bool replacetokenize(std::vector<gs1::Expr*>& args);
	bool replacestrequals(std::vector<gs1::Expr*>& args);
	bool replacestartswith(std::vector<gs1::Expr*>& args);
	bool replacesetstring(std::vector<gs1::Expr*>& args);
	bool replacestrtofloat(std::vector<gs1::Expr*>& args);
	bool replaceplay(std::vector<gs1::Expr*>& args);
	bool replacestopmidi(std::vector<gs1::Expr*>& args);
	bool replaceenableweapons(std::vector<gs1::Expr*>& args);
	bool replacedisableweapons(std::vector<gs1::Expr*>& args);
	bool replacesetgif(std::vector<gs1::Expr*>& args);
	bool replaceset(std::vector<gs1::Expr*>& args);
	bool replaceunset(std::vector<gs1::Expr*>& args);
	bool replaceputhorse(std::vector<gs1::Expr*>& args);
	bool replaceputcomp(std::vector<gs1::Expr*>& args);
	bool replaceputnewcomp(std::vector<gs1::Expr*>& args);
	
	bool replaceplayersays(std::vector<gs1::Expr*>& args);
	bool replacesay(std::vector<gs1::Expr*>& args);
	bool replacesay2(std::vector<gs1::Expr*>& args);
	bool replacesetani(std::vector<gs1::Expr*>& args);
	bool replacesetcharani(std::vector<gs1::Expr*>& args);
	bool replacedontblocklocal(std::vector<gs1::Expr*>& args);
	bool replaceblockagainlocal(std::vector<gs1::Expr*>& args);
	bool replacevisible(std::vector<gs1::Expr*>& args);
	bool replacecarryobject(std::vector<gs1::Expr*>& args);
	bool replacetakeplayercarry(std::vector<gs1::Expr*>& args);

	bool replacesetarray(std::vector<gs1::Expr*>& args);
	bool replaceonwall(std::vector<gs1::Expr*>& args);
	bool replacerandom(std::vector<gs1::Expr*>& args);
	bool replacemessage(std::vector<gs1::Expr*>& args);
	bool replacelay(std::vector<gs1::Expr*>& args);
	bool replacelay2(std::vector<gs1::Expr*>& args);
	bool replaceputleaps(std::vector<gs1::Expr*>& args);
	bool replaceremovecompus(std::vector<gs1::Expr*>& args);
	bool replacefreezeplayer(std::vector<gs1::Expr*>& args);
	bool replacesetlevel(std::vector<gs1::Expr*>& args);
	bool replacetoweapons(std::vector<gs1::Expr*>& args);
	bool replacesetgifpart(std::vector<gs1::Expr*>& args);
	bool replacetake(std::vector<gs1::Expr*>& args);
	bool replacehitplayer(std::vector<gs1::Expr*>& args);
	bool replacesetbackpal(std::vector<gs1::Expr*>& args);
	bool replacehitcompu(std::vector<gs1::Expr*>& args);
	bool replacehurt(std::vector<gs1::Expr*>& args);
	bool replacevecx(std::vector<gs1::Expr*>& args);
	bool replacevecy(std::vector<gs1::Expr*>& args);
	bool replaceputbomb(std::vector<gs1::Expr*>& args);
	bool replaceputexplosion(std::vector<gs1::Expr*>& args);
	bool replaceputexplosion2(std::vector<gs1::Expr*>& args);
	bool replacesetletters(std::vector<gs1::Expr*>& args);
	bool replacesetlevel2(std::vector<gs1::Expr*>& args);

	bool replacesethead(std::vector<gs1::Expr*>& args);
	bool replacesetskincolor(std::vector<gs1::Expr*>& args);
	bool replacesetsleevecolor(std::vector<gs1::Expr*>& args);
	bool replacesetcoatcolor(std::vector<gs1::Expr*>& args);
	bool replacesetbeltcolor(std::vector<gs1::Expr*>& args);
	bool replacesetshoecolor(std::vector<gs1::Expr*>& args);
	bool replacesetshield(std::vector<gs1::Expr*>& args);
	bool replacesetsword(std::vector<gs1::Expr*>& args);
	bool replacestrlen(std::vector<gs1::Expr*>& args);
	bool replacehidelocal(std::vector<gs1::Expr*>& args);
	bool replaceshowlocal(std::vector<gs1::Expr*>& args);
	bool replacehasweapon(std::vector<gs1::Expr*>& args);
	bool replacearraylen(std::vector<gs1::Expr*>& args);
	bool replaceindexof(std::vector<gs1::Expr*>& args);
	bool replacestrcontains(std::vector<gs1::Expr*>& args);
	bool replacekeydown(std::vector<gs1::Expr*>& args);
	bool replacecallweapon(std::vector<gs1::Expr*>& args);
	bool replacecallnpc(std::vector<gs1::Expr*>& args);
	bool replacetriggeraction(std::vector<gs1::Expr*>& args);
	bool replaceshootball(std::vector<gs1::Expr*>& args);
	bool commandReplacement(const std::string& name, std::vector<gs1::Expr*>& args);

	bool generateshowcharacter();
	bool generateonwall();
	bool generateshowimg();
	bool generatechangeimgvis();
	bool generatehideimg();
	bool generatechangeimgpart();
	bool generatetriggeraction();
	bool generatekeycodes();
	bool generatesave();
	bool generateremoveBaddies();
	bool generateFunction(const std::string& name);
	
	std::string getLevelX() const;
	std::string getLevelY() const;
	std::string getPlayerList() const;
	std::string getNPCList() const;
	std::string getBaddiesList() const;
	std::string fixIdentifierCoordinates(std::string identifier) const;

	std::string formatStringProp(const std::string& input, bool player, bool* wasColour, bool encloseFirstString = true);

	void Visit(struct gs1::SyntaxTerminal* node) override;
	void Visit(struct gs1::Stmt* node) override;
	void Visit(struct gs1::StmtEmpty* node) override;
	void Visit(struct gs1::StmtBlock* node) override;

	void Visit(struct gs1::StmtIf* node) override;

	void Visit(struct gs1::StmtFor* node) override;

	void Visit(struct gs1::StmtWhile* node) override;

	void Visit(struct gs1::StmtBreak* node) override;

	void Visit(struct gs1::StmtContinue* node) override;

	void Visit(struct gs1::StmtReturn* node) override;

	void Visit(struct gs1::StmtCommand* node) override;

	void Visit(struct gs1::StmtFunctionDecl* node) override;

	void Visit(struct gs1::ExprId* node) override;
	void Visit(struct gs1::ExprList* node) override;
	void Visit(struct gs1::ExprStringLiteral* node) override;
	void Visit(struct gs1::ExprNumberLiteral* node) override;
	void Visit(struct gs1::ExprBinaryOp* node) override;
	void Visit(struct gs1::ExprUnaryOp* node) override;
	void Visit(struct gs1::ExprTernaryOp* node) override;
	void Visit(struct gs1::ExprCall* node) override;
	void Visit(struct gs1::ExprIndex* node) override;
	void Visit(struct gs1::ExprIndexDotLookup* node) override;

	std::string fixIdentifier(std::string identifier, bool asIndex = false) const;

public:
	bool convert(const std::string& input, bool clientSide);
	static std::string convert2(const std::string& input, bool clientSide);
	static std::string convert3(const std::string& input);
	static std::string revert(const std::string& input);
	std::string result() const;


};
#endif
