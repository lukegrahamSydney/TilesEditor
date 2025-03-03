#ifndef TILEMAPH
#define TILEMAPH

#include <QPainter>
#include <QList>
#include <QPair>
#include <QRect>
#include "AbstractLevelEntity.h"
#include "Image.h"
#include "LevelEntityType.h"
#include "sgscript/sgscript.h"
#include "AbstractResourceManager.h"

namespace TilesEditor
{
	class Tilemap :
		public AbstractLevelEntity
	{
		friend class Renderer;

	private:
		unsigned int    m_hcount,
			m_vcount;

		double m_layerIndex;
		int* m_tiles;


	public:
		Tilemap(const Tilemap& source);

		Tilemap(IWorld* world, double x, double y, int hcount, int vcount, int layerIndex);

		~Tilemap() {
			delete m_tiles;
		}

		void clear(int tile);

		/*
		void reset(int hcount, int vcount) {
			delete m_tiles;
			m_hcount = hcount;
			m_vcount = vcount;
			m_tiles = new int[hcount * vcount];
		}*/


		int getWidth() const override {
			return m_hcount * 16;
		}

		int getHeight() const override {
			return m_vcount * 16;
		}

		int getHCount() const {
			return m_hcount;
		}

		int getVCount() const {
			return m_vcount;
		}

		int getLayerIndex() const {
			return m_layerIndex;
		}

		double getDepth() const override { return (double)m_layerIndex; }

		double getRealDepth() const override {
			return getDepth();
		}
		bool update(double delta) { return false; }

		Tilemap& operator=(const Tilemap& other);
		void setTile(unsigned int x, unsigned int y, int tile) {
			if (x < m_hcount && y < m_vcount) {
				m_tiles[y * m_hcount + x] = tile;
			}
		}

		int getTile(unsigned int x, unsigned int y) const {
			if (x < m_hcount && y < m_vcount) {
				return m_tiles[y * m_hcount + x];
			}
			return 0;
		}

		bool tryGetTile(unsigned int x, unsigned int y, int* tile) const {
			if (x < m_hcount && y < m_vcount) {
				*tile = m_tiles[y * m_hcount + x];
				return true;
			}
			*tile = MakeInvisibleTile(0);
			return false;
		}

		LevelEntityType getEntityType() const { return LevelEntityType::ENTITY_TILEMAP; }
		void draw(QPainter* painter, const QRectF& viewRect, double x, double y) override;
		void draw(QPainter* painter, const QRectF& viewRect, Image* tilesetImage, double x, double y);

		AbstractLevelEntity* duplicate() { return nullptr; };

		

		static bool IsInvisibleTile(int tile) {
			static int invisibleTile = MakeInvisibleTile(0) & 0xFFFFF;

			return (tile & 0xFFFFF) == invisibleTile;
		}

		static int MakeTile(int left, int top, int type, int translucency = 0) {
			int retval = ((translucency & 0xF) << 28) | ((type & 0xFF) << 20) | ((left & 0x3FF) << 10) | ((top & 0x3FF));
			return retval;
		}

		static int ReplaceTranslucency(int tile, int transValue) {
			return ((transValue & 0xF) << 28) | (tile & 0xFFFFFFF);
		}
		static int MakeInvisibleTile(int type) {
			return MakeTile(0x3FF, 0x3FF, type, 0);
		}


		static int GetTileY(int tile) {
			return ((unsigned int)tile) & 0x3FF;
		}

		static int GetTileX(int tile) {
			return ((unsigned int)tile >> 10) & 0x3FF;
		}


		static int GetTileType(int tile) {
			return (unsigned int)((tile >> 20) & 0xFF);
		}

		static int GetTileTranslucency(int tile) {
			return (unsigned int)((tile >> 28) & 0xF);
		}

		static sgs_Variable sgs_classMembers;

		static sgs_ObjInterface sgs_interface;
		static void registerScriptClass(IEngine* engine);
	};
};

#endif
