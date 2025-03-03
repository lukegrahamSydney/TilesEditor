#include "AbstractLevelEntity.h"
#include "AbstractSelection.h"
#include "Level.h"
#include "IEngine.h"

namespace TilesEditor
{
	sgs_Variable AbstractLevelEntity::sgs_classMembers;
	sgs_ObjInterface AbstractLevelEntity::sgs_interface;


	AbstractLevelEntity::AbstractLevelEntity(IWorld* world, double x, double y) {
		m_world = world;
		setX(x);
		setY(y);
		m_level = nullptr;
		m_tileLayer = 0;

		m_dragOffsetX = m_dragOffsetY = 0.0;
		m_entityOrder = nextEntityOrder;
		nextEntityOrder += 0.000001;


		m_thisObject = sgs_MakeNull();
		m_sgsUserTable = sgs_MakeNull();
	}

	AbstractLevelEntity::~AbstractLevelEntity()
	{
		if (m_world && m_thisObject.type != SGS_VT_NULL)
		{
			m_thisObject.data.O->data = nullptr;
			auto engine = m_world->getEngine();
			sgs_Release(engine->getScriptContext(), &m_sgsUserTable);
			engine->removeCPPOwnedObject(m_thisObject);
			sgs_Release(engine->getScriptContext(), &m_thisObject);
		}
	}

	void AbstractLevelEntity::setProperty(const QString& name, const QVariant& value)
	{
		if (name == "order")
			setEntityOrder(value.toDouble());
	}

	sgs_Variable& AbstractLevelEntity::getScriptObject()
	{
		if (m_thisObject.type == SGS_VT_NULL)
		{
			auto engine = getWorld()->getEngine();
			sgs_CreateObject(engine->getScriptContext(), &m_thisObject, this, &sgs_interface);
			sgs_CreateDict(engine->getScriptContext(), &m_sgsUserTable, 0);
			engine->addCPPOwnedObject(m_thisObject);
		}
			
		return m_thisObject;
	}

	double AbstractLevelEntity::getUnitWidth() const
	{
		return getWorld()->getUnitWidth();
	}

	double AbstractLevelEntity::getUnitHeight() const {
		return getWorld()->getUnitHeight();
	}

	void AbstractLevelEntity::setDragOffset(double x, double y, bool snap, double snapX, double snapY) {
		if (snap)
		{
			m_dragOffsetX = int(std::floor((x - this->getX()) / snapX) * snapX);
			m_dragOffsetY = int(std::floor((y - this->getY()) / snapY) * snapY);
		}
		else {
			m_dragOffsetX = x - this->getX();
			m_dragOffsetY = y - this->getY();
		}
	}

	void AbstractLevelEntity::drag(double x, double y, bool snap, double snapX, double snapY) {
		auto oldX = getX();
		auto oldY = getY();

		auto dragRect = getDragRect(x, y, snap, snapX, snapY);
		setX(dragRect.x());
		setY(dragRect.y());


		if (this->getLevel())
		{
			if (oldX != getX() || oldY != getY())
				getWorld()->setModified(this->getLevel());
		}
	}

	QRectF AbstractLevelEntity::getDragRect(double x, double y, bool snap, double snapX, double snapY) const
	{
		if (snap)
		{
			return QRectF((std::floor(x / snapX) * snapX) - m_dragOffsetX, (std::floor(y / snapY) * snapY) - m_dragOffsetY, getWidth(), getHeight());
		}
		else {
			return QRectF(x - m_dragOffsetX, y - m_dragOffsetY, getWidth(), getHeight());
		}
	}

	void AbstractLevelEntity::updateResize(int edges, int mouseX, int mouseY, bool snap, double snapX, double snapY)
	{
		if (edges & AbstractSelection::EDGE_LEFT)
		{
			int x = std::min(mouseX, (int)getRight());
			int width = (int)getRight() - x;
			width = std::max(int(snapX), int(std::floor(width / snapX) * snapX));

			setX(getRight() - width);
			setWidth(width);
		}

		if (edges & AbstractSelection::EDGE_RIGHT)
		{
			int pos2 = std::max(mouseX, (int)getX());
			int width = pos2 - (int)getX();
			width = std::max(int(snapX), int(std::ceil(width / snapX) * snapX));

			setWidth(width);
		}

		if (edges & AbstractSelection::EDGE_TOP)
		{
			int y = std::min(mouseY, (int)getBottom());
			int height = (int)getBottom() - y;
			height = std::max(int(snapY), int(std::floor(height / snapY) * snapY));

			setY(getBottom() - height);
			setHeight(height);
		}


		if (edges & AbstractSelection::EDGE_BOTTOM)
		{
			int pos2 = std::max(mouseY, (int)getY());
			int height = pos2 - (int)getY();
			height = std::max(int(snapY), int(std::ceil(height / snapY) * snapY));
			setHeight(height);
		}
	}

	QRectF AbstractLevelEntity::updateResizeRect(int edges, int mouseX, int mouseY, bool snap, double snapX, double snapY)
	{
		QRectF retval = toQRectF();
		if (edges & AbstractSelection::EDGE_LEFT)
		{
			int x = std::min(mouseX, (int)getRight());
			int width = (int)getRight() - x;
			width = std::max(int(snapX), int(std::floor(width / snapX) * snapX));

			retval.setX(getRight() - width);
			retval.setWidth(width);
		}

		if (edges & AbstractSelection::EDGE_RIGHT)
		{
			int pos2 = std::max(mouseX, (int)getX());
			int width = pos2 - (int)getX();
			width = std::max(int(snapX), int(std::ceil(width / snapX) * snapX));

			retval.setWidth(width);
		}

		if (edges & AbstractSelection::EDGE_TOP)
		{
			int y = std::min(mouseY, (int)getBottom());
			int height = (int)getBottom() - y;
			height = std::max(int(snapY), int(std::floor(height / snapY) * snapY));

			retval.setY(getBottom() - height);
			retval.setHeight(height);
		}


		if (edges & AbstractSelection::EDGE_BOTTOM)
		{
			int pos2 = std::max(mouseY, (int)getY());
			int height = pos2 - (int)getY();
			height = std::max(int(snapY), int(std::ceil(height / snapY) * snapY));
			retval.setHeight(height);
		}
		return retval;
	}

	void AbstractLevelEntity::mark(sgs_Context* ctx)
	{
		sgs_GCMark(ctx, &m_sgsUserTable);
	}

	void AbstractLevelEntity::registerScriptClass(IEngine* engine)
	{
        auto ctx = engine->getScriptContext();
        auto startStackSize = sgs_StackSize(ctx);


        auto memberCount = sgs_StackSize(ctx) - startStackSize;
        sgs_CreateDict(ctx, &sgs_classMembers, memberCount);
        engine->addCPPOwnedObject(sgs_classMembers);

        auto objDestruct = [](sgs_Context* ctx, sgs_VarObj* obj) -> int
        {
            auto self = static_cast<AbstractLevelEntity*>(obj->data);

            if (self == nullptr)
                return 0;

            return 1;
        };


        auto objMark = [](sgs_Context* C, sgs_VarObj* obj) -> int
        {
            auto self = static_cast<AbstractLevelEntity*>(obj->data);
            if (self == nullptr)
                return 0;

			self->mark(C);
            return 1;
        };

        auto objGetIndex = [](sgs_Context* C, sgs_VarObj* obj) -> int
        {
            auto self = static_cast<AbstractLevelEntity*>(obj->data);

            if (self == nullptr)
                return 0;

			auto retval = self->scriptGetMember(C);
			if (retval)
				return retval;

			sgs_Variable propName = sgs_StackItem(C, 0);

            if (sgs_PushIndex(C, AbstractLevelEntity::sgs_classMembers, propName, 1))
                return 1;

            return 0;
        };

        auto objSetIndex = [](sgs_Context* C, sgs_VarObj* obj) -> int
            {
                auto self = static_cast<AbstractLevelEntity*>(obj->data);

                if (self == nullptr)
                    return 0;

                return self->scriptSetMember(C);
            };


        sgs_interface = {
            "LevelEntity",
            objDestruct, objMark,  /* destruct, gcmark */
            objGetIndex, objSetIndex,  /* getindex, setindex */
            NULL, NULL, NULL, NULL, /* convert, serialize, dump, getnext */
            NULL, NULL,              /* call, expr */
            NULL, &AbstractLevelEntity::sgs_interface	/*proplist, parent*/
        };

	}
}