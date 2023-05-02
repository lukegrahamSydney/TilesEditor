#ifndef IENTITYSPATIALMAPH
#define IENTITYSPATIALMAPH

#include <QList>
#include <QSet>
#include "Rectangle.h"

namespace TilesEditor
{
	template <typename  T>
	class IEntitySpatialMap
	{
	public:

		virtual int search(const IRectangle& rect, bool accurate, QList<T*>& output, bool (*f)(T*, void* userData) = nullptr, void* userData = nullptr) = 0;
		virtual int search(const IRectangle& rect, bool accurate, QSet<T*>& output, bool (*f)(T*, void* userData) = nullptr, void* userData = nullptr) = 0;

		virtual T* searchFirst(const IRectangle& rect, bool accurate, bool (*f)(T*, void* userData) = nullptr, void* userData = nullptr) = 0;



		virtual T* entityAt(double x, double y) = 0;
		virtual void add(T* entity) = 0;
		virtual void remove(T* entity) = 0;
		virtual void updateEntity(T* entity) = 0;
	};
};

#endif
